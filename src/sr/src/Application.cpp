#include "Application.hpp"

#include "Audio/Voice.hpp"
#include "Commands/Container.hpp"
#include "Game/Demo/DemoContainer.hpp"
#include "Game/VR.hpp"
#include "System/Debug.hpp"
#include "System/Environment.hpp"
#include "System/Netchan.hpp"

namespace SR
{
	void Application::Start()
	{
		Log::WriteLine("^5[SR] Start");
		Environment::Build();

		Netchan::Initialize();
		Voice::Initialize();
		VR::Initialize();
		DemoContainer::Initialize();
		CommandsContainer::Initialize();
		Debug::Initialize();
	}

	void Application::Shutdown()
	{
		Log::WriteLine("^5[SR] Shutdown");

		Async::Shutdown();
		Voice::Shutdown();
	}
}
