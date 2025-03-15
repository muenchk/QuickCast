#include "Settings.h"
#include "Utility.h"
#include <fstream>
#include <iostream>
#include <type_traits>
#include <utility>
#include <string_view>
#include <chrono>
#include <set>
#include <time.h>
#include <random>
#include <tuple>
#include <vector>
#include <unordered_map>
#include "Data.h"
#include "Compatibility.h"
#include "Logging.h"

using Comp = Compatibility;

static std::mt19937 randi((unsigned int)(std::chrono::system_clock::now().time_since_epoch().count()));
/// <summary>
/// trims random numbers to 1 to RR
/// </summary>
static std::uniform_int_distribution<signed> rand100(1, 100);

#pragma region Settings

/*void Settings::Interfaces::RequestAPIs()
{
	loginfo("");
	// get tmp api
	if (!tdm_api) {
		loginfo("Trying to get True Directional Movement API");
		tdm_api = reinterpret_cast<TDM_API::IVTDM1*>(TDM_API::RequestPluginAPI(TDM_API::InterfaceVersion::V1));
		if (tdm_api) {
			loginfo("Acquired True Directional Movement API");
		} else {
			loginfo("Failed to get True Directional Movement API");
		}
	}
}*/

void Settings::InitGameStuff()
{
	loginfo("Load Game Stuff");
	RE::TESDataHandler* datahandler = RE::TESDataHandler::GetSingleton();
	const RE::TESFile* file = nullptr;
	uint32_t index = 0;
	for (int i = 0; i <= 254; i++) {
		file = datahandler->LookupLoadedModByIndex((uint8_t)i);
		if (file) {
			pluginnames[i] = std::string(file->GetFilename());
			index = (uint32_t)i << 24;
			pluginNameMap.insert_or_assign(pluginnames[i], index);
			pluginIndexMap.insert_or_assign(index, pluginnames[i]);
		} else
			pluginnames[i] = "";
	}
	// 0xFF... is reserved for objects created by the game during runtime 
	pluginnames[255] = "runtime";
	for (int i = 0; i <= 4095; i++) {
		file = datahandler->LookupLoadedLightModByIndex((uint16_t)i);
		if (file) {
			pluginnames[256 + i] = std::string(file->GetFilename());
			index = 0xFE000000 | ((uint32_t)i << 12);
			pluginNameMap.insert_or_assign(pluginnames[256 + i], index);
			pluginIndexMap.insert_or_assign(index, pluginnames[256 + i]);
		} else
			pluginnames[256 + i] = "";
	}
	for (auto& [name, id] : pluginNameMap)
		loginfo("ID: {:3}, Name:{}", Utility::GetHex(id), name);

	
	Settings::ActorTypeDwarven = RE::TESForm::LookupByID<RE::BGSKeyword>(0x1397A);
	if (Settings::ActorTypeDwarven == nullptr) {
		loginfo("[INIT] Couldn't find ActorTypeDwarven Keyword in game.");
	}
	Settings::ActorTypeCreature = RE::TESForm::LookupByID<RE::BGSKeyword>(0x13795);
	if (Settings::ActorTypeCreature == nullptr) {
		loginfo("[INIT] Couldn't find ActorTypeCreature Keyword in game.");
	}
	Settings::ActorTypeAnimal = RE::TESForm::LookupByID<RE::BGSKeyword>(0x13798);
	if (Settings::ActorTypeAnimal == nullptr) {
		loginfo("[INIT] Couldn't find ActorTypeAnimal Keyword in game.");
	}
	Settings::Vampire = RE::TESForm::LookupByID<RE::BGSKeyword>(0xA82BB);
	if (Settings::Vampire == nullptr) {
		loginfo("[INIT] Couldn't find Vampire Keyword in game.");
	}

	Settings::CurrentFollowerFaction = RE::TESForm::LookupByID<RE::TESFaction>(0x0005C84E);
	if (Settings::CurrentFollowerFaction == nullptr) {
		loginfo("[INIT] Couldn't find CurrentFollowerFaction Faction in game.");
	}
	Settings::CurrentHirelingFaction = RE::TESForm::LookupByID<RE::TESFaction>(0xbd738);
	if (Settings::CurrentHirelingFaction == nullptr) {
		loginfo("[INIT] Couldn't find CurrentHirelingFaction Faction in game.");
	}

	Settings::Equip_LeftHand = RE::TESForm::LookupByID<RE::BGSEquipSlot>(0x13F43);
	Settings::Equip_RightHand = RE::TESForm::LookupByID<RE::BGSEquipSlot>(0x13F42);
	Settings::Equip_EitherHand = RE::TESForm::LookupByID<RE::BGSEquipSlot>(0x13F44);
	Settings::Equip_BothHands = RE::TESForm::LookupByID<RE::BGSEquipSlot>(0x13F45);
	Settings::Equip_Shield = RE::TESForm::LookupByID<RE::BGSEquipSlot>(0x141E8);
	Settings::Equip_Voice = RE::TESForm::LookupByID<RE::BGSEquipSlot>(0x25BEE);
	Settings::Equip_Potion = RE::TESForm::LookupByID<RE::BGSEquipSlot>(0x35698);


	loginfo("Finished");
}


void Settings::UpdateSettings()
{
	loginfo("Apply configuration changes");
	uint32_t flag = Settings::_updateSettings;
	Settings::_updateSettings = 0;
	if (flag & (uint32_t)UpdateFlag::kCompatibility)
	{
		::Compatibility::GetSingleton()->Clear();
		::Compatibility::GetSingleton()->Load();
	}
}

#pragma endregion
