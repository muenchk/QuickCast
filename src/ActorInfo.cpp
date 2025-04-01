#include <exception>

#include "ActorInfo.h"
#include "Settings.h"
#include "Utility.h"
#include "BufferOperations.h"
#include "Data.h"
#include "Compatibility.h"
#include "Logging.h"

void AnimationInfo::Interrupt()
{
	if (!interrupt && acinfo && acinfo->IsValid()) {
		interrupt = true;
		if (consumedMagicka > 0) {
			acinfo->RestoreActorValue(RE::ActorValue::kMagicka, consumedMagicka);
		}
	}
}


void ActorInfo::Init()
{
	playerRef = RE::PlayerCharacter::GetSingleton();
}

ActorInfo::ActorInfo(RE::Actor* _actor)
{
	loginfo("");
	if (_actor) {
		actor = _actor->GetHandle();
		formid.SetID(_actor->GetFormID());
		// get original id
		if (const auto extraLvlCreature = _actor->extraList.GetByType<RE::ExtraLeveledCreature>()) {
			if (const auto originalBase = extraLvlCreature->originalBase) {
				formid.SetOriginalID(originalBase->GetFormID());
			}
			if (const auto templateBase = extraLvlCreature->templateBase) {
				formid.AddTemplateID(templateBase->GetFormID());
			}
		} else {
			formid.SetOriginalID(_actor->GetActorBase()->GetFormID());
		}
		name = std::string(_actor->GetName());
		pluginname = Utility::Mods::GetPluginName(_actor);
		pluginID = Utility::Mods::GetPluginIndex(pluginname);
		// if there is no plugin ID, it means that npc is temporary, so base it off of the base npc
		if (pluginID == 0x1) {
			pluginID = Utility::ExtractTemplateInfo(_actor->GetActorBase()).pluginID;
		}
		if (_actor->HasKeyword(Settings::ActorTypeDwarven) || _actor->GetRace()->HasKeyword(Settings::ActorTypeDwarven))
			_automaton = true;
		if (_actor->HasKeyword(Settings::Vampire) || _actor->GetRace()->HasKeyword(Settings::Vampire))
			_vampire = true;
		for (auto slot : _actor->GetRace()->equipSlots) {
			if (slot->GetFormID() == 0x13F43) // LeftHand
				_haslefthand = true;
		}
		// Run since [actor] is valid
		UpdateMetrics(actor);
		// set to valid
		valid = true;
		_formstring = Utility::PrintForm(this);
		timestamp_invalid = 0;
		dead = false;
	}
}

void ActorInfo::Reset(RE::Actor* _actor)
{
	loginfo("");
	if (blockReset) {
		loginfo("Reset has been blocked.");
		return;
	}
	aclock;
	globalCooldownTimer = 0;
	formid = ID();
	pluginname = "";
	pluginID = 1;
	name = "";
	_automaton = false;
	Animation_busy_left = false;
	Animation_busy_right = false;
	whitelisted = false;
	whitelistedcalculated = false;
	combatstate = CombatState::OutOfCombat;
	combatdata = 0;
	tcombatdata = 0;
	target = std::weak_ptr<ActorInfo>{};
	handleactor = false;
	if (_actor) {
		actor = _actor->GetHandle();
		formid.SetID(_actor->GetFormID());
		// get original id
		if (const auto extraLvlCreature = _actor->extraList.GetByType<RE::ExtraLeveledCreature>()) {
			if (const auto originalBase = extraLvlCreature->originalBase) {
				formid.SetOriginalID(originalBase->GetFormID());
			}
			if (const auto templateBase = extraLvlCreature->templateBase) {
				formid.AddTemplateID(templateBase->GetFormID());
			}
		} else {
			formid.SetOriginalID(_actor->GetActorBase()->GetFormID());
		}
		name = std::string(_actor->GetName());
		pluginname = Utility::Mods::GetPluginName(_actor);
		pluginID = Utility::Mods::GetPluginIndex(pluginname);
		// if there is no plugin ID, it means that npc is temporary, so base it off of the base npc
		if (pluginID == 0x1) {
			pluginID = Utility::ExtractTemplateInfo(_actor->GetActorBase()).pluginID;
		}
		if (_actor->HasKeyword(Settings::ActorTypeDwarven) || _actor->GetRace()->HasKeyword(Settings::ActorTypeDwarven))
			_automaton = true;
		for (auto slot : _actor->GetRace()->equipSlots) {
			if (slot->GetFormID() == 0x13F43)  // LeftHand
				_haslefthand = true;
		}
		// Run since [actor] is valid
		UpdateMetrics(actor);
		// update poison resistance
		UpdatePermanentPoisonResist();
		// set to valid
		valid = true;
		_formstring = Utility::PrintForm(this);
		timestamp_invalid = 0;
		dead = false;
	}
}

ActorInfo::ActorInfo(bool blockedReset)
{
	blockReset = blockedReset;
}

ActorInfo::ActorInfo()
{

}

bool ActorInfo::IsValid()
{
	aclock;
	return valid && actor.get() && actor.get().get();
}

void ActorInfo::SetValid()
{
	aclock;
	valid = true;
	timestamp_invalid = 0;
}

#define CurrentMilliseconds std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()

void ActorInfo::SetInvalid()
{
	aclock;
	valid = false;
	timestamp_invalid = CurrentMilliseconds;
}

bool ActorInfo::IsExpired()
{
	aclock;
	// if object was invalidated more than 10 seconds ago, it is most likely not needed anymore
	if (valid == false && (CurrentMilliseconds - timestamp_invalid) > 10000)
	{
		return true;
	}
	return false;
}

void ActorInfo::SetDead()
{
	aclock;
	dead = true;
}

bool ActorInfo::GetDead()
{
	aclock;
	return dead;
}

RE::Actor* ActorInfo::GetActor()
{
	aclock;
	if (!valid || dead)
		return nullptr;

	if (actor.get() && actor.get().get())
		return actor.get().get();
	return nullptr;
}

RE::ActorHandle ActorInfo::GetHandle()
{
	aclock;
	if (!valid || dead)
		return RE::ActorHandle();

	if (actor.get().get())
		return actor;
	return RE::ActorHandle();
}

RE::FormID ActorInfo::GetFormID()
{
	aclock;
	if (!valid)
		return 0;
	return formid;
}

RE::FormID ActorInfo::GetFormIDBlank()
{
	return formid;
}

RE::FormID ActorInfo::GetFormIDOriginal()
{
	aclock;
	if (!valid)
		return 0;
	return formid.GetOriginalID();
}

std::vector<RE::FormID> ActorInfo::GetTemplateIDs()
{
	aclock;
	if (!valid)
		return {};
	return formid.GetTemplateIDs();
}

std::string ActorInfo::GetPluginname()
{
	aclock;
	if (!valid)
		return "";
	return pluginname;
}

uint32_t ActorInfo::GetPluginID()
{
	aclock;
	if (!valid)
		return 0x1;
	return pluginID;
}

std::string ActorInfo::GetName()
{
	aclock;
	if (!valid)
		return "";
	return name;
}

std::string ActorInfo::ToString()
{
	aclock;
	if (!valid || !actor.get() || !actor.get().get())
		return "Invalid Actor Info";
	return "actor addr: " + Utility::GetHex(reinterpret_cast<std::uintptr_t>(actor.get().get())) + "\tactor:" + Utility::PrintForm(actor.get().get());
}

void ActorInfo::UpdateMetrics(RE::ActorHandle handle)
{
	if (RE::Actor* reac = actor.get().get(); reac != nullptr) {
		playerDistance = reac->GetPosition().GetSquaredDistance(playerPosition);
		playerHostile = reac->IsHostileToActor(playerRef);
	}
}

bool ActorInfo::IsInCombat()
{
	aclock;
	if (!valid || dead)
		return false;
	else if (combatstate == CombatState::InCombat || combatstate == CombatState::Searching)
		return true;
	else // combatstate == CombatState::OutOfCombat
		return false;
}

CombatState ActorInfo::GetCombatState()
{
	aclock;
	if (!valid || dead)
		return CombatState::OutOfCombat;
	return combatstate;
}

void ActorInfo::SetCombatState(CombatState state)
{
	aclock;
	if (!valid || dead)
		combatstate = CombatState::OutOfCombat;
	combatstate = state;
}

bool ActorInfo::IsWeaponDrawn()
{
	aclock;
	if (!valid || dead)
		return false;
	return weaponsDrawn;
}

// data functions

uint32_t ActorInfo::GetVersion()
{
	return version;
}

int32_t ActorInfo::GetDataSize()
{
	int32_t size = 0;
	// versionid
	//size += 4;
	// actor id
	//size += 4;
	// pluginname
	size += Buffer::CalcStringLength(pluginname);
	//size += 1;
	// Animation_busy_left
	//size += 1;
	// Animation_busy_rigt
	//size += 1;
	// globalCooldownTimer;
	//size += 4;
	// valid
	//size += 1;
	// combatstate
	//size += 4;
	// haslefthand
	//size = 1;

	// all except string are constant:
	size += 21;
	return size;
}

int32_t ActorInfo::GetMinDataSize(int32_t vers)
{
	switch (vers) {
	case 0x10:
		return 21;
	default:
		return 0;
	}
}

bool ActorInfo::WriteData(unsigned char* buffer, size_t offset)
{
	aclock;
	size_t addoff = 0;
	// version
	Buffer::Write(version, buffer, offset);
	// valid
	Buffer::Write(valid, buffer, offset);
	// actor id
	if ((formid & 0xFF000000) == 0xFF000000) {
		// temporary id, save whole id
		Buffer::Write(formid, buffer, offset);
	} else if ((formid & 0xFF000000) == 0xFE000000) {
		// only save index in light plugin
		Buffer::Write(formid & 0x00000FFF, buffer, offset);
	} else {
		// save index in normal plugin
		Buffer::Write(formid & 0x00FFFFFF, buffer, offset);
	}
	// pluginname
	Buffer::Write(pluginname, buffer, offset);
	// Animation_busy_left
	Buffer::Write(Animation_busy_left, buffer, offset);
	// Animation_busy_right
	Buffer::Write(Animation_busy_right, buffer, offset);
	// globalCooldownTimer
	Buffer::Write(globalCooldownTimer, buffer, offset);
	// combatstate
	Buffer::Write(static_cast<uint32_t>(combatstate), buffer, offset);
	// haslefthand
	Buffer::Write(_haslefthand, buffer, offset);
	return true;
}

bool ActorInfo::ReadData(unsigned char* buffer, size_t offset, size_t length)
{
	aclock;
	int ver = Buffer::ReadUInt32(buffer, offset);
	try {
		switch (ver) {
		case 0x00000010:
			{
				valid = Buffer::ReadBool(buffer, offset);

				// first try to make sure that the buffer contains all necessary data and we do not go out of bounds
				int size = GetMinDataSize(ver);
				int strsize = (int)Buffer::CalcStringLength(buffer, offset + 4);  // offset + actorid is begin of pluginname
				if (length < size + strsize)
					return false;

				formid.SetID(Buffer::ReadUInt32(buffer, offset));
				pluginname = Buffer::ReadString(buffer, offset);
				// if the actorinfo is not valid, then do not evaluate the actor
				RE::TESForm* form = Utility::GetTESForm(RE::TESDataHandler::GetSingleton(), formid, pluginname);
				if (form == nullptr) {
					form = RE::TESForm::LookupByID(formid);
					if (!form) {
						return false;
					}
				}
				RE::Actor* reac = form->As<RE::Actor>();
				if (reac == nullptr) {
					return false;
				}
				actor = reac->GetHandle();
				// set formid to the full formid including plugin index
				formid.SetID(reac->GetFormID());

				name = reac->GetName();
				Animation_busy_left = Buffer::ReadBool(buffer, offset);
				Animation_busy_right = Buffer::ReadBool(buffer, offset);
				globalCooldownTimer = Buffer::ReadInt32(buffer, offset);
				combatstate = static_cast<CombatState>(Buffer::ReadUInt32(buffer, offset));
				_haslefthand = Buffer::ReadBool(buffer, offset);

				// init dependend stuff
				pluginID = Utility::Mods::GetPluginIndex(pluginname);
				if (pluginID == 0x1) {
					pluginID = Utility::ExtractTemplateInfo(reac->GetActorBase()).pluginID;
				}
				_formstring = Utility::PrintForm(this);
				// get original id
				if (const auto extraLvlCreature = reac->extraList.GetByType<RE::ExtraLeveledCreature>()) {
					if (const auto originalBase = extraLvlCreature->originalBase) {
						formid.SetOriginalID(originalBase->GetFormID());
					}
					if (const auto templateBase = extraLvlCreature->templateBase) {
						formid.AddTemplateID(templateBase->GetFormID());
					}
				} else {
					formid.SetOriginalID(reac->GetActorBase()->GetFormID());
				}
				UpdateMetrics(actor);
				timestamp_invalid = 0;
				// update poison resitance
				UpdatePermanentPoisonResist();
			}
			return true;
		default:
			return false;
		}
	} catch (std::exception&) {
		return false;
	}
}

void ActorInfo::Update()
{
	aclock;
	if (!valid)
		return;
	if (RE::Actor* reac = actor.get().get(); reac != nullptr) {
		// update vampire status
		_vampire = false;
		if (reac->HasKeyword(Settings::Vampire) || reac->GetRace()->HasKeyword(Settings::Vampire))
			_vampire = true;
		// update the metrics, since we are sure our object is valid
		UpdateMetrics(actor);
	}
	else
	{
		SetInvalid();
	}
}

std::weak_ptr<ActorInfo> ActorInfo::GetTarget()
{
	aclock;
	if (!valid)
		return std::weak_ptr<ActorInfo>{};
	return target;
}

void ActorInfo::ResetTarget()
{
	aclock;
	target = std::weak_ptr<ActorInfo>{};
}

void ActorInfo::SetTarget(std::weak_ptr<ActorInfo> tar)
{
	aclock;
	if (!valid || dead)
		target = std::weak_ptr<ActorInfo>{};
	else
		target = tar;
}

short ActorInfo::GetTargetLevel()
{
	aclock;
	if (!valid || dead)
		return 1;
	if (std::shared_ptr<ActorInfo> tar = target.lock()) {
		return tar->GetLevel();
	}
	return 1;
}

uint32_t ActorInfo::GetCombatData()
{
	aclock;
	if (!valid || dead)
		return 0;
	return combatdata;
}

void ActorInfo::SetCombatData(uint32_t data)
{
	aclock;
	if (!valid || dead)
		combatdata = 0;
	else
		combatdata = data;
}

uint32_t ActorInfo::GetCombatDataTarget()
{
	aclock;
	if (!valid || dead)
		return 0;
	return tcombatdata;
}

void ActorInfo::SetCombatDataTarget(uint32_t data)
{
	aclock;
	if (!valid || dead)
		tcombatdata = 0;
	else
		tcombatdata = data;
}

bool ActorInfo::GetHandleActor()
{
	aclock;
	if (!valid || dead)
		return false;
	return handleactor;
}

void ActorInfo::SetHandleActor(bool handle)
{
	aclock;
	if (!valid || dead) {
		handleactor = false;
	} else {
		handleactor = handle;
	}
}

float ActorInfo::GetPlayerDistance()
{
	aclock;
	if (!valid)
		return FLT_MAX;
	return playerDistance;
}

void ActorInfo::SetPlayerDistance(float distance)
{
	aclock;
	if (!valid || dead)
		playerDistance = FLT_MAX;
	else
		playerDistance = distance;
}

bool ActorInfo::GetPlayerHostile()
{
	aclock;
	if (!valid || dead)
		return false;
	return playerHostile;
}

void ActorInfo::SetPlayerHostile(bool hostile)
{
	aclock;
	if (!valid || dead)
		playerHostile = false;
	playerHostile = hostile;
}

bool ActorInfo::GetWeaponsDrawn()
{
	aclock;
	if (!valid || dead)
		return false;
	return weaponsDrawn;
}

void ActorInfo::SetWeaponsDrawn(bool drawn)
{
	aclock;
	if (!valid || dead)
		weaponsDrawn = false;
	else
		weaponsDrawn = drawn;
}

void ActorInfo::UpdateWeaponsDrawn()
{
	aclock;
	if (!valid || dead)
		weaponsDrawn = false;

	if (actor.get() && actor.get().get())
		weaponsDrawn = actor.get().get()->AsActorState()->IsWeaponDrawn();
}

bool ActorInfo::GetGraphVariableBool(const RE::BSFixedString& a_variableName, bool& a_out)
{
	aclock;
	if (!valid || dead)
		return false;

	if (actor.get().get())
		return actor.get().get()->GetGraphVariableBool(a_variableName, a_out);
	return false;
}

bool ActorInfo::SetGraphVariableBool(const RE::BSFixedString& a_variable, bool a_in)
{
	aclock;
	if (!valid || dead)
		return false;

	if (actor.get().get())
		return actor.get().get()->SetGraphVariableBool(a_variable, a_in);
	return false;
}

bool ActorInfo::GetGraphVariableInt(const RE::BSFixedString& a_variableName, int32_t& a_out)
{
	aclock;
	if (!valid || dead)
		return false;

	if (actor.get().get())
		return actor.get().get()->GetGraphVariableInt(a_variableName, a_out);
	return false;
}

bool ActorInfo::SetGraphVariableInt(const RE::BSFixedString& a_variable, int32_t a_in)
{
	aclock;
	if (!valid || dead)
		return false;

	if (actor.get().get())
		return actor.get().get()->SetGraphVariableInt(a_variable, a_in);
	return false;
}

bool ActorInfo::GetGraphVariableFloat(const RE::BSFixedString& a_variableName, float& a_out)
{
	aclock;
	if (!valid || dead)
		return false;

	if (actor.get().get())
		return actor.get().get()->GetGraphVariableFloat(a_variableName, a_out);
	return false;
}

bool ActorInfo::SetGraphVariableFloat(const RE::BSFixedString& a_variable, float a_in)
{
	aclock;
	if (!valid || dead)
		return false;

	if (actor.get().get())
		return actor.get().get()->SetGraphVariableFloat(a_variable, a_in);
	return false;
}

bool ActorInfo::NotifyAnimationGraph(const RE::BSFixedString& a_eventName)
{
	aclock;
	if (!valid || dead)
		return false;

	if (actor.get().get())
		return actor.get().get()->NotifyAnimationGraph(a_eventName);
	return false;
}

void ActorInfo::CastSpellImmediate(RE::SpellItem* spell, RE::Actor* target, RE::MagicSystem::CastingSource castsource, bool consumeMagicka)
{
	aclock;
	if (!valid || dead)
		return;
	if (actor.get() && actor.get().get()) {
		actor.get().get()->GetMagicCaster(castsource)->CastSpellImmediate(spell, false, target, 1, false, false, actor.get().get());
	}
}

void ActorInfo::UpdatePermanentPoisonResist()
{
	aclock;
	if (!valid || dead)
		return;
	if (RE::Actor* ac = actor.get().get(); ac != nullptr) {
		auto race = ac->GetRace();
		if (race && race->actorEffects && race->actorEffects->numSpells > 0) {
			// find all abilities that add poison resistance
			for (int i = 0; i < race->actorEffects->numSpells; i++) {
				if (race->actorEffects->spells[i]) {
					RE::EffectSetting* sett = nullptr;
					for (int c = 0; c < race->actorEffects->spells[i]->effects.size(); c++) {
						sett = race->actorEffects->spells[i]->effects[c]->baseEffect;
						if (sett) {
							if ((ConvertToAlchemyEffectPrimary(sett) & AlchemicEffect::kPoisonResist).IsValid() || (ConvertToAlchemyEffectSecondary(sett) & AlchemicEffect::kPoisonResist).IsValid()) {
								// found effect wth poison resist
								if (sett->IsDetrimental())
									_permanentPoisonResist -= race->actorEffects->spells[i]->effects[c]->effectItem.magnitude;
								else
									_permanentPoisonResist += race->actorEffects->spells[i]->effects[c]->effectItem.magnitude;
							}
						}
					}
				}
			}
		}
	}
}

#pragma region ActorSpecificFunctions

bool ActorInfo::IsFollower()
{
	aclock;
	if (!valid)
		return false;

	if (actor.get() && actor.get().get()) {
		RE::Actor* reac = actor.get().get();
		bool follower = reac->IsInFaction(Settings::CurrentFollowerFaction) || reac->IsInFaction(Settings::CurrentHirelingFaction);
		if (follower)
			return true;
		/* if (reac->GetActorBase()) {
			auto itr = reac->GetActorBase()->factions.begin();
			while (itr != reac->GetActorBase()->factions.end()) {
				if (Distribution::followerFactions()->contains(itr->faction->GetFormID()) && itr->rank >= 0)
					return true;
				itr++;
			}
		}*/
	}
	return false;
}

bool ActorInfo::IsPlayer()
{
	return formid == 0x14;
}

bool ActorInfo::Is3DLoaded()
{
	aclock;
	if (!valid)
		return false;
	if (RE::Actor* ac = actor.get().get(); ac != nullptr)
	{
		return ac->Is3DLoaded();
	}
	return false;
}

RE::TESObjectREFR::InventoryItemMap ActorInfo::GetInventory()
{
	aclock;
	if (!valid)
		return RE::TESObjectREFR::InventoryItemMap{};

	if (actor.get() && actor.get().get())
		return actor.get().get()->GetInventory();
	return RE::TESObjectREFR::InventoryItemMap{};
}

RE::TESObjectREFR::InventoryCountMap ActorInfo::GetInventoryCounts()
{
	aclock;
	if (!valid)
		return RE::TESObjectREFR::InventoryCountMap{};

	if (actor.get() && actor.get().get())
		return actor.get().get()->GetInventoryCounts();
	return RE::TESObjectREFR::InventoryCountMap{};
}

bool ActorInfo::HasMagicEffect(RE::EffectSetting* effect)
{
	aclock;
	if (!valid || dead)
		return false;

	if (actor.get() && actor.get().get())
		return actor.get().get()->AsMagicTarget()->HasMagicEffect(effect);
	return false;
}

bool ActorInfo::DrinkPotion(RE::AlchemyItem* potion, RE::ExtraDataList* extralist)
{
	aclock;
	if (!valid || dead)
		return false;

	if (actor.get() && actor.get().get())
		return actor.get().get()->DrinkPotion(potion, extralist);
	return false;
}

RE::InventoryEntryData* ActorInfo::GetEquippedEntryData(bool leftHand)
{
	aclock;
	if (!valid || dead)
		return nullptr;

	if (!leftHand || _haslefthand)
		if (actor.get() && actor.get().get())
			return actor.get().get()->GetEquippedEntryData(leftHand);
	return nullptr;
}

RE::TESForm* ActorInfo::GetEquippedObject(bool leftHand)
{
	aclock;
	if (!valid || dead)
		return nullptr;

	if (!leftHand || _haslefthand)
		if (actor.get() && actor.get().get())
			return actor.get().get()->GetEquippedObject(leftHand);
	return nullptr;
}

bool ActorInfo::EquipItem(bool lefthand, RE::TESBoundObject* object)
{
	aclock;
	if (!valid || dead)
		return false;
	if (lefthand) {
		if (_haslefthand) {
			if (actor.get() && actor.get().get()) {
				RE::ActorEquipManager::GetSingleton()->EquipObject(actor.get().get(), object, nullptr, 1, Settings::Equip_LeftHand, true, true, true, true);
				return true;
			}
			else
				return false;
		} else
			return false;
	} else {
		if (actor.get() && actor.get().get()) {
			RE::ActorEquipManager::GetSingleton()->EquipObject(actor.get().get(), object, nullptr, 1, Settings::Equip_RightHand, true, true, true, true);
			return true;
		} else
			return false;
	}
}

bool ActorInfo::EquipSpell(RE::SpellItem* spell, RE::BGSEquipSlot* slot)
{
	aclock;
	if (!valid || dead)
		return false;
	if (actor.get() && actor.get().get()) {
		loginfo("equip");
		RE::ActorEquipManager::GetSingleton()->EquipSpell(actor.get().get(), spell, slot);
		return true;
	} else
		return false;
}

bool ActorInfo::EquipShout(RE::TESShout* shout)
{
	aclock;
	if (!valid || dead)
		return false;
	if (actor.get() && actor.get().get()) {
		RE::ActorEquipManager::GetSingleton()->EquipShout(actor.get().get(), shout);
		return true;
	} else
		return false;
}

void ActorInfo::RemoveItem(RE::TESBoundObject* item, int32_t count)
{
	aclock;
	if (!valid)
		return;

	if (actor.get() && actor.get().get())
		actor.get().get()->RemoveItem(item, count, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
}

void ActorInfo::AddItem(RE::TESBoundObject* item, int32_t count)
{
	aclock;
	if (!valid)
		return;

	if (actor.get() && actor.get().get())
		actor.get().get()->AddObjectToContainer(item, nullptr, count, nullptr);
}

uint32_t ActorInfo::GetFormFlags()
{
	aclock;
	if (!valid || dead)
		return 0;

	if (actor.get() && actor.get().get())
		return actor.get().get()->formFlags;
	return 0;
}

bool ActorInfo::IsDead()
{
	aclock;
	if (!valid || dead)
		return true;

	if (actor.get() && actor.get().get())
		return actor.get().get()->IsDead();
	return true;
}

RE::TESNPC* ActorInfo::GetActorBase()
{
	aclock;
	if (!valid)
		return nullptr;

	if (actor.get() && actor.get().get())
		return actor.get().get()->GetActorBase();
	return nullptr;
}

RE::FormID ActorInfo::GetActorBaseFormID()
{
	aclock;
	if (!valid)
		return 0;

	if (actor.get() && actor.get().get())
		if (actor.get().get()->GetActorBase())
			return actor.get().get()->GetActorBase()->GetFormID();
	return 0;
}

std::string ActorInfo::GetActorBaseFormEditorID()
{
	aclock;
	if (!valid)
		return "";

	if (actor.get() && actor.get().get())
		if (actor.get().get()->GetActorBase())
			return actor.get().get()->GetActorBase()->GetFormEditorID();
	return "";
}

RE::TESCombatStyle* ActorInfo::GetCombatStyle()
{
	aclock;
	if (!valid)
		return nullptr;

	if (actor.get() && actor.get().get())
		if (actor.get().get()->GetActorBase())
			return actor.get().get()->GetActorBase()->GetCombatStyle();
	return nullptr;
}

RE::TESRace* ActorInfo::GetRace()
{
	aclock;
	if (!valid)
		return nullptr;

	if (actor.get() && actor.get().get())
		if (actor.get().get()->GetActorBase())
			return actor.get().get()->GetActorBase()->GetRace();
	return nullptr;
}

RE::FormID ActorInfo::GetRaceFormID()
{
	aclock;
	if (!valid)
		return 0;

	if (actor.get() && actor.get().get())
		if (actor.get().get()->GetActorBase() && actor.get().get()->GetActorBase()->GetRace())
			return actor.get().get()->GetActorBase()->GetRace()->GetFormID();
	return 0;
}

bool ActorInfo::IsGhost()
{
	aclock;
	if (!valid)
		return false;

	if (actor.get() && actor.get().get())
		return actor.get().get()->IsGhost();
	return false;
}

bool ActorInfo::IsSummonable()
{
	aclock;
	if (!valid)
		return false;

	if (actor.get() && actor.get().get())
		if (actor.get().get()->GetActorBase())
			return actor.get().get()->GetActorBase()->IsSummonable();
	return false;
}

bool ActorInfo::Bleeds()
{
	aclock;
	if (!valid)
		return false;

	if (actor.get() && actor.get().get())
		if (actor.get().get()->GetActorBase())
			return actor.get().get()->GetActorBase()->Bleeds();
	return false;
}

short ActorInfo::GetLevel()
{
	aclock;
	if (!valid)
		return 1;
	if (actor.get() && actor.get().get())
		return actor.get().get()->GetLevel();
	return 1;
}

SKSE::stl::enumeration<RE::Actor::BOOL_BITS, uint32_t> ActorInfo::GetBoolBits()
{
	aclock;
	if (!valid)
		return SKSE::stl::enumeration<RE::Actor::BOOL_BITS, uint32_t>{};

	if (actor.get() && actor.get().get())
		return actor.get().get()->GetActorRuntimeData().boolBits;
	return SKSE::stl::enumeration<RE::Actor::BOOL_BITS, uint32_t>{};
}

bool ActorInfo::IsFlying()
{
	aclock;
	if (!valid || dead)
		return false;

	if (actor.get() && actor.get().get())
		return actor.get().get()->AsActorState()->IsFlying();
	return false;
}

bool ActorInfo::IsInKillMove()
{
	aclock;
	if (!valid || dead)
		return false;

	if (actor.get() && actor.get().get())
		return actor.get().get()->IsInKillMove();
	return false;
}

bool ActorInfo::IsInMidair()
{
	aclock;
	if (!valid || dead)
		return false;

	if (actor.get() && actor.get().get())
		return actor.get().get()->IsInMidair();
	return false;
}

bool ActorInfo::IsInRagdollState()
{
	aclock;
	if (!valid || dead)
		return false;

	if (actor.get() && actor.get().get())
		return actor.get().get()->IsInRagdollState();
	return false;
}

bool ActorInfo::IsUnconscious()
{
	aclock;
	if (!valid || dead)
		return false;

	if (actor.get() && actor.get().get())
		return actor.get().get()->AsActorState()->IsUnconscious();
	return false;
}

bool ActorInfo::IsParalyzed()
{
	aclock;
	if (!valid || dead)
		return false;

	if (actor.get() && actor.get().get())
		if (actor.get().get()->GetActorRuntimeData().boolBits & RE::Actor::BOOL_BITS::kParalyzed)
			return true;
	return false;
}

bool ActorInfo::IsStaggered()
{
	aclock;
	if (!valid || dead)
		return false;

	if (actor.get() && actor.get().get())
		return actor.get().get()->AsActorState()->actorState2.staggered;
	return false;
}

bool ActorInfo::IsBleedingOut()
{
	aclock;
	if (!valid || dead)
		return false;

	if (actor.get() && actor.get().get())
		return actor.get().get()->AsActorState()->IsBleedingOut();
	return false;
}

bool ActorInfo::IsSleeping()
{
	aclock;
	if (!valid || dead)
		return false;

	if (actor.get() && actor.get().get())
		return actor.get().get()->AsActorState()->actorState1.sitSleepState == RE::SIT_SLEEP_STATE::kIsSleeping || actor.get().get()->AsActorState()->actorState1.sitSleepState == RE::SIT_SLEEP_STATE::kWaitingForSleepAnim;
	return false;
}

/*
class PerkEntryVisitorActor : public RE::PerkEntryVisitor
{
public:
	std::list<RE::BGSPerkEntry*> entries;

	int32_t GetDosage()
	{
		entries.sort([](RE::BGSPerkEntry* first, RE::BGSPerkEntry* second) {
			return first->GetPriority() > second->GetPriority();
		});
		auto itr = entries.begin();
		while (itr != entries.end())
		{
			switch ((*itr)->GetFunction() == RE::BGSPerkEntry::EntryPoint::kModPoisonDoseCount)
				(*itr)->GetFunctionData()
			itr++;
		}
	}

	RE::BSContainer::ForEachResult Visit(RE::BGSPerkEntry* a_perkEntry) override
	{
		if (a_perkEntry)
			entries.push_back(a_perkEntry);
		return RE::BSContainer::ForEachResult::kContinue;
	}
};
*/

/// <summary>
/// check whether a magic item has an effect that has PoisonResistance as its resistance value
/// </summary>
/// <param name="item"></param>
/// <returns></returns>
bool HasPoisonResistValue(RE::MagicItem* item)
{
	loginfo("");
	RE::EffectSetting* sett = nullptr;
	if ((item->avEffectSetting) == nullptr && item->effects.size() == 0) {
		loginfo("fail: no item effects");
		return false;
	}
	bool poison = false;
	if (item->effects.size() > 0) {
		for (uint32_t i = 0; i < item->effects.size(); i++) {
			sett = item->effects[i]->baseEffect;
			if (sett && sett->data.resistVariable == RE::ActorValue::kPoisonResist) {
				poison = true;
			}
		}
	} else {
		if (item->avEffectSetting && item->avEffectSetting->data.resistVariable == RE::ActorValue::kPoisonResist) {
			poison = true;
		}
	}

	loginfo("slow doing");
	// save calculated values to data
	return poison;
}

bool ActorInfo::IsPoisoned()
{
	aclock;
	if (!valid || dead)
		return false;

	if (actor.get() && actor.get().get()) {
		RE::Actor* act = actor.get().get();
		auto list = act->GetMagicTarget()->GetActiveEffectList();
		if (list) {
			auto itr = list->begin();
			while (itr != list->end()) {
				if ((*itr)->GetBaseObject()->IsDetrimental()) {
					//if (RE::AlchemyItem* alch = (*itr)->spell->As<RE::AlchemyItem>(); alch != nullptr)
					//	if (alch->IsPoison())
					//		return true;
					return HasPoisonResistValue((*itr)->spell);
				}
				itr++;
			}
		}
	}
	return false;
}

void ActorInfo::DamageActorValue(RE::ActorValue av, float value)
{
	aclock;
	if (!valid || dead)
		return;
	if (actor.get())
	{
		if (auto act = actor.get().get())
		{
			loginfo("damage av: {}", value);
			act->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, av, -value);
		}
	}
}

void ActorInfo::RestoreActorValue(RE::ActorValue av, float value)
{
	aclock;
	if (!valid || dead)
		return;
	if (actor.get()) {
		if (auto act = actor.get().get()) {
			act->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, av, value);
		}
	}
}

float ActorInfo::GetAV(RE::ActorValue av)
{
	aclock;
	if (!valid || dead)
		return false;

	if (actor.get() && actor.get().get()) {
		return actor.get().get()->AsActorValueOwner()->GetActorValue(av);
	}
	return 0;
}

#pragma endregion
