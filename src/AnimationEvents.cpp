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
		while (itr != _eventQueue.end())
		{
			if ((*itr)->timeout != 0 && (*itr)->timeout < frameTime) {
				(*itr)->Interrupt();
				(*itr)->Clear();
				itr = _eventQueue.erase(itr);
				continue;
			}
			if (a_event->tag == (*itr)->waitinganim.c_str())
			{
				loginfo("found event type \"{}\" with payload \"{}\" for {}", (*itr)->waitinganim,a_event->payload.c_str(), Utility::GetHexFill((*itr)->identifier));
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
