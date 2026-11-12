#pragma once
#include "Demo.hpp"

namespace SR
{
	// A demo replayed beside a player as a second player model. It only exists in the snapshots of that
	// player and of whoever follows them, in a client slot no real player holds.
	class GhostPlayer
	{
	public:
		Ref<class Demo> Demo;
		class Player* Player = nullptr;
		std::string Name;
		std::vector<int> Weapons;
		int StartTime = 0;
		int Lead = 0;
		int FrameIndex = 0;
		int RecordTime = 0;
		int Delta = 0;
		bool HasDelta = false;
		bool Finished = false;

		GhostPlayer(class Player* player);

		void Play(const Ref<class Demo>& demo, const std::string& name, int runTime, int recordTime);
		void Finish(int time);
		void Stop();
		int Snapshot(const client_t* receiver, entityState_t* entity, clientState_t* client);
		void Send(client_t* cl);

		static void Initialize();
		static void State(client_t* cl);
		static int Slot();

	private:
		static inline cvar_t* LeadOffset = nullptr;
		int Progress = 0;
		int TrackTime = 0;

		bool Frame(const client_t* receiver, playerState_t& ps);
		void Track();
		float Nearest(const float* origin, int from, int to, int& index, float& fraction) const;
	};
}
