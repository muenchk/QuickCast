#include "AnimationEvents.h"
#include "ActorInfo.h"
#include "Utility.h"

RE::BSEventNotifyControl PlayerAnimationHandler::ProcessEvent(const RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>*)
{
	if (!a_event)
		return RE::BSEventNotifyControl::kContinue;

	if (a_event->holder && a_event->holder->IsPlayerRef()) {
		loginfo("\"{}\" | \"{}\"", a_event->tag.c_str(), a_event->payload.c_str());
		auto itr = _eventQueue.begin();
		uint64_t frameTime = 0;
		QueryUnbiasedInterruptTime(&frameTime);
		//loginfo("queue size: {}", _eventQueue.size());
		while (itr != _eventQueue.end()) {
			if ((*itr)->timeout != 0 && (*itr)->timeout < frameTime) {
				(*itr)->Interrupt();
				(*itr)->Clear();
				itr = _eventQueue.erase(itr);
				continue;
			}
			if (a_event->tag == (*itr)->waitinganim.c_str()) {
				loginfo("found event type \"{}\" with payload \"{}\" for {}", (*itr)->waitinganim, a_event->payload.c_str(), Utility::GetHexFill((*itr)->identifier));
				if ((*itr)->GetInterrupt() == false) {
					if ((*itr)->callback != nullptr)
						(*itr)->callback(*itr);
					else
						loginfo("callback empty");
				} else
					loginfo("interrupt");
				itr = _eventQueue.erase(itr);
				continue;
			}
			itr++;
		}
	}

	return RE::BSEventNotifyControl::kContinue;
}

void PlayerAnimationHandler::AddEvent(std::shared_ptr<AnimationInfo> info, uint64_t timeout)
{
	info->timeout = timeout;
	_eventQueue.push_back(info);
}

void PlayerAnimationHandler::AdjustOffsets(uint64_t offset)
{
	for (auto info : _eventQueue) {
		info->timeout += offset;
	}
}


bool PlayerAnimationHandler::RegisterEvents(RE::Actor* actor)
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

PlayerAnimationHandler* PlayerAnimationHandler::GetSingleton()
{
	static PlayerAnimationHandler eventhandler;
	return std::addressof(eventhandler);
}
