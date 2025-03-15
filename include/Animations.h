#pragma once

struct AnimationInfo;

#define TimeMultSeconds 10000000
#define TimeMultMillis 10000

class Animations
{
private:
	static void CastLeft_CastSpell(std::shared_ptr<AnimationInfo> event);
	static void CastRight_CastSpell(std::shared_ptr<AnimationInfo> event);

	static void EquippedPreLeft(std::shared_ptr<AnimationInfo> event);
	static void EquippedPreRight(std::shared_ptr<AnimationInfo> event);
	static void EquippedPostLeft(std::shared_ptr<AnimationInfo> event);
	static void EquippedPostRight(std::shared_ptr<AnimationInfo> event);
	static void EquippedPostDual(std::shared_ptr<AnimationInfo> event);

public:
	/// <summary>
	/// Handles CastAimedLeft events
	/// </summary>
	/// <param name="event"></param>
	/// <returns>whether the animation has finished up with the main thread</returns>
	static bool CastAimedLeft(std::shared_ptr<AnimationInfo> event);
	/// <summary>
	/// Handles CastAimedRight events
	/// </summary>
	/// <param name="event"></param>
	/// <returns>whether the animation has finished up with the main thread</returns>
	static bool CastAimedRight(std::shared_ptr<AnimationInfo> event);

	static bool CastSelfLeft(std::shared_ptr<AnimationInfo> event);
	static bool CastSelfRight(std::shared_ptr<AnimationInfo> event);

	static bool CastConcAimedLeft(std::shared_ptr<AnimationInfo> event);
	static bool CastConcAimedRight(std::shared_ptr<AnimationInfo> event);

	static bool CastConcSelfLeft(std::shared_ptr<AnimationInfo> event);
	static bool CastConcSelfRight(std::shared_ptr<AnimationInfo> event);

	static bool CastWardRight(std::shared_ptr<AnimationInfo> event);
	static bool CastWardLeft(std::shared_ptr<AnimationInfo> event);

	static bool DualCastAimed(std::shared_ptr<AnimationInfo> event);
	static bool DualCastSelf(std::shared_ptr<AnimationInfo> event);
	static bool DualCastConcAimed(std::shared_ptr<AnimationInfo> event);
	static bool DualCastConcSelf(std::shared_ptr<AnimationInfo> event);
	static bool DualCastWard(std::shared_ptr<AnimationInfo> event);

	static bool Shout(std::shared_ptr<AnimationInfo> event);

	static bool Ritual(std::shared_ptr<AnimationInfo> event);
};

