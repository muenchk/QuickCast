#include "ID.h"

#include <mutex>
#include <memory>
#include <chrono>

#include "AlchemyEffect.h"

#define aclock ((void)0);  //std::lock_guard<std::mutex> guard(mutex);

class Compatibility;

#pragma once

enum class CombatState
{
	OutOfCombat = 0,
	InCombat = 1 << 0,
	Searching = 1 << 1,
};

enum class EventType
{
	CastAimedRight,
	CastAimedLeft,
	CastSelfRight,
	CastSelfLeft,
	CastConcAimedRight,
	CastConcAimedLeft,
	CastConcSelfRight,
	CastConcSelfLeft,

	CastWardRight,
	CastWardLeft,

	DualCastAimed,
	DualCastSelf,
	DualCastConcAimed,
	DualCastConcSelf,
	DualCastWard,

	Shout,

	Ritual,
};

class SpellInfo;

class ActorInfo;


struct AnimationInfo
{
private:
	bool interrupt = false;

public:
	float consumedMagicka = 0;

	unsigned long long ticks;
	unsigned long long timeout;
	uint64_t identifier = 0;
	std::string waitinganim;
	std::function<void(std::shared_ptr<AnimationInfo>)> callback = nullptr;

	std::function<void(bool)> exitcallback = nullptr;

	std::shared_ptr<ActorInfo> acinfo;
	std::shared_ptr<SpellInfo> spell;

	bool sheathed = false;
	RE::TESForm* equippedleft = nullptr;
	RE::TESForm* equippedright = nullptr;
	bool equippedTwoHanded = false;

	RE::BSSoundHandle activeSound;

	EventType event;
	int32_t animstage = 0;

	AnimationInfo()
	{
		identifier = std::chrono::steady_clock::now().time_since_epoch().count();
	}

	void Clear()
	{
		if (exitcallback)
			exitcallback(!interrupt);
		exitcallback = nullptr;
		callback = nullptr;
		equippedleft = nullptr;
		equippedright = nullptr;
		acinfo.reset();
		spell.reset();
		if (activeSound.IsValid() && activeSound.IsPlaying()) {
			activeSound.Stop();
		}
	}
	void Interrupt();
	bool GetInterrupt()
	{
		return interrupt;
	}
};
/// <summary>
/// Stores information about an actor.
/// </summary>
class ActorInfo
{

#pragma region static

private:
	/// <summary>
	/// current position of the player character, for faster access
	/// </summary>
	static inline RE::NiPoint3 playerPosition;
	/// <summary>
	/// PlayerRef
	/// </summary>
	static inline RE::Actor* playerRef = RE::PlayerCharacter::GetSingleton();

public:
	/// <summary>
	/// Sets the current position of the player character
	/// </summary>
	/// <param name="position"></param>
	static void SetPlayerPosition(RE::NiPoint3 position) { playerPosition = position; }
	/// <summary>
	/// Inits static class data
	/// </summary>
	static void Init();
#pragma endregion

#pragma region runtime
public:

private:
	/// <summary>
	/// central lock coordinating all function accesses
	/// </summary>
	std::mutex mutex;

	/// <summary>
	/// The actor
	/// </summary>
	RE::ActorHandle actor;
	/// <summary>
	/// form id of the actor
	/// </summary>
	ID formid;
	/// <summary>
	/// pluginname the actor is defined in
	/// </summary>
	std::string pluginname = "";
	/// <summary>
	/// ID of the plugin in the current loadorder [runtime]
	/// </summary>
	uint32_t pluginID = 0;
	/// <summary>
	/// name of the actor
	/// </summary>
	std::string name = "";

	/// <summary>
	/// global cooldown
	/// </summary>
	int globalCooldownTimer = 0;

	/// <summary>
	/// if the actor is a automaton
	/// </summary>
	bool _automaton = false;
	/// <summary>
	/// if the actor is a vampire
	/// </summary>
	bool _vampire = false;

	/// <summary>
	/// while the actor is busy with one animation, no other animation should be prepared / played
	/// </summary>
	bool Animation_busy_left = false;
	std::shared_ptr<AnimationInfo> _animleft;
	/// <summary>
	/// while the actor is busy with one animation, no other animation should be played
	/// </summary>
	bool Animation_busy_right = false;
	std::shared_ptr<AnimationInfo> _animright;

	/// <summary>
	/// Whether the NPC has been whitelisted
	/// </summary>
	bool whitelisted = false;
	/// <summary>
	/// Whether it has been calculated whether the npc is whitelisted
	/// </summary>
	bool whitelistedcalculated = false;

	/// <summary>
	/// Combat state of the actor
	/// </summary>
	CombatState combatstate = CombatState::OutOfCombat;

	/// <summary>
	/// whether the actor has a lefthand slot
	/// </summary>
	bool _haslefthand = false;

	/// <summary>
	/// the permanent poison resistance of an actor
	/// </summary>
	int _permanentPoisonResist = 0;

	// temporary targeting variables
	
	// own combat data
	uint32_t combatdata = 0;
	// target combat data
	uint32_t tcombatdata = 0;
	// current target
	std::weak_ptr<ActorInfo> target;
	// whether to process the actor
	bool handleactor = true;
	// distance to player
	float playerDistance = 0;
	// hostile to player
	bool playerHostile = false;
	// whether the weapons are drawn
	bool weaponsDrawn = false;

	
	/// <summary>
	/// string that represents this object (for fast usage)
	/// </summary>
	std::string _formstring;

	/// <summary>
	/// timestamp when the object was invalidated
	/// </summary>
	long long timestamp_invalid;


public:
	/// <summary>
	/// version of class [used for save and load]
	/// </summary>
	static inline const uint32_t version = 0x00000010;

	ActorInfo(RE::Actor* _actor);
	ActorInfo(bool blockedReset);
	ActorInfo();

	/// <summary>
	/// returns a string that represents the actor
	/// </summary>
	/// <returns></returns>
	std::string ToString();
	

	~ActorInfo()
	{
	}

private:
	/// <summary>
	/// if [true] the ActorInfo is valid and can be used, if [false] the ActorInfo is a dummy object
	/// </summary>
	bool valid = false;
	/// <summary>
	/// Whether the actor has been deleted;
	/// </summary>
	bool dead = false;
	/// <summary>
	/// Blocks resetting this info
	/// </summary>
	bool blockReset = false;

	/// <summary>
	/// Updates certain actor metrics
	/// [Should only be called, directly after updating the actor value]
	/// </summary>
	void UpdateMetrics(RE::ActorHandle handle);

public:

	/// <summary>
	/// Returns the string representation of the ActorInfo
	/// </summary>
	std::string GetFormString() { return _formstring; }

	/// <summary>
	/// Returns whether the ActorInfo is valid
	/// </summary>
	bool IsValid();
	/// <summary>
	/// Sets the ActorInfo to valid
	/// </summary>
	void SetValid();
	/// <summary>
	/// Sets the ActorInfo to invalid
	/// </summary>
	void SetInvalid();
	/// <summary>
	/// return whether the object is invalid and expired
	/// </summary>
	bool IsExpired();
	/// <summary>
	/// Sets the ActorInfo to deleted
	/// </summary>
	void SetDead();
	/// <summary>
	/// Returns whether the actor has been deleted
	/// </summary>
	bool GetDead();

	/// <summary>
	/// Resets the actorinfo to default values
	/// </summary>
	/// <param name="actor"></param>
	void Reset(RE::Actor* _actor);

	/// <summary>
	/// Returns the underlying actor object
	/// </summary>
	/// <returns></returns>
	RE::Actor* GetActor();
	/// <summary>
	/// Returns the underlying actor handle
	/// </summary>
	/// <returns></returns>
	RE::ActorHandle GetHandle();
	/// <summary>
	/// Returns the formid
	/// </summary>
	/// <returns></returns>
	RE::FormID GetFormID();
	/// <summary>
	/// Returns the formid without checking for validity
	/// </summary>
	/// <returns></returns>
	RE::FormID GetFormIDBlank();
	/// <summary>
	/// Returns the original formid
	/// </summary>
	/// <returns></returns>
	RE::FormID GetFormIDOriginal();
	/// <summary>
	/// Returns all known formids of the actors templates
	/// </summary>
	/// <returns></returns>
	std::vector<RE::FormID> GetTemplateIDs();
	/// <summary>
	/// Returns the name of the plugin the actor is defined in
	/// </summary>
	/// <returns></returns>
	std::string GetPluginname();
	/// <summary>
	/// Returns the ID of the plugin in the current loadorder [runtime]
	/// </summary>
	/// <returns></returns>
	uint32_t GetPluginID();
	/// <summary>
	/// Returns the name of the actor
	/// </summary>
	/// <returns></returns>
	std::string GetName();

	/// <summary>
	/// Returns whether the actor is currently in an animation
	/// </summary>
	/// <returns></returns>
	bool IsAnimationBusyLeft() { return Animation_busy_left; }
	/// <summary>
	/// Returns whether the actor is currently in an animation
	/// </summary>
	/// <returns></returns>
	bool IsAnimationBusyRight() { return Animation_busy_right; }
	/// <summary>
	/// Sets whether the actor is currently in an animation
	/// </summary>
	/// <param name="value"></param>
	void SetAnimationBusyLeft(bool value) { Animation_busy_left = value; }
	/// <summary>
	/// Sets whether the actor is currently in an animation
	/// </summary>
	/// <param name="value"></param>
	void SetAnimationBusyRight(bool value) { Animation_busy_right = value; }
	void SetAnimationLeft(std::shared_ptr<AnimationInfo> info) { _animleft = info; }
	void SetAnimationRight(std::shared_ptr<AnimationInfo> info) { _animright = info; }
	void InterruptAnimationLeft()
	{
		if (_animleft)
			_animleft->Interrupt();
		_animleft.reset();
		Animation_busy_left = false;
	}
	void InterruptAnimationRight()
	{
		if (_animright)
			_animright->Interrupt();
		_animright.reset();
		Animation_busy_right = false;
	}
	void ResetAnimationStatusLeft()
	{
		Animation_busy_left = false;
		_animleft.reset();
	}
	void ResetAnimationStatusRight()
	{
		Animation_busy_right = false;
		_animright.reset();
	}

	/// <summary>
	/// Return the global cooldown
	/// </summary>
	int GetGlobalCooldownTimer() { return globalCooldownTimer; }
	/// <summary>
	/// Set the global cooldown
	/// </summary>
	void SetGlobalCooldownTimer(int value) { globalCooldownTimer = value; }
	/// <summary>
	/// Decreases the global cooldown
	/// </summary>
	void DecGlobalCooldownTimer(int value) { globalCooldownTimer -= value; }

	/// <summary>
	/// Return whether the actor is whitelisted
	/// </summary>
	bool IsWhitelisted() { return whitelisted; }
	/// <summary>
	/// Set that the actor is whitelisted
	/// </summary>
	void SetWhitelisted() { whitelisted = true; }
	/// <summary>
	/// Returns whether the whitelist status of the actors has been calculated
	/// </summary>
	bool IsWhitelistCalculated() { return whitelistedcalculated; }
	/// <summary>
	/// Set that the whitelist status of the actor has been calculated
	/// </summary>
	void SetWhitelistCalculated() { whitelistedcalculated = true; }

	/// <summary>
	/// Whether the NPC is currently in combat
	/// </summary>
	bool IsInCombat();
	/// <summary>
	/// Returns the combat state of the actor
	/// </summary>
	/// <returns></returns>
	CombatState GetCombatState();
	/// <summary>
	/// Sets the combatstate of the npc
	/// </summary>
	/// <returns></returns>
	void SetCombatState(CombatState state);
	/// <summary>
	/// Whether an NPC has drawn their weapons
	/// </summary>
	/// <returns></returns>
	bool IsWeaponDrawn();
	/// <summary>
	/// Returns whether the actor is a vampire
	/// </summary>
	/// <returns></returns>
	bool IsVampire() { return _vampire; }
	/// <summary>
	/// Returns whether the actor is an automaton
	/// </summary>
	/// <returns></returns>
	bool IsAutomaton() { return _automaton; }

	/// <summary>
	/// Returns the actors current target
	/// </summary>
	/// <returns></returns>
	std::weak_ptr<ActorInfo> GetTarget();
	/// <summary>
	/// Resets the current target
	/// </summary>
	void ResetTarget();
	/// <summary>
	/// Set the currebt combat target
	/// </summary>
	void SetTarget(std::weak_ptr<ActorInfo> tar);
	/// <summary>
	/// Returns the level of the current target
	/// </summary>
	short GetTargetLevel();
	/// <summary>
	/// Returns the combat data of the actor
	/// </summary>
	uint32_t GetCombatData();
	/// <summary>
	/// Sets the combat data of the actor
	/// </summary>
	void SetCombatData(uint32_t data);
	/// <summary>
	/// Returns the combat data of the current target
	/// </summary>
	uint32_t GetCombatDataTarget();
	/// <summary>
	/// Set the combat data of the current target
	/// </summary>
	void SetCombatDataTarget(uint32_t data);
	/// <summary>
	/// Returns whether the actor should be handled
	/// </summary>
	bool GetHandleActor();
	/// <summary>
	/// Sets whether to handle the actor
	/// </summary>
	void SetHandleActor(bool handle);
	/// <summary>
	/// Returns the distance to the player
	/// </summary>
	float GetPlayerDistance();
	/// <summary>
	/// Sets the dstance to the player
	/// </summary>
	void SetPlayerDistance(float distance);
	/// <summary>
	/// Returns whether the actor is hostile to the player [intern]
	/// </summary>
	bool GetPlayerHostile();
	/// <summary>
	/// Set wether the actor is hostile to the player [intern]
	/// </summary>
	void SetPlayerHostile(bool hostile);
	/// <summary>
	/// Returns whether the actors weapons are drawn [intern]
	/// </summary>
	bool GetWeaponsDrawn();
	/// <summary>
	/// Set whether the actor has his weapons drawn [intern]
	/// </summary>
	/// <param name="drawn">Whether the weapons are drawn</param>
	void SetWeaponsDrawn(bool drawn);
	/// <summary>
	/// Updates the weapon drawn state of the actor [intern]
	/// </summary>
	void UpdateWeaponsDrawn();

	/// <summary>
	/// Returns a graph variable in the second parameter
	/// </summary>
	/// <param name="a_variableName"></param>
	/// <param name="a_out"></param>
	/// <returns></returns>
	bool GetGraphVariableBool(const RE::BSFixedString& a_variableName, bool& a_out);
	/// <summary>
	/// Set a graph variable
	/// </summary>
	/// <param name="a_variable"></param>
	/// <param name="a_in"></param>
	/// <returns></returns>
	bool SetGraphVariableBool(const RE::BSFixedString& a_variable, bool a_in);
	/// <summary>
	/// Returns a graph variable in the second parameter
	/// </summary>
	/// <param name="a_variableName"></param>
	/// <param name="a_out"></param>
	/// <returns></returns>
	bool GetGraphVariableInt(const RE::BSFixedString& a_variableName, int32_t& a_out);
	/// <summary>
	/// Set a graph variable
	/// </summary>
	/// <param name="a_variable"></param>
	/// <param name="a_in"></param>
	/// <returns></returns>
	bool SetGraphVariableInt(const RE::BSFixedString& a_variable, int32_t a_in);
	/// <summary>
	/// Returns a graph variable in the second parameter
	/// </summary>
	/// <param name="a_variableName"></param>
	/// <param name="a_out"></param>
	/// <returns></returns>
	bool GetGraphVariableFloat(const RE::BSFixedString& a_variableName, float& a_out);
	/// <summary>
	/// Set a graph variable
	/// </summary>
	/// <param name="a_variable"></param>
	/// <param name="a_in"></param>
	/// <returns></returns>
	bool SetGraphVariableFloat(const RE::BSFixedString& a_variable, float a_in);
	/// <summary>
	/// Begin animation
	/// </summary>
	/// <param name="a_eventName"></param>
	/// <returns></returns>
	bool NotifyAnimationGraph(const RE::BSFixedString& a_eventName);

	void CastSpellImmediate(RE::SpellItem* spell, RE::Actor* target, RE::MagicSystem::CastingSource castsource, bool consumeMagicka = true);

	/// <summary>
	/// Returns the permanent Poison Resistance of the actor
	/// </summary>
	/// <returns></returns>
	int GetPermanentPoisonResist() { return _permanentPoisonResist; }
	/// <summary>
	/// Updates the permanent Poison Resistance value of the actor
	/// </summary>
	void UpdatePermanentPoisonResist();

	/// <summary>
	/// Returns the version of the class
	/// </summary>
	/// <returns></returns>
	static uint32_t GetVersion();

	/// <summary>
	/// Returns the save size of the object in bytes
	/// </summary>
	/// <returns></returns>
	int32_t GetDataSize();
	/// <summary>
	/// Returns the minimal save size of the object in bytes
	/// </summary>
	/// <param name="version"></param>
	/// <returns></returns>
	int32_t GetMinDataSize(int32_t version);
	/// <summary>
	/// Writes the object information to the given buffer
	/// </summary>
	/// <param name="buffer">buffer to write to</param>
	/// <param name="offset">offset at which writing will begin</param>
	/// <returns>Whether the data was successfully written</returns>
	bool WriteData(unsigned char* buffer, size_t offset);
	/// <summary>
	/// Reads the object information from the given data
	/// </summary>
	/// <param name="buffer">buffer to read from</param>
	/// <param name="offset">offset in the buffer where the read operation will begin</param>
	/// <param name="length">maximal length to read</param>
	/// <returns>Whether the read operation was successful</returns>
	bool ReadData(unsigned char* buffer, size_t offset, size_t length);

	/// <summary>
	/// Updates the actor and whether the ActorInfo is valid
	/// </summary>
	void Update();

#pragma endregion

#pragma region ActorSpecificFunctions

	/// <summary>
	/// Returns whether the actor is follower
	/// </summary>
	/// <returns></returns>
	bool IsFollower();

	/// <summary>
	/// Returns whether the actor is the player character
	/// </summary>
	/// <returns></returns>
	bool IsPlayer();

	/// <summary>
	/// Returns whether the 3D of the actor is loaded
	/// </summary>
	/// <returns></returns>
	bool Is3DLoaded();

	/// <summary>
	/// Returns the inventory of the actor
	/// </summary>
	/// <returns></returns>
	RE::TESObjectREFR::InventoryItemMap GetInventory();

	/// <summary>
	/// Returns the inventory counts of the actor
	/// </summary>
	/// <returns></returns>
	RE::TESObjectREFR::InventoryCountMap GetInventoryCounts();

	/// <summary>
	/// Returns whether the magic effect is applied to the actor
	/// </summary>
	/// <param name="effect"></param>
	/// <returns></returns>
	bool HasMagicEffect(RE::EffectSetting* effect);

	/// <summary>
	/// Makes the actor drink the potion
	/// </summary>
	/// <param name="a_potion"></param>
	/// <param name="a_extralist"></param>
	/// <returns></returns>
	bool DrinkPotion(RE::AlchemyItem* potion, RE::ExtraDataList* extralist);

	/// <summary>
	/// Returns the inventory entry data for the specified hand
	/// </summary>
	/// <param name="leftHand"></param>
	/// <returns></returns>
	RE::InventoryEntryData* GetEquippedEntryData(bool leftHand);
	RE::TESForm* GetEquippedObject(bool leftHand);

	bool EquipItem(bool lefthand, RE::TESBoundObject* object);
	bool EquipSpell(RE::SpellItem* spell, RE::BGSEquipSlot* slot);
	bool EquipShout(RE::TESShout* shout);

	/// <summary>
	/// Removes an item from the actors inventory
	/// </summary>
	/// <param name="item"></param>
	/// <param name="count"></param>
	void RemoveItem(RE::TESBoundObject* item, int32_t count);

	/// <summary>
	/// Adds an item to the actors inventory
	/// </summary>
	/// <returns></returns>
	void AddItem(RE::TESBoundObject* item, int32_t count);

	/// <summary>
	/// Returns the formflags
	/// </summary>
	/// <returns></returns>
	uint32_t GetFormFlags();

	/// <summary>
	/// Returns whether the actor is dead
	/// </summary>
	/// <returns></returns>
	bool IsDead();

	/// <summary>
	/// Returns the actors base
	/// </summary>
	/// <returns></returns>
	RE::TESNPC* GetActorBase();

	/// <summary>
	/// Returns the FormID of the actorbase
	/// </summary>
	/// <returns></returns>
	RE::FormID GetActorBaseFormID();

	/// <summary>
	/// Returns the EditorID of the actorbase
	/// </summary>
	/// <returns></returns>
	std::string GetActorBaseFormEditorID();

	/// <summary>
	/// Returns the actors combat style
	/// </summary>
	/// <returns></returns>
	RE::TESCombatStyle* GetCombatStyle();

	/// <summary>
	/// Returns the actors race
	/// </summary>
	RE::TESRace* GetRace();

	/// <summary>
	/// Returns the FormID of the actors race
	/// </summary>
	RE::FormID GetRaceFormID();

	/// <summary>
	/// Returns whether the actor is a Ghost
	/// </summary>
	bool IsGhost();

	/// <summary>
	/// Returns whether the actor is summonable
	/// </summary>
	bool IsSummonable();

	/// <summary>
	/// Returns whether the actor bleeds
	/// </summary>
	bool Bleeds();

	/// <summary>
	/// Returns the actors level
	/// </summary>
	short GetLevel();

	/// <summary>
	/// Returns the boolbits of the actor
	/// </summary>
	SKSE::stl::enumeration<RE::Actor::BOOL_BITS, uint32_t> GetBoolBits();

	/// <summary>
	/// Returns whether the actor is flying
	/// </summary>
	bool IsFlying();

	/// <summary>
	/// Returns whether the actor is in a kill move
	/// </summary>
	bool IsInKillMove();

	/// <summary>
	/// Returns whether the actor is in midair
	/// </summary>
	bool IsInMidair();

	/// <summary>
	/// Returns whether the actor is ragdolling
	/// </summary>
	bool IsInRagdollState();

	/// <summary>
	/// Returns whether the actor is unconcious
	/// </summary>
	bool IsUnconscious();

	/// <summary>
	/// Returns whether the actor is paralyzed
	/// </summary>
	bool IsParalyzed();

	/// <summary>
	/// Returns whether the actor is staggered
	/// </summary>
	bool IsStaggered();

	/// <summary>
	/// Returns whether the actor is bleeding out
	/// </summary>
	bool IsBleedingOut();

	/// <summary>
	/// Returns whether the actor is sleeping
	/// </summary>
	/// <returns></returns>
	bool IsSleeping();

	/// <summary>
	/// Returns whether there are any effects applied to the actor that originate in a poison item
	/// </summary>
	/// <returns></returns>
	bool IsPoisoned();

	void DamageActorValue(RE::ActorValue av, float value);
	void RestoreActorValue(RE::ActorValue av, float value);

	float GetAV(RE::ActorValue av);

#pragma endregion
};

