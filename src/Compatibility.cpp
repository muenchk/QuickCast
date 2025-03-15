#include <semaphore>

#include "Compatibility.h"
#include "Data.h"
#include "Game.h"
#include "Logging.h"
#include "Utility.h"

Compatibility* Compatibility::GetSingleton()
{
	static Compatibility singleton;
	return std::addressof(singleton);
}

std::binary_semaphore sem{ 1 };

void Compatibility::Load()
{
	// get lock to avoid deadlocks (should not occur, since the functions should not be called simultaneously
	sem.acquire();
	Data* data = Data::GetSingleton();
	RE::TESDataHandler* datahandler = RE::TESDataHandler::GetSingleton();

	// NPCsUsePotions
	if (const uint32_t index = Utility::Mods::GetPluginIndex(NPCsUsePotions); index != 0x1) {
		loginfo("Found plugin NPCsUsePotions.esp.");
		_loadedNPCsUsePotions = true;
	}

	// animated poisons
	if (const uint32_t index = Utility::Mods::GetPluginIndex(AnimatedPoisons); index != 0x1) {
		_loadedAnimatedPoisons = true;
		loginfo("Found plugin AnimatedPoisons.esp.");
	}
	if (const uint32_t index = Utility::Mods::GetPluginIndex(AnimatedPoisons_5); index != 0x1) {
		_loadedAnimatedPoisons = true;
		loginfo("Found plugin Animated Poisons.esp.");
	}

	// animated potions
	if (const uint32_t index = Utility::Mods::GetPluginIndex(AnimatedPotions_4_4); index != 0x1){
		_loadedAnimatedPotions = true;
		loginfo("Found plugin {}.",AnimatedPotions_4_4);
	}
	if (const uint32_t index = Utility::Mods::GetPluginIndex(AnimatedPotions_4_3); index != 0x1){
		_loadedAnimatedPotions = true;
		loginfo("Found plugin {}.",AnimatedPotions_4_3);
	}

	// ZUPA
	if (const uint32_t index = Utility::Mods::GetPluginIndex(ZUPA); index != 0x1){
		loginfo("Found plugin {}.", ZUPA);
		_loadedZUPA = true;
	}

	// Sacrosanct
	if (const uint32_t index = Utility::Mods::GetPluginIndex(Sacrosanct); index != 0x1) {
		_loadedSacrosanct = true;
		loginfo("Found plugin Sacrosanct.esp.");
	}

	// Ultimate Animated Potions
	auto findPlugin = [](std::string pluginname) {
		std::wstring wstr = std::wstring(pluginname.begin(), pluginname.end());
		auto pluginHandle = GetModuleHandle(wstr.c_str());
		if (pluginHandle)
			return true;
		return false;
	};

	_loadedUltimatePotions = findPlugin("UAPNG.dll");
	if (_loadedUltimatePotions)
	{
		loginfo("Found Ultimate Animted Potions.");
	}

	// ordinator
	if (const uint32_t index = Utility::Mods::GetPluginIndex(Ordinator); index != 0x1) {
		loginfo("Found plugin Ordinator - Perks of Skyrim.esp.");
		_loadedOrdinator = true;
	}

	// vokrii
	if (const uint32_t index = Utility::Mods::GetPluginIndex(Vokrii); index != 0x1) {
		loginfo("Found plugin Vokrii - Minimalistic Perks of Skyrim.esp.");
		_loadedVokrii = true;
	}

	// adamant
	if (const uint32_t index = Utility::Mods::GetPluginIndex(Adamant); index != 0x1) {
		loginfo("Found plugin Adamant.esp.");
		_loadedAdamant = true;
	}

	// global

	if (_loadedAnimatedPoisons) {
		_disableParalyzedItems = true;
	}
	if (_loadedAnimatedPotions) {
		_disableParalyzedItems = true;
	}
	if (_loadedZUPA) {
		_disableParalyzedItems = true;
	}
	if (_loadedUltimatePotions) {
		_disableParalyzedItems = true;
	}

	sem.release();
}

std::binary_semaphore actorpoisonlock{ 1 };
std::binary_semaphore actorpotionlock{ 1 };

void Compatibility::Clear()
{
	// get lock to avoid deadlocks (should not occur, since the functions should not be called simultaneously
	sem.acquire();

	// NPCsUsePotions
	_loadedNPCsUsePotions = false;

	// animated poisons
	_loadedAnimatedPoisons = false;

	// animated potions
	_loadedAnimatedPotions = false;

	// ZUPA
	_loadedZUPA = false;

	// sacrosanct
	Sac_MockeryOfLife = nullptr;
	_loadedSacrosanct = false;

	// ordinator
	_loadedOrdinator = false;

	// vokrii
	_loadedVokrii = false;

	// adamant
	_loadedAdamant = false;

	// global
	_disableParalyzedItems = false;

	sem.release();
}


void SaveGameCallback(SKSE::SerializationInterface* /*a_intfc*/)
{
	
}

void LoadGameCallback(SKSE::SerializationInterface* /*a_intfc*/)
{
	loginfo("");
	Compatibility::GetSingleton()->Load();
}

void RevertGameCallback(SKSE::SerializationInterface* /*a_intfc*/)
{
	loginfo("");
	Compatibility::GetSingleton()->Clear();
}

void Compatibility::Register()
{
	Game::SaveLoad::GetSingleton()->RegisterForLoadCallback(0xFF000100, LoadGameCallback);
	loginfo("Registered {}", typeid(LoadGameCallback).name());
	Game::SaveLoad::GetSingleton()->RegisterForRevertCallback(0xFF000200, RevertGameCallback);
	loginfo("Registered {}", typeid(RevertGameCallback).name());
	Game::SaveLoad::GetSingleton()->RegisterForSaveCallback(0xFF000300, SaveGameCallback);
	loginfo("Registered {}", typeid(SaveGameCallback).name());
}

bool Compatibility::CannotRestoreHealth(std::shared_ptr<ActorInfo> acinfo)
{
	bool res = false;
	if (_loadedSacrosanct)
		res |= acinfo->HasMagicEffect(Sac_MockeryOfLife);

	return res;
}

bool Compatibility::CannotRestoreMagicka(std::shared_ptr<ActorInfo> acinfo)
{
	return false;
}

bool Compatibility::CannotRestoreStamina(std::shared_ptr<ActorInfo> acinfo)
{
	return false;
}
