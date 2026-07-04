#pragma once

#include <memory>

struct AnimationEvent;
struct AnimationInfo;

class PlayerAnimationHandler : public RE::BSTEventSink<RE::BSAnimationGraphEvent>
{
private:
	std::list<std::shared_ptr<AnimationInfo>> _eventQueue;

public:
	static PlayerAnimationHandler* GetSingleton();

	virtual RE::BSEventNotifyControl ProcessEvent(const RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource);

	static bool RegisterEvents(RE::Actor* actor);

	void AddEvent(std::shared_ptr<AnimationInfo> info, uint64_t timeout);

	void AdjustOffsets(uint64_t offset);
};
