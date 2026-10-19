#pragma once
#include "Demo.hpp"

#include "Game/Player/Player.hpp"

namespace SR
{
	class DemoPlayer
	{
	public:
		Ref<class Demo> Demo;
		class Player* Player = nullptr;
		gentity_t* Entity = nullptr;
		int Velocity = 0;
		std::string Weapon = "";
		std::string Name = "";
		DemoFrame CurrentFrame;
		int FrameIndex = 0;
		int PreviousFrameIndex = 0;
		double Clock = 0;
		float Rate = 1;
		int LastServerTime = 0;
		bool HasFrame = false;

		DemoPlayer(class Player* player);
		~DemoPlayer() = default;

		void Play(const Ref<class Demo>& demo, const std::string& name);
		void Stop();

		void UpdateEntity(snapshotInfo_t* snapInfo, msg_t* msg, const int time, entityState_t* from, entityState_t* to,
			qboolean force);
		bool ComputeFrame();
		void InterpolateFrame(DemoFrame& frame, const DemoFrame& next, float interpolate);
		int RunTime() const;
		void Send(client_t* cl) const;

		void Packet();
		void Frame();

		static void Snapshot(client_t* cl);
	};
}
