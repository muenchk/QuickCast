#pragma once

#include "SimpleIni.h"
#include <fstream>
#include <iostream>
#include <type_traits>
#include <utility>
#include <string_view>
#include <chrono>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <time.h>
#include <random>
#include <tuple>
#include <vector>

#include <string.h>
#include <thread>
#include <forward_list>
#include <semaphore>
#include <limits>

class Settings
{
public:
	/// <summary>
	/// Name of this plugin
	/// </summary>
	static inline std::string PluginName = "QuickCast.esp";
	/// <summary>
	/// Plain name of this plugin
	/// </summary>
	static inline std::string PluginNamePlain = "QuickCast";

	static inline std::filesystem::path log_directory = "";

	static inline std::filesystem::path file_directory = "";

	/// <summary>
	/// Indicates whether something has been modified
	/// </summary>
	enum class ChangeFlag
	{
		kNone = 1,
		kChanged = 2,
	};

	/// <summary>
	/// flag that specifies updates to run on settings and lists
	/// </summary>
	enum class UpdateFlag
	{
		kNone = 0x1,
		kMagnitude = 0x1 << 1,
		kCompatibility = 0x1 << 2,
		kProhibitedEffects = 0x1 << 3,
	};

	static inline ChangeFlag _modifiedSettings = ChangeFlag::kNone;
	static inline uint32_t _updateSettings = (uint32_t)UpdateFlag::kNone;


	/// <summary>
	/// settings related to the player character
	/// </summary>
	struct Player
	{

	};




	/// <summary>
	/// General settings for compatibility
	/// </summary>
	struct Compatibility
	{
	};

	struct CastOptions
	{
		/// <summary>
		/// [Settings] actor actually performs the casting animation and spells are cast when animation ends
		/// </summary>
		static inline bool _doCast = true;
		/// <summary>
		/// [Settings] scales the casting time by this value
		/// </summary>
		static inline float _castingScaling = 1.0f;
		/// <summary>
		/// [Settings] time the actor stays in the ready animation (after fully charging the spell)
		/// </summary>
		static inline std::chrono::milliseconds _readyTime = std::chrono::milliseconds(100);
	};

	struct Usage
	{
		
		/// <summary>
		/// [Settings] Global cooldown of using items in milliseconds
		/// </summary>
		static inline long _globalCooldown = 1000;
		/// <summary>
		/// [Settings] Whether to disable item usage while an actor is staggered (or otherwise indisponated)
		/// </summary>
		static inline bool _DisableItemUsageWhileStaggered = false;  // disables potion and poison usage while the npc is staggered
		/// <summary>
		/// [Settings] Whether to disable item usage while an actor is in the air
		/// </summary>
		static inline bool _DisableItemUsageWhileFlying = false;
		/// <summary>
		/// [Settings] Whether to disable item usage while an actor is bleeding out
		/// </summary>
		static inline bool _DisableItemUsageWhileBleedingOut = false;
		/// <summary>
		/// [Settings] Whether to disable item usage while an actor is sleeping
		/// </summary>
		static inline bool _DisableItemUsageWhileSleeping = false;
	};

	/// <summary>
	/// Settings for debug features
	/// </summary>
	struct Debug
	{
		/// <summary>
		/// [Setting] Enables general logging
		/// </summary>
		static inline bool EnableLog = false;
		/// <summary>
		/// [Setting] Enables profiling
		/// </summary>
		static inline bool EnableProfiling = false;
	};
	/*
	class Interfaces
	{
	public:
		static inline TDM_API::IVTDM1* tdm_api = nullptr;

		static void RequestAPIs();
	};*/

	// intern
public:

	static inline std::string pluginnames[256+4096];
	static inline std::unordered_map<std::string, uint32_t> pluginNameMap;
	static inline std::unordered_map<uint32_t, std::string> pluginIndexMap;

	
	static inline RE::BGSKeyword* ActorTypeDwarven;
	static inline RE::BGSKeyword* ActorTypeCreature;
	static inline RE::BGSKeyword* ActorTypeAnimal;
	static inline RE::BGSKeyword* Vampire;

	static inline RE::TESFaction* CurrentFollowerFaction;
	static inline RE::TESFaction* CurrentHirelingFaction;

	/// <summary>
	/// Game equip slot for the left hand
	/// </summary>
	static inline RE::BGSEquipSlot* Equip_LeftHand;
	/// <summary>
	/// Game equip slot for the right hand
	/// </summary>
	static inline RE::BGSEquipSlot* Equip_RightHand;
	/// <summary>
	/// Game equip slot for either hand
	/// </summary>
	static inline RE::BGSEquipSlot* Equip_EitherHand;
	/// <summary>
	/// Game equip slot for both hands
	/// </summary>
	static inline RE::BGSEquipSlot* Equip_BothHands;
	/// <summary>
	/// Game equip slots for shields
	/// </summary>
	static inline RE::BGSEquipSlot* Equip_Shield;
	/// <summary>
	/// Game equip slot for voice
	/// </summary>
	static inline RE::BGSEquipSlot* Equip_Voice;
	/// <summary>
	/// Game equip slots for potions
	/// </summary>
	static inline RE::BGSEquipSlot* Equip_Potion;

	/// <summary>
	/// Loads game objects
	/// </summary>
	static void InitGameStuff();

	/// <summary>
	/// Loads the plugin configuration
	/// </summary>
	static void Load();
	
	/// <summary>
	/// Saves the plugin configuration
	/// </summary>
	static void Save();

	/// <summary>
	/// Updates settings that have been changed during runtime and result in changes to classification
	/// </summary>
	static void UpdateSettings();

};
