#include "Utility.h"
#include "Compatibility.h"


std::string Utility::PrintForm(ActorInfo* acinfo)
{
	if (acinfo == nullptr)
		return "None";
	return std::string("[") + typeid(ActorInfo).name() + "<" + Utility::GetHex(acinfo->GetFormID()) + "><" + acinfo->GetName() + "><" + acinfo->GetPluginname() + ">]";
}

std::string Utility::PrintForm(std::shared_ptr<ActorInfo> const& acinfo)
{
	if (acinfo == nullptr || acinfo->IsValid() == false)
		return "None";
	return std::string("[") + typeid(ActorInfo).name() + "<" + Utility::GetHex(acinfo->GetFormID()) + "><" + acinfo->GetName() + "><" + acinfo->GetPluginname() + ">]";
}

std::string Utility::PrintForm(std::weak_ptr<ActorInfo> acweak)
{
	if (std::shared_ptr<ActorInfo> acinfo = acweak.lock()) {
		if (acinfo == nullptr || acinfo->IsValid() == false)
			return "None";
		return std::string("[") + typeid(ActorInfo).name() + "<" + Utility::GetHex(acinfo->GetFormID()) + "><" + acinfo->GetName() + "><" + acinfo->GetPluginname() + ">]";
	} else {
		return "None";
	}
}

bool Utility::HasArchetype(RE::MagicItem* item, RE::EffectArchetype archetype, bool checkAV, RE::ActorValue AV)
{
	RE::EffectSetting* sett = nullptr;
	RE::EffectSetting* prim = nullptr;
	if ((item->avEffectSetting) == nullptr && item->effects.size() == 0) {
		loginfo("fail: no item effects");
		return false;
	}
	bool found = false;
	if (item->effects.size() > 0) {
		for (uint32_t i = 0; i < item->effects.size(); i++) {
			sett = item->effects[i]->baseEffect;
			if (sett) {
				if (prim == nullptr) {
					prim = sett;
				}
				uint32_t formid = sett->GetFormID();
				if (sett->GetArchetype() == archetype &&
					(checkAV == false || 
						sett->data.primaryAV == AV ||
						sett->data.secondaryAV == AV))
					return true;
			}
		}
	} 
	return false;
}
