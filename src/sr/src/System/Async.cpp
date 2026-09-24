#include "Async.hpp"

namespace SR
{
	void Async::Initialize(size_t threadCount)
	{
		if (Running)
			return;

		// Com_Error calls exit(), and destroying joinable workers there would std::terminate.
		static std::once_flag registered;
		std::call_once(registered, [] { std::atexit([] { Shutdown(); }); });

		Running = true;
		Workers.reserve(threadCount);

		for (size_t i = 0; i < threadCount; i++)
			Workers.emplace_back(&Async::WorkerThread);
	}

	void Async::Shutdown()
	{
		if (!Running)
			return;

		{
			// Flipped under the lock so a worker between its predicate check and wait() can't miss the wakeup.
			std::scoped_lock lock(QueueMutex);
			Running = false;

			for (auto& task : ActiveTasks)
				task->Cancel();
		}
		Condition.notify_all();

		for (auto& worker : Workers)
		{
			if (worker.get_id() == std::this_thread::get_id())
				worker.detach();
			else if (worker.joinable())
				worker.join();
		}
		Workers.clear();

		std::scoped_lock lock(QueueMutex);
		while (!Tasks.empty())
			Tasks.pop();

		ActiveTasks.clear();
	}

	Ref<AsyncTask> Async::Create(void* data)
	{
		auto task = CreateRef<AsyncTask>();
		task->Data = data;
		task->Status = AsyncStatus::Pending;
		{
			std::scoped_lock lock(QueueMutex);
			std::erase_if(ActiveTasks,
				[](const auto& active)
				{
					const AsyncStatus status = active->Status;
					return status != AsyncStatus::Pending && status != AsyncStatus::Running;
				});
			ActiveTasks.push_back(task);
		}
		return task;
	}

	void Async::Submit(std::function<void()> work)
	{
		{
			std::scoped_lock lock(QueueMutex);
			Tasks.push(std::move(work));
		}
		Condition.notify_one();
	}

	void Async::WorkerThread()
	{
		while (Running)
		{
			std::function<void()> work;
			{
				std::unique_lock lock(QueueMutex);
				Condition.wait(lock, [] { return !Tasks.empty() || !Running; });

				if (!Running && Tasks.empty())
					return;

				work = std::move(Tasks.front());
				Tasks.pop();
			}
			try
			{
				work();
			}
			catch (const std::exception& e)
			{
				Log::WriteLine("^1Async exception {}", e.what());
			}
		}
	}
}
