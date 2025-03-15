#pragma once

class ActorInfo;
struct AnimationInfo;

enum class SpellType
{
	kSpell = 0,
	kPower = 2,
	kLesserPower = 3,
	kVoicePower = 11,
	kScroll = 13,
};

enum class CastingType
{
	kConstantEffect = 0,
	kFireAndForget = 1,
	kConcentration = 2,
	kScroll = 3,
};
/// <summary>
/// the way the deliveroo hops to its target
/// </summary>
enum class DeliverooType
{
	kAimed,
	kSelf,
	kWard,
	kConcAimed,
	kConcSelf,
};

enum class CastingOrigin
{
	kLeftHand,
	kRightHand,
	kVoice
};

class SpellInfo
{
private:
	RE::SpellItem* _spell = nullptr;
	uint32_t _formid;

	SpellType _spellType;
	CastingType _castingType;
	DeliverooType _deliveryType;
	RE::BGSPerk* _castingPerk = nullptr;

	bool _valid = false;

	RE::BGSEquipSlot* _equipSlot = nullptr;

	bool _isRitual = false;

	long long _chargeTime;

	void DoCast(std::shared_ptr<ActorInfo> caster, CastingOrigin origin, std::shared_ptr<AnimationInfo> info);
	void NextCast(std::shared_ptr<ActorInfo> caster, CastingOrigin origin, std::shared_ptr<AnimationInfo> info);
	void FurtherCast(std::shared_ptr<ActorInfo> caster, CastingOrigin origin, std::shared_ptr<AnimationInfo> info);

	void SetCastVars(std::shared_ptr<ActorInfo> caster, CastingOrigin origin, std::shared_ptr<AnimationInfo> info);

public:
	SpellInfo(RE::SpellItem* spell);

	float GetMagickaCost(std::shared_ptr<ActorInfo> caster);

	void CastSpell(std::shared_ptr<SpellInfo> self, std::shared_ptr<ActorInfo> caster, CastingOrigin origin,bool dualCast);

	bool IsValid() { return _valid; }

	uint32_t GetFormID() { return _formid; }

	RE::SpellItem* GetSpell() { return _spell; }

	long long GetChargeTime() { return _chargeTime; }
	 
	RE::BGSSoundDescriptorForm* GetSound(RE::MagicSystem::SoundID soundID);
};
