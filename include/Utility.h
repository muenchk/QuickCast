#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "ActorInfo.h"
#include "Settings.h"
#include <tuple>
#include <utility>

using ActorInfoPtr = std::weak_ptr<ActorInfo>;

namespace Misc
{
	/// <summary>
	/// Stores information about a template npc
	/// </summary>
	class NPCTPLTInfo
	{
	public:
		/// <summary>
		/// factions associated with the template npc
		/// </summary>
		std::vector<RE::TESFaction*> tpltfactions;
		/// <summary>
		/// keywords associated with the template npc
		/// </summary>
		std::vector<RE::BGSKeyword*> tpltkeywords;
		/// <summary>
		/// race of the template npc
		/// </summary>
		RE::TESRace* tpltrace = nullptr;
		/// <summary>
		/// combat style of the template npc
		/// </summary>
		RE::TESCombatStyle* tpltstyle = nullptr;
		/// <summary>
		/// class of the template npc
		/// </summary>
		RE::TESClass* tpltclass = nullptr;
		/// <summary>
		/// index of the most top-level non-temporary template
		/// </summary>
		uint32_t pluginID = 0x1;
		/// <summary>
		/// the first ActorBase from the top, with a non-runtime formid
		/// </summary>
		RE::TESNPC* base = nullptr;
		/// <summary>
		/// the first LeveledChar with non-runtime formid
		/// </summary>
		RE::TESLevCharacter* baselvl = nullptr;
	};
}


/// <summary>
/// Provides generic functions
/// </summary>
class Utility
{
public:

	#pragma region Comparison
	struct EqualsWeakPtrActorInfo
	{
		bool operator()(const ActorInfoPtr& lhs, const ActorInfoPtr& rhs)
		{
			return !lhs.owner_before(rhs) && !rhs.owner_before(lhs);
		}
	};

	template<typename T, typename U> 
	bool EqualsSharedPtr(const std::weak_ptr<T>& t, const std::weak_ptr<U>& u)
	{
		return !t.owner_before(u) && !u.owner_before(t);
	}

	/* template <typename T, typename U>
	inline bool EqualsSharedPtr(const std::weak_ptr<T>& t, const std::shared_ptr<U>& u)
	{
		return !t.owner_before(u) && !u.owner_before(t);
	}*/
	#pragma endregion

	/// <summary>
	/// Returns a string showing [val] as Hexadecimal number
	/// </summary>
	/// <param name="val"></param>
	/// <returns></returns>
	static std::string GetHex(long val)
	{
		std::stringstream ss;
		ss << std::hex << val;
		return ss.str();
	}
	/// <summary>
	/// Returns a string showing [val] as Hexadecimal number
	/// </summary>
	/// <param name="val"></param>
	/// <returns></returns>
	static std::string GetHex(uint64_t val)
	{
		std::stringstream ss;
		ss << std::hex << val;
		return ss.str();
	}
	/// <summary>
	/// Returns a string showing [val] as Hexadecimal number
	/// </summary>
	/// <param name="val"></param>
	/// <returns></returns>
	static std::string GetHex(uint32_t val)
	{
		std::stringstream ss;
		ss << std::hex << val;
		return ss.str();
	}
	/// <summary>
	/// Returns a string showing [val] as Hexadecimal number
	/// </summary>
	/// <param name="val"></param>
	/// <returns></returns>
	static std::string GetHex(int val)
	{
		std::stringstream ss;
		ss << std::hex << val;
		return ss.str();
	}
	/// <summary>
	/// Returns a string showing [val] as Hexadecimal number with padding
	/// </summary>
	/// <param name="val"></param>
	/// <returns></returns>
	static std::string GetHexFill(uint32_t val)
	{
		std::stringstream ss;
		ss << std::setw(16) << std::hex << std::setfill('0') << val;
		return ss.str();
	}
	/// <summary>
	/// Returns a string showing [val] as Hexadecimal number with padding
	/// </summary>
	/// <param name="val"></param>
	/// <returns></returns>
	static std::string GetHexFill(uint64_t val)
	{
		std::stringstream ss;
		ss << std::setw(16) << std::hex << std::setfill('0') << val;
		return ss.str();
	}

	/// <summary>
	/// Returns a string representing the given form
	/// </summary>
	/// <param name="form"></param>
	/// <returns></returns>
	template <class T>
	static std::string PrintForm(T* form)
	{
		if (form == nullptr || form->GetFormID() == 0)
			return "None";
		std::string plugin = "";
		if ((form->GetFormID() & 0xFF000000) != 0xFE000000) {
			plugin = Settings::pluginnames[(form->GetFormID() >> 24)];
		} else
			plugin = Settings::pluginnames[256 + (((form->GetFormID() & 0x00FFF000)) >> 12)];

		return std::string("[") + typeid(T).name() + "<" + Utility::GetHex(form->GetFormID()) + "><" + form->GetName() + "><" + plugin + ">]";
	}
	static std::string PrintForm(ActorInfo* acinfo);
	static std::string PrintForm(std::shared_ptr<ActorInfo> const& acinfo);
	static std::string PrintForm(std::weak_ptr<ActorInfo> acweak);

	/// <summary>
	/// Returns a string representing the given form
	/// </summary>
	/// <param name="form"></param>
	/// <returns></returns>
	template <class T>
	static std::string PrintFormNonDebug(T* form)
	{
		if (form == nullptr || form->GetFormID() == 0)
			return "None";
		std::string plugin = "";
		if ((form->GetFormID() & 0xFF000000) != 0xFE000000) {
			plugin = Settings::pluginnames[(form->GetFormID() >> 24)];
		} else
			plugin = Settings::pluginnames[256 + (((form->GetFormID() & 0x00FFF000)) >> 12)];

		return std::string("[") + typeid(T).name() + "<" + Utility::GetHex(form->GetFormID()) + "><" + form->GetName() + "><" + plugin + ">]";
	}

	/// <summary>
	/// Converts all symbols in a string into lower case.
	/// </summary>
	/// <param name="s"></param>
	/// <returns></returns>
	static std::string ToLower(std::string s)
	{
		std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (unsigned char)std::tolower(c); }
		);
		return s;
	}

	/// <summary>
	/// Splits a string at a delimiter, optionally removes empty results, and optionally respects escaped sequences
	/// </summary>
	/// <param name="str">input to split</param>
	/// <param name="delimiter">delimiter at which to split</param>
	/// <param name="removeEmpty">whether to remove empty strings</param>
	/// <param name="escape">whether to respect escaped sequences</param>
	/// <param name="escapesymbol">symbol that marks escaped sequences, sequences need to be surrounded by this symbol</param>
	/// <returns></returns>
	static std::vector<std::string> SplitString(std::string str, char delimiter, bool removeEmpty, bool escape = false, char escapesymbol = '\"', bool allowdisableescape = false, char disblechar = '\\');

	/// <summary>
	/// Splits a string at a delimiter, optionally removes empty results, and optionally respects escaped sequences
	/// </summary>
	/// <param name="str">input to split</param>
	/// <param name="delimiter">delimiter at which to split</param>
	/// <param name="removeEmpty">whether to remove empty strings</param>
	/// <param name="escape">whether to respect escaped sequences</param>
	/// <param name="escapesymbol">symbol that marks escaped sequences, sequences need to be surrounded by this symbol</param>
	/// <returns></returns>
	static std::vector<std::string> SplitString(std::string str, std::string delimiter, bool removeEmpty, bool escape = false, char escapesymbol = '\"');

	/// <summary>
	/// Evaluates whether the char at position [position] is escaped
	/// </summary>
	/// <param name="str">input to search</param>
	/// <param name="escapesymbol">symbol that marks escaped sequences, sequences need to be surrounded by this symbol</param>
	/// <param name="position">the position to check</param>
	/// <returns></returns>
	static bool IsEscaped(std::string str, int32_t position, char escapesymbol = '\"')
	{
		bool escaped = false;
		int32_t count = 0;
		for (char c : str) {
			//logdebug("Char: {}\t Window: {}\t Wsize: {}\t Wmaxsize: {}\t tmp: {}", c, slide.GetWindow(), slide.size, slide.maxsize, tmp);
			if (escaped == true) {
				// escaped sequence.
				if (c == escapesymbol) {
					escaped = !escaped;
				}
			} else {
				if (c == escapesymbol) {
					// we are at the beginning of an escaped sequence
					escaped = !escaped;
				}
			}
			if (count == position)
				return escaped;
			count++;
		}
		return false;
	}

	/// <summary>
	/// Removes whitespace from an input in-place, optionally respects escaped sequences
	/// </summary>
	static std::string& RemoveWhiteSpaces(std::string& str, char escape, bool removetab = false, bool allowdisableescape = false, char disablechar = '\\');

	/// <summary>
	/// Removes Symbols from an input in-place, optionally respects escaped sequences
	/// </summary>
	static std::string& RemoveSymbols(std::string& str, char symbol, bool enableescape = false, char escape = '\"');

	/// <summary>
	/// Removes Symbols from an input in-place, optionally doesn't remove escaped characters
	/// </summary>
	static std::string& RemoveSymbols(std::string& str, char symbol, char disablechar);

	/// <summary>
	/// Counts the occurences of the given symbol, respecting escaped sequences
	/// </summary>
	static int32_t CountSymbols(std::string str, char symbol, char escaped1, char escaped2)
	{
		int32_t count = 0;
		bool escape = false;
		for (char& c : str) {
			// escaped strings are marked by, e.g., " or ' and everything between two of them should not be counted
			// we need to check for both since one string could be marked with ", while another one could use '
			// this means that something thats invalid in python "usdzfgf' is counted as valid.
			if (c == escaped1 || c == escaped2)
				escape = !escape;
			if (escape == false)
				if (c == symbol)
					count++;
		}
		return count;
	}

	static std::vector<std::pair<char, int32_t>> GetSymbols(std::string str);

	/// <summary>
	/// The current combat style of an actor
	/// </summary>
	enum class CurrentCombatStyle
	{
		Spellsword = 0x1, // combination spell and onehanded
		OneHandedShield = 0x2,
		TwoHanded = 0x4,
		MagicDestruction = 0x8,
		MagicConjuration = 0x10,
		MagicAlteration = 0x20,
		MagicIllusion = 0x40,
		MagicRestoration = 0x80,
		OneHanded = 0x100,
		Ranged = 0x200,
		DualWield = 0x400,
		Staffsword = 0x800,  // combination staff and onehanded
		HandToHand = 0x1000,
		Mage = 0x2000,
		DualStaff = 0x4000,
		Staff = 0x8000,
		MagicDamageFire = 0x100000,
		MagicDamageShock = 0x200000,
		MagicDamageFrost = 0x400000,
	};

	/// <summary>
	/// Converts a CurrentCombatStyle value into a string
	/// </summary>
	/// <param name="style">CurrentCombatStyle value to convert</param>
	/// <returns>String representing [style]</returns>
	static std::string ToStringCombatStyle(uint32_t style);

	/// <summary>
	/// Retrieves data about the current equiped items and spells of an actor
	/// </summary>
	/// <param name="actor">the actor to check</param>
	/// <returns>A linear combination of CurrentCombatStyle, representing the combat data of an actor</returns>
	static uint32_t GetCombatData(RE::Actor* actor);

	enum class CurrentArmor
	{
		None = 0,
		HeavyArmor = 1 << 0,
		LightArmor = 1 << 1,
		Clothing = 1 << 2,
	};

	/// <summary>
	/// Returns which armor types are worn by a given npc.
	/// </summary>
	/// <param name="actor">The NPC to check.</param>
	/// <returns>Linear combination of values of CurrentArmor</returns>
	static uint32_t GetArmorData(RE::Actor* actor);

	/// <summary>
	/// Returns the type of item, specific weapon type etc
	/// </summary>
	/// <param name="form"></param>
	/// <returns></returns>
	static int GetItemType(RE::TESForm* form);

	/// <summary>
	/// Checks whether the given MagicItem has at least one effect with ArcheType [archetype] and the actorvalue [AV]
	/// </summary>
	/// <param name="item">the item to check</param>
	/// <param name="archetype">the archetype to check for</param>
	/// <param name="checkAV">whether to check if the effects also have [AV] as one of their actor values</param>
	/// <param name="AV">the actorvalue to check for</param>
	/// <returns></returns>
	static bool HasArchetype(RE::MagicItem* item, RE::EffectArchetype archetype, bool checkAV, RE::ActorValue AV);

	#pragma region Parsing

	/// <summary>
	/// Parses a string into a vector of int (array of int)
	/// </summary>
	/// <param name="line">string to parse</param>
	/// <returns>vector of int (array of int)</returns>
	static std::vector<int> ParseIntArray(std::string line);

	/// <summary>
	/// returns a TESForm* from various inputs
	/// </summary>
	/// <param name="datahandler">datahandler to get data from</param>
	/// <param name="formid">id or partial id of item (may be 0, if editorid is set)</param>
	/// <param name="pluginname">name of the plugin the item is included (may be the empty string, if item is in the basegame, or editorid is given)</param>
	/// <param name="editorid">editorid of the item, defaults to empty string</param>
	/// <returns></returns>
	static RE::TESForm* GetTESForm(RE::TESDataHandler* datahandler, RE::FormID formid, std::string pluginname, std::string editorid = "");

	/// <summary>
	/// Verifies that acinfo is a valid object
	/// <param name="acinfo">ActorInfo to verify</param>
	/// </summary>
	static bool VerifyActorInfo(std::shared_ptr<ActorInfo> const& acinfo);

	/// <summary>
	/// Returns whether an actor is valid and safe to work with
	/// </summary>
	/// <param name="actor">Actor to validate</param>
	/// <returns></returns>
	static bool ValidateActor(RE::Actor* actor);

	/// <summary>
	/// Returns whether a form is valid and safe to work with
	/// </summary>
	/// <param name="form">Form to validate</param>
	/// <returns></returns>
	template<class T>
	static bool ValidateForm(T* form)
	{
		if (form == nullptr || form->GetFormID() == 0 || form->formFlags & RE::TESForm::RecordFlags::kDeleted)
			return false;
		return true;
	}

	/// <summary>
	/// Extracts template information from an Actor
	/// </summary>
	/// <param name="npc"></param>
	/// <returns></returns>
	static Misc::NPCTPLTInfo ExtractTemplateInfo(RE::Actor* actor);
	/// <summary>
	/// Extracts template information from a NPC
	/// </summary>
	/// <param name="npc"></param>
	/// <returns></returns>
	static Misc::NPCTPLTInfo ExtractTemplateInfo(RE::TESNPC* npc);
	/// <summary>
	/// Exctracts template information from a Leveled Character
	/// </summary>
	/// <param name="lvl"></param>
	/// <returns></returns>
	static Misc::NPCTPLTInfo ExtractTemplateInfo(RE::TESLevCharacter* lvl);
	#pragma endregion


	#pragma region Mods

	class Mods
	{
	public:
		/// <summary>
		/// Returns the pluginname the form is defined in
		/// </summary>
		static std::string GetPluginName(RE::TESForm* form);

		/// <summary>
		/// Returns the pluginname the form is defined in
		/// </summary>
		static std::string GetPluginNameFromID(RE::FormID formid);

		/// <summary>
		/// Returns the pluginname corresponding to the pluginIndex
		/// </summary>
		static std::string GetPluginName(uint32_t pluginIndex);

		/// <summary>
		/// Returns the plugin index of the given plugin [returns 0x1 on failure]
		/// </summary>
		/// <param name="pluginname"></param>
		/// <returns></returns>
		static uint32_t GetPluginIndex(std::string pluginname);

		/// <summary>
		/// Returns the plugin index of the given form [returns 0x1 on failure]
		/// </summary>
		/// <param name="pluginname"></param>
		/// <returns></returns>
		static uint32_t GetPluginIndex(RE::TESForm* form);

		/// <summary>
		/// Returns the formid with the plugin index
		/// </summary>
		static uint32_t GetIndexLessFormID(RE::TESForm* form);

		/// <summary>
		/// Returns the formid with the plugin index
		/// </summary>
		static uint32_t GetIndexLessFormID(RE::FormID formid);

		/// <summary>
		/// Returns a vector with all forms of the given type in the plugin
		/// </summary>
		/// <typeparam name="T">form type</typeparam>
		/// <param name="pluginname">name of the plugin</param>
		/// <returns></returns>
		template <class T>
		static std::vector<T*> GetFormsInPlugin(std::string pluginname)
		{
			auto datahandler = RE::TESDataHandler::GetSingleton();
			const RE::TESFile* file = nullptr;
			uint32_t pindex = Utility::Mods::GetPluginIndex(pluginname);
			std::vector<T*> ret;
			if (pindex == 0x1) // if mod cannot be found return
				return ret;
			if ((pindex & 0x00FFF000) != 0)  // light mod
				file = datahandler->LookupLoadedLightModByIndex((uint16_t)((pindex & 0x00FFF000) >> 12));
			else  // normal mod
				file = datahandler->LookupLoadedModByIndex((uint8_t)(pindex >> 24));
			if (file != nullptr) {
				uint32_t mask = 0;
				uint32_t index = 0;
				if (file->IsLight()) {
					mask = 0xFFFFF000;
					index = file->GetPartialIndex() << 12;
				} else {
					mask = 0xFF000000;
					index = file->GetPartialIndex() << 24;
				}
				auto forms = datahandler->GetFormArray<T>();
				for (int i = 0; i < (int)forms.size(); i++) {
					if ((forms[i]->GetFormID() & mask) == index)
						ret.push_back(forms[i]);
				}
			}
			return ret;
		}
	};

	#pragma endregion
};
