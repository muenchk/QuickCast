#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "ActorInfo.h"
#include "Data.h"
#include <tuple>
#include <utility>

using ActorInfoPtr = std::weak_ptr<ActorInfo>;


/// <summary>
/// Provides generic functions
/// </summary>
class Utility : public UtilityBase
{
public:

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
			plugin = Data::pluginnames[(form->GetFormID() >> 24)];
		} else
			plugin = Data::pluginnames[256 + (((form->GetFormID() & 0x00FFF000)) >> 12)];

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
			plugin = Data::pluginnames[(form->GetFormID() >> 24)];
		} else
			plugin = Data::pluginnames[256 + (((form->GetFormID() & 0x00FFF000)) >> 12)];

		return std::string("[") + typeid(T).name() + "<" + Utility::GetHex(form->GetFormID()) + "><" + form->GetName() + "><" + plugin + ">]";
	}

	/// <summary>
	/// Checks whether the given MagicItem has at least one effect with ArcheType [archetype] and the actorvalue [AV]
	/// </summary>
	/// <param name="item">the item to check</param>
	/// <param name="archetype">the archetype to check for</param>
	/// <param name="checkAV">whether to check if the effects also have [AV] as one of their actor values</param>
	/// <param name="AV">the actorvalue to check for</param>
	/// <returns></returns>
	static bool HasArchetype(RE::MagicItem* item, RE::EffectArchetype archetype, bool checkAV, RE::ActorValue AV);
};
