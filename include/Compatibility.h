#pragma once
#include "Settings.h"
#include <unordered_map>
#include "ActorInfo.h"

class Compatibility
{
public:


	// NPCsUsePotions
	std::string NPCsUsePotions = "NPCsUsePotions.esp";

	// ordinator
	static inline std::string Ordinator = "Ordinator - Perks of Skyrim.esp";

	// Vokrii
	static inline std::string Vokrii = "Vokrii - Minimalistic Perks of Skyrim.esp";
	static inline RE::BGSPerk* ConcPoison2;
	static inline RE::BGSPerk* ConcPoison3;

	// Adamant
	static inline std::string Adamant = "Adamant.esp";
	// animated poisons
	static inline std::string AnimatedPoisons = "AnimatedPoisons.esp";
	static inline std::string AnimatedPoisons_5 = "Animated Poisons.esp";

	// animated potions
	static inline std::string AnimatedPotions_4_4 = "Animated Potions.esp";
	static inline std::string AnimatedPotions_4_3 = "AnimatedPotions.esp";

	// ZUPA
	static inline std::string ZUPA = "zxlice's ultimate potion animation.esp";

	// Sacrosanct
	static inline std::string Sacrosanct = "Sacrosanct - Vampires of Skyrim.esp";
	RE::EffectSetting* Sac_MockeryOfLife = nullptr;

	// Ultimate Animated Potions
	int Ult_GlobalCooldown = 2500;

	// general section
private:
	/// <summary>
	/// Whether Ordinator plugin is loaded
	/// </summary>
	bool _loadedOrdinator = false;
	/// <summary>
	/// Whether Vokrii plugin is loaded
	/// </summary>
	bool _loadedVokrii = false;
	/// <summary>
	/// Whether Adamant plugin is loaded
	/// </summary>
	bool _loadedAdamant = false;
	/// <summary>
	/// Whether Sacrosanct is present in the game
	/// </summary>
	bool _loadedSacrosanct = false;
	/// <summary>
	/// Whether NPCsUsePotions plugin is loaded
	/// </summary>
	bool _loadedNPCsUsePotions = false;
	/// <summary>
	/// Whether Ultimate Animated Potions is loaded
	/// </summary>
	bool _loadedUltimatePotions = false;
	/// <summary>
	/// Whether all objects for Animated Poisons have been found
	/// </summary>
	bool _loadedAnimatedPoisons = false;
	/// <summary>
	/// Whether all objects for Animated Potions have been found
	/// </summary>
	bool _loadedAnimatedPotions = false;
	/// <summary>
	/// Whether all objects for zxlice's Ultimate Potion Animation have been found
	/// </summary>
	bool _loadedZUPA = false;

	/// <summary>
	/// whether item usage while paralyzed is disabled
	/// </summary>
	bool _disableParalyzedItems = false;


public:
	/// <summary>
	/// Returns a static Compatibility object
	/// </summary>
	/// <returns></returns>
	static Compatibility* GetSingleton();

	/// <summary>
	/// Loads all game items for the mods, and verifies that compatibility has been established
	/// </summary>
	void Load();

	/// <summary>
	/// Clears all loaded data from memory
	/// </summary>
	void Clear();

	/// <summary>
	/// Returns whether compatibility for Ordinator is enabled
	/// </summary>
	/// <returns></returns>
	bool LoadedOrdinator()
	{
		return _loadedOrdinator;
	}

	/// <summary>
	/// Returns whether compatibility for Vokrii is enabled
	/// </summary>
	/// <returns></returns>
	bool LoadedVokrii()
	{
		return _loadedVokrii;
	}

	/// <summary>
	/// Returns whether compatibility for Adamant is enabled
	/// </summary>
	/// <returns></returns>
	bool LoadedAdamant()
	{
		return _loadedAdamant;
	}

	/// <summary>
	/// returns whether the compatibilty for Sacrosanct is enabled
	/// </summary>
	/// <returns></returns>
	bool LoadedSacrosanct()
	{
		return _loadedSacrosanct;
	}

	/// <summary>
	/// Returns whether NPCsUsePotions plugin is loaded
	/// </summary>
	/// <returns></returns>
	bool LoadedNPCsUsePotions()
	{
		return _loadedNPCsUsePotions;
	}

	/// <summary>
	/// returns whether compatibility for animated potions is enabled
	/// </summary>
	/// <returns></returns>
	bool LoadedAnimatedPotions()
	{
		return _loadedAnimatedPotions;
	}

	/// <summary>
	/// returns whether compatibility for animated poisons is enabled
	/// </summary>
	/// <returns></returns>
	bool LoadedAnimatedPoisons()
	{
		return _loadedAnimatedPoisons;
	}

	/// <summary>
	/// returns whether the compatibility for ZUPA is enabled
	/// </summary>
	/// <returns></returns>
	bool LoadedZUPA()
	{
		return _loadedZUPA;
	}

	/// <summary>
	/// returns whether Ultimate Animated Potions has been loaded
	/// </summary>
	/// <returns></returns>
	bool LoadedUltimatePotions()
	{
		return _loadedUltimatePotions;
	}

	/// <summary>
	/// returns whether item usage should be disabled while an actor is paralyzed, considering the settings and the loaded plugins
	/// </summary>
	/// <returns></returns>
	bool DisableItemUsageWhileParalyzed()
	{
		return Settings::Usage::_DisableItemUsageWhileStaggered || _disableParalyzedItems;
	}

	/// <summary>
	/// returns whether item usage should be disabled while an actor is in the air
	/// </summary>
	/// <returns></returns>
	bool DisableItemUsageWhileFlying()
	{
		return Settings::Usage::_DisableItemUsageWhileFlying || _disableParalyzedItems;
	}

	/// <summary>
	/// returns whether item usage should be disabled while an actor is bleeding out
	/// </summary>
	/// <returns></returns>
	bool DisableItemUsageWhileBleedingOut()
	{
		return Settings::Usage::_DisableItemUsageWhileBleedingOut || _disableParalyzedItems;
	}

	/// <summary>
	/// returns whether item usage should be disabled while an actor is sleeping
	/// </summary>
	/// <returns></returns>
	bool DisableItemUsageWhileSleeping()
	{
		return Settings::Usage::_DisableItemUsageWhileSleeping || _disableParalyzedItems;
	}

	/// <summary>
	/// Returns whether usage of health potions is impared
	/// </summary>
	/// <returns></returns>
	bool CannotRestoreHealth(std::shared_ptr<ActorInfo> acinfo);
	/// <summary>
	/// Returns whether usage of magicka potions is impared
	/// </summary>
	/// <returns></returns>
	bool CannotRestoreMagicka(std::shared_ptr<ActorInfo> acinfo);
	/// <summary>
	/// Returns whether usage of stamina potions is impared
	/// </summary>
	/// <returns></returns>
	bool CannotRestoreStamina(std::shared_ptr<ActorInfo> acinfo);

	/// <summary>
	/// Registers Game Callbacks
	/// </summary>
	static void Register();
};
