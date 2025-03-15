#pragma once

#include "Logging.h"
struct AnimationEvent;

class PlayerAnimationHandler : public RE::BSTEventSink<RE::BSAnimationGraphEvent>
{
private:
	std::list<std::shared_ptr<AnimationInfo>> _eventQueue;

public:
	static PlayerAnimationHandler* GetSingleton()
	{
		static PlayerAnimationHandler eventhandler;
		return std::addressof(eventhandler);
	}

	virtual RE::BSEventNotifyControl ProcessEvent(const RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource);

	static bool RegisterEvents(RE::Actor* actor)
	{
		static PlayerAnimationHandler eventhandler;

		RE::BSAnimationGraphManagerPtr graphMgr;

		if (actor)
			actor->GetAnimationGraphManager(graphMgr);

		if (!graphMgr || !graphMgr->graphs.cbegin()) {
			logcritical("Player Graph not found!");
			return false;
		}

		graphMgr->graphs.cbegin()->get()->AddEventSink(PlayerAnimationHandler::GetSingleton());

		loginfo("Register Animation Event Handler!");

		return true;
	}

	void AddEvent(std::shared_ptr<AnimationInfo> info, uint64_t timeout)
	{
		info->timeout = timeout;
		_eventQueue.push_back(info);
	}

	void AdjustOffsets(uint64_t offset)
	{
		for (auto info : _eventQueue)
		{
			info->timeout += offset;
		}
	}
};
