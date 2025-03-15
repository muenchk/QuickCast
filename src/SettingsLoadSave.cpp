#include "Settings.h"
#include "Logging.h"
#include "Utility.h"
#include "Compatibility.h"

using Comp = Compatibility;

void Settings::Load()
{
	constexpr auto path = L"Data/SKSE/Plugins/NPCsUsePotions.ini";

	bool Ultimateoptions = false;

	CSimpleIniA ini;

	ini.SetUnicode();
	ini.LoadFile(path);

	// read settings
	{
		// player

		// usage
		Usage::_DisableItemUsageWhileStaggered = ini.GetBoolValue("Usage", "DisableItemUsageWhileStaggered", Usage::_DisableItemUsageWhileStaggered);
		loginfo("Setting: {} {}", "usage:             DisableItemUsageWhileStaggered", std::to_string(Usage::_DisableItemUsageWhileStaggered));
		Usage::_DisableItemUsageWhileFlying = ini.GetBoolValue("Usage", "DisableItemUsageWhileFlying", Usage::_DisableItemUsageWhileFlying);
		loginfo("Setting: {} {}", "usage:             DisableItemUsageWhileFlying", std::to_string(Usage::_DisableItemUsageWhileFlying));
		Usage::_DisableItemUsageWhileBleedingOut = ini.GetBoolValue("Usage", "DisableItemUsageWhileBleedingOut", Usage::_DisableItemUsageWhileBleedingOut);
		loginfo("Setting: {} {}", "usage:             DisableItemUsageWhileBleedingOut", std::to_string(Usage::_DisableItemUsageWhileBleedingOut));
		Usage::_DisableItemUsageWhileSleeping = ini.GetBoolValue("Usage", "DisableItemUsageWhileSleeping", Usage::_DisableItemUsageWhileSleeping);
		loginfo("Setting: {} {}", "usage:             DisableItemUsageWhileSleeping", std::to_string(Usage::_DisableItemUsageWhileSleeping));
		
		// compatibility
		
		// debug
		Debug::EnableLog = ini.GetBoolValue("Debug", "EnableLogging", Debug::EnableLog);
		Logging::EnableLog = Debug::EnableLog;
		loginfo("Setting: {} {}", "Debug:             EnableLogging", std::to_string(Debug::EnableLog));
		Debug::EnableProfiling = ini.GetBoolValue("Debug", "EnableProfiling", Debug::EnableProfiling);
		Logging::EnableProfile = Debug::EnableProfiling;
		loginfo("Setting: {} {}", "Debug:             EnableProfiling", std::to_string(Debug::EnableProfiling));

	}

	// save user settings, before applying adjustments
	Save();
}


void Settings::Save()
{
	// reset change flag
	Settings::_modifiedSettings = Settings::ChangeFlag::kNone;

	constexpr auto path = L"Data/SKSE/Plugins/NPCsUsePotions.ini";

	CSimpleIniA ini;

	ini.SetUnicode();

	// player

	// usage
	ini.SetBoolValue("Usage", "DisableItemUsageWhileStaggered", Usage::_DisableItemUsageWhileStaggered, "// NPCs that are staggered, unconcious, ragdolling or in a kill-move aren't able to use any potions and poisons.");
	ini.SetBoolValue("Usage", "DisableItemUsageWhileFlying", Usage::_DisableItemUsageWhileFlying, "// NPCs that are in mid-air or flying aren't able to use any potions and poisons.");
	ini.SetBoolValue("Usage", "DisableItemUsageWhileBleedingOut", Usage::_DisableItemUsageWhileBleedingOut, "// NPCs that are bleeding-out aren't able to use any potions and poisons.");
	ini.SetBoolValue("Usage", "DisableItemUsageWhileSleeping", Usage::_DisableItemUsageWhileSleeping, "// NPCs that are sleeping aren't able to use any potions and poisons.");
	
	// compatibility
	
	// debug
	ini.SetBoolValue("Debug", "EnableLogging", Debug::EnableLog, "// Enables logging output. Use with care as logs may get very large.");
	ini.SetBoolValue("Debug", "EnableProfiling", Debug::EnableProfiling, "// Enables profiling output.");

	ini.SaveFile(path);
}
