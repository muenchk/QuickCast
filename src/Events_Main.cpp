
#include <chrono>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <thread>

#include "Animations.h"
#include "Events.h"
#include "Game.h"
#include "Logging.h"
#include "Settings.h"
#include "Utility.h"
#include "BufferOperations.h"
#include "AnimationEvents.h"
#include "ActorInfo.h"

namespace Events
{
#define EvalProcessing()   \
	if (!enableProcessing) \
		return;
#define GetProcessing() \
	enableProcessing
#define WaitProcessing()      \
	while (!enableProcessing) \
		std::this_thread::sleep_for(100ms);
#define EvalProcessingEvent() \
	if (!Main::CanProcess())  \
		return EventResult::kContinue;

	//-------------------Random-------------------------

	/// <summary>
	/// random number generator for processing probabilities
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	static std::mt19937 rand((unsigned int)(std::chrono::system_clock::now().time_since_epoch().count()));
	/// <summary>
	/// trims random numbers to 1 to 100
	/// </summary>
	static std::uniform_int_distribution<signed> rand100(1, 100);
	/// <summary>
	/// trims random numbers to 1 to 3
	/// </summary>
	static std::uniform_int_distribution<signed> rand3(1, 3);

	/*
	/// <summary>
	/// Refreshes important runtime data of an ActorInfo, including combatdata and status
	/// </summary>
	/// <param name="acinfo"></param>
	void Main::HandleActorRuntimeData(std::shared_ptr<ActorInfo> acinfo)
	{
		if (!acinfo->IsValid()) {
			LOG_1("Actor is invalid");
			return;
		}
		// if global cooldown greater zero, we can skip everything
		if (acinfo->GetGlobalCooldownTimer() > tolerance) {
			LOG_1("Actor has active cooldown exceeding tolerance");
			acinfo->SetHandleActor(false);
			return;
		}
		// if npc 3d isn't loaded, skip them
		if (acinfo->Is3DLoaded() == false)
		{
			LOG_5("3d not loaded {}", Utility::PrintForm(acinfo));
			acinfo->SetHandleActor(false);
			return;
		}
		LOG_1("{}", Utility::PrintForm(acinfo));
		LOG_2("cooldowns: durHealth:{}\tdurMagicka:{}\tdurStamina:{}\tdurFortify:{}\tdurRegen:{}", acinfo->GetDurHealth(), acinfo->GetDurMagicka(), acinfo->GetDurStamina(), acinfo->GetDurFortify(), acinfo->GetDurRegeneration());
		// check for staggered option
		// check for paralyzed
		if (comp->DisableItemUsageWhileParalyzed()) {
			if (acinfo->IsParalyzed() ||
				acinfo->IsInKillMove() ||
				acinfo->IsInRagdollState() ||
				acinfo->IsUnconscious() ||
				acinfo->IsStaggered()) {
				LOG_1("Actor is unable to use items");
				acinfo->SetHandleActor(false);
				return;
			}
		}
		if (comp->DisableItemUsageWhileFlying()) {
			if (acinfo->IsFlying() ||
				acinfo->IsInMidair()) {
				LOG_1("Actor is in the air and unable to use items");
				acinfo->SetHandleActor(false);
				return;
			}
		}
		if (comp->DisableItemUsageWhileBleedingOut()) {
			if (acinfo->IsBleedingOut()) {
				LOG_1("Actor is bleeding out and unable to use items");
				acinfo->SetHandleActor(false);
				return;
			}
		}
		if (comp->DisableItemUsageWhileBleedingOut()) {
			if (acinfo->IsSleeping()) {
				LOG_1("Actor is sleeping and unable to use items");
				acinfo->SetHandleActor(false);
				return;
			}
		}
		// check for non-follower option
		if (Settings::Usage::_DisableNonFollowerNPCs && acinfo->IsFollower() == false && acinfo->IsPlayer() == false) {
			LOG_2("Actor is not a follower, and non-follower processing has been disabled");
			acinfo->SetHandleActor(false);
			return;
		}

		auto CheckHandle = [](RE::ActorHandle handle) {
			if (handle && handle.get() && handle.get().get())
				return data->FindActor(handle.get().get());
			else
				return std::shared_ptr<ActorInfo>{};
		};

		// reset target
		acinfo->ResetTarget();
		acinfo->SetCombatDataTarget(0);

		// only try to get combat target, if the actor is actually in combat
		if (acinfo->IsInCombat()) {
			// get combatdata of current actor
			acinfo->SetCombatData(Utility::GetCombatData(acinfo->GetActor()));
			LOG_2("CombatData: {}", Utility::GetHex(acinfo->GetCombatData()));
			RE::ActorHandle handle;
			if (acinfo->IsPlayer() == false && acinfo->GetActor() != nullptr) {
				// retrieve target of current actor if present
				acinfo->SetTarget(CheckHandle(acinfo->GetActor()->GetActorRuntimeData().currentCombatTarget));
				if (std::shared_ptr<ActorInfo> tar = acinfo->GetTarget().lock()) {
					// we can make the usage dependent on the target
					acinfo->SetCombatDataTarget(Utility::GetCombatData(tar->GetActor()));
				}
			} else {
				// try to find out the players combat target, since we cannot get it the normal way

				// if we have access to the True Directional Movement API and target lock is activated
				// try to get the actor from there
				if (Settings::Interfaces::tdm_api != nullptr && Settings::Interfaces::tdm_api->GetTargetLockState() == true) {
					acinfo->SetTarget(CheckHandle(Settings::Interfaces::tdm_api->GetCurrentTarget()));
				}
				if (std::shared_ptr<ActorInfo> tar = acinfo->GetTarget().lock()) {
				} else {
					// try to infer the target from the npcs that are in combat
					// get the combatant with the shortest range to player, which is hostile to the player
					auto GetClosestEnemy = []() {
						std::shared_ptr<ActorInfo> current = nullptr;
						for (auto aci : combatants) {
							if (current == nullptr || (aci != nullptr && aci->GetPlayerHostile() && aci->GetPlayerDistance() < current->GetPlayerDistance()))
								current = aci;
						}
						return current;
					};
					acinfo->SetTarget(GetClosestEnemy());
				}
			}
		}

		// if actor is valid and not dead
		if (acinfo->IsValid() && !acinfo->IsDead() && acinfo->GetActor() && acinfo->GetActor()->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth) > 0) {
			LOG_1("Handle Actor");
			acinfo->SetHandleActor(true);
		} else {
			LOG_1("invalid, dead, etc.");
			acinfo->SetHandleActor(false);
		}

		if (acinfo->IsInCombat()) {
			// increase time spent in combat
			acinfo->IncDurCombat(1000);
		} else {
			// reset time spent in combat
			acinfo->SetDurCombat(0);
		}

		// get whether weapons are drawn
		acinfo->UpdateWeaponsDrawn();
	}
	*/

	/// <summary>
	/// Finds actors that are temporarily banned from processing
	/// </summary>
	void Main::PullForbiddenActors()
	{
		// delete old forbiddens
		forbidden.clear();
		if (DGIntimidate != nullptr && DGIntimidate->IsRunning()) {
			// find all npcs participating in a brawl and add them to the exceptions
			for (auto& [id, objectrefhandle] : DGIntimidate->refAliasMap) {
				loginfo("Alias with id: {}", id);
				if (id == 0 || id == 352 || id == 351) {
					if (objectrefhandle && objectrefhandle.get() && objectrefhandle.get().get()) {
						if (objectrefhandle.get().get()->formType == RE::FormType::ActorCharacter) {
							if (RE::Actor* ac = objectrefhandle.get().get()->As<RE::Actor>(); ac != nullptr) {
								forbidden.insert(ac->GetFormID());
								loginfo("Found Brawling actor: {}", Utility::GetHex(ac->GetFormID()));
							}
						}
					}
				}
			}
		}
	}

	/// <summary>
	/// Callback on saving the game
	/// </summary>
	/// <param name=""></param>
	void Main::SaveGameCallback(SKSE::SerializationInterface* /*a_intfc*/)
	{
		loginfo("");
	}

	/// <summary>
	/// Callback on loading a save game, initializes actor processing
	/// </summary>
	/// <param name=""></param>
	void Main::LoadGameCallback(SKSE::SerializationInterface* /*a_intfc*/)
	{
		loginfo("");
		StartProfiling;
		auto begin = std::chrono::steady_clock::now();
		// if we canceled the main thread, reset that
		initialized = false;

		// reset event time_map
		EventHandler::GetSingleton()->time_map.clear();

		// reset the list of actors that died
		deads.clear();
		// set player to alive
		PlayerDied((bool)(RE::PlayerCharacter::GetSingleton()->GetActorRuntimeData().boolBits & RE::Actor::BOOL_BITS::kDead) || RE::PlayerCharacter::GetSingleton()->IsDead());
		enableProcessing = true;

		InitializeCompatibilityObjects();

		// get the MCM quest and start it if its not running
		if (Utility::Mods::GetPluginIndex(Settings::PluginName) != 0x1) {
			RE::TESForm* form = Data::GetSingleton()->FindForm(0x800, Settings::PluginName);
			if (form) {
				RE::TESQuest* q = form->As<RE::TESQuest>();
				if (q && q->IsRunning() == false) {
					q->Start();
				}
			}
		}

		PlayerAnimationHandler::RegisterEvents(RE::PlayerCharacter::GetSingleton());

		loginfo("end");
		profile(TimeProfiling, "function execution time");
	}

	/// <summary>
	/// Callback on reverting the game. Disables processing and stops all handlers
	/// </summary>
	/// <param name=""></param>
	void Main::RevertGameCallback(SKSE::SerializationInterface* /*a_intfc*/)
	{
		loginfo("");
		enableProcessing = false;
		loginfo("{}", playerdied);
		DGIntimidate = nullptr;
	}

	long Main::SaveDeadActors(SKSE::SerializationInterface* a_intfc)
	{
		loginfo("Writing dead actors");
		loginfo("{} dead actors to write", deads.size());

		long size = 0;
		long successfulwritten = 0;

		for (auto& handle : deads) {
			if (RE::Actor* actor = handle.get().get(); actor != nullptr) {
				RE::FormID id = actor->GetFormID();
				uint32_t formid = Utility::Mods::GetIndexLessFormID(id);
				std::string pluginname = Utility::Mods::GetPluginNameFromID(id);
				if (a_intfc->OpenRecord('EDID', 0)) {
					// get entry length
					int length = 4 + Buffer::CalcStringLength(pluginname);
					// save written bytes number
					size += length;
					// create buffer
					unsigned char* buffer = new unsigned char[length + 1];
					if (buffer == nullptr) {
						logwarn("failed to write Dead Actor record: buffer null");
						continue;
					}
					// fill buffer
					size_t offset = 0;
					Buffer::Write(id, buffer, offset);
					Buffer::Write(pluginname, buffer, offset);
					// write record
					a_intfc->WriteRecordData(buffer, length);
					delete[] buffer;
					successfulwritten++;
				}
			}
		}

		loginfo("Writing dead actors finished");

		return size;
	}

	long Main::ReadDeadActors(SKSE::SerializationInterface* a_intfc, uint32_t length)
	{
		long size = 0;

		loginfo("Reading Dead Actor...");
		unsigned char* buffer = new unsigned char[length];
		a_intfc->ReadRecordData(buffer, length);
		if (length >= 12) {
			size_t offset = 0;
			uint32_t formid = Buffer::ReadUInt32(buffer, offset);
			std::string pluginname = Buffer::ReadString(buffer, offset);
			RE::TESForm* form = RE::TESDataHandler::GetSingleton()->LookupForm(formid, pluginname);
			if (form) {
				if (RE::Actor* actor = form->As<RE::Actor>(); actor != nullptr) {
					deads.insert(actor->GetHandle());
				}
			}
		}
		delete[] buffer;

		return size;
	}

	/// <summary>
	/// initializes important variables, which need to be initialized every time a game is loaded
	/// </summary>
	void Main::InitializeCompatibilityObjects()
	{
		EvalProcessing();
		// now that the game was loaded we can try to initialize all our variables we conuldn't before
		if (!initialized) {
			if (DGIntimidate == nullptr) {
				DGIntimidate = RE::TESForm::LookupByID<RE::TESQuest>(0x00047AE6);
				if (DGIntimidate == nullptr)
					logcritical("Cannot find DGIntimidate quest");
			}

			initialized = true;
		}
	}

	bool Main::IsDead(RE::Actor* actor)
	{
		return actor == nullptr || deads.contains(actor->GetHandle()) || actor->IsDead();
	}

	bool Main::IsDeadEventFired(RE::Actor* actor)
	{
		return actor == nullptr || deads.contains(actor->GetHandle());
	}

	void Main::SetDead(RE::ActorHandle actor)
	{
		if (actor.get().get())
			deads.insert(actor);
	}

	void Main::TrySetHotkey(uint32_t key)
	{
		try {
			if (ui->IsMenuOpen(RE::MagicMenu::MENU_NAME)) {
				auto* magMenu = static_cast<RE::MagicMenu*>(ui->GetMenu(RE::MagicMenu::MENU_NAME).get());
				if (!magMenu) {
					loginfo("failed to get inventory menu");
					return;
				}
				RE::TESForm* form = nullptr;
				RE::GFxValue result;
				magMenu->uiMovie->GetVariable(&result, "_root.Menu_mc.inventoryLists.itemList.selectedEntry.formId");
				if (result.GetType() == RE::GFxValue::ValueType::kNumber) {
					RE::FormID formID = static_cast<std::uint32_t>(result.GetNumber());
					form = RE::TESForm::LookupByID(formID);
				}
				if (!form) {
					loginfo("failed to get underlying spell info");
					return;
				}
				switch (form->GetFormType()) {
				case RE::FormType::Spell:
					{
						loginfo("{}", Utility::PrintForm(form->As<RE::SpellItem>()));

						data->AddHotkey(key, CastingOrigin::kLeftHand, false, form);
					}
					break;
				case RE::FormType::Shout:
					{
						loginfo("{}", Utility::PrintForm(form->As<RE::TESShout>()));

						data->AddHotkey(key, CastingOrigin::kLeftHand, false, form);
					}
					break;
				}
			}
			/* RE::InventoryMenu* inventoryMenu = static_cast<RE::InventoryMenu*>(ui->GetMenu(RE::InventoryMenu::MENU_NAME).get());
					if (inventoryMenu) {
						RE::GFxValue result;
						inventoryMenu->uiMovie->GetVariable(&result, "_root.MenuHolder.Menu_mc.itemList._selectedIndex");
						if (result.GetType() == RE::GFxValue::ValueType::kNumber) {
						}
					}*/
		} catch (std::exception& e) {
			loginfo("Exception: {}", e.what());
		}
	}

	void Main::Init()
	{
		ui = RE::UI::GetSingleton();
	}

	RE::BSSoundHandle Main::PlaySound(RE::BGSSoundDescriptorForm* soundDesc, std::shared_ptr<ActorInfo> acinfo)
	{
		RE::BSSoundHandle handle;
		if (soundDesc) {
			Events::Main::audiomanager->BuildSoundDataFromDescriptor(handle, soundDesc);
			handle.SetObjectToFollow(acinfo->GetActor()->Get3D());
			handle.SetVolume(1.0);
			handle.Play();
		}
		return handle;
	}

	void Main::OnFrame()
	{
		// if game is apused no events should be fired, and the timing on all event should be adjustest
		if (ui->GameIsPaused()) {
			paused = true;
			return;
		}
		uint64_t offset = 0;
		QueryUnbiasedInterruptTime(&frameTime);
		// is the game is no longer paused, calculate the event offset
		if (paused) {
			offset = frameTime - frameTimeLast;
			paused = false;
			PlayerAnimationHandler::GetSingleton()->AdjustOffsets(offset);
		}
		frameTimeLast = frameTime;

		auto itr = _eventQueue.begin();
		while (itr != _eventQueue.end()) {
			auto event = *itr;
			if (!event || event->GetInterrupt()) {
				itr = _eventQueue.erase(itr);
				continue;
			}
			event->ticks += offset;
			if (frameTime > event->ticks) {
				loginfo("OnFrame: Handle event {}", event->identifier);
				// we have reached our target can wll now handle the event
				switch (event->event) {
				case EventType::CastAimedLeft:
					if (Animations::CastAimedLeft(event)) {
						itr = _eventQueue.erase(itr);
						continue;
					}
					break;
				case EventType::CastAimedRight:
					if (Animations::CastAimedRight(event)) {
						itr = _eventQueue.erase(itr);
						continue;
					}
					break;
				case EventType::CastSelfLeft:
					if (Animations::CastSelfLeft(event)) {
						itr = _eventQueue.erase(itr);
						continue;
					}
					break;
				case EventType::CastSelfRight:
					if (Animations::CastSelfRight(event)) {
						itr = _eventQueue.erase(itr);
						continue;
					}
					break;
				case EventType::CastConcAimedLeft:
					if (Animations::CastConcAimedLeft(event)) {
						itr = _eventQueue.erase(itr);
						continue;
					}
					break;
				case EventType::CastConcAimedRight:
					if (Animations::CastConcAimedRight(event)) {
						itr = _eventQueue.erase(itr);
						continue;
					}
					break;
				case EventType::CastConcSelfLeft:
					if (Animations::CastConcSelfLeft(event)) {
						itr = _eventQueue.erase(itr);
						continue;
					}
					break;
				case EventType::CastConcSelfRight:
					if (Animations::CastConcSelfRight(event)) {
						itr = _eventQueue.erase(itr);
						continue;
					}
					break;
				case EventType::CastWardLeft:
					if (Animations::CastWardLeft(event)) {
						itr = _eventQueue.erase(itr);
						continue;
					}
					break;
				case EventType::CastWardRight:
					if (Animations::CastWardRight(event)) {
						itr = _eventQueue.erase(itr);
						continue;
					}
					break;
				case EventType::DualCastAimed:
					if (Animations::DualCastAimed(event)) {
						itr = _eventQueue.erase(itr);
						continue;
					}
					break;
				case EventType::DualCastSelf:
					if (Animations::DualCastSelf(event)) {
						itr = _eventQueue.erase(itr);
						continue;
					}
					break;
				case EventType::DualCastConcAimed:
					if (Animations::DualCastConcAimed(event)) {
						itr = _eventQueue.erase(itr);
						continue;
					}
					break;
				case EventType::DualCastConcSelf:
					if (Animations::DualCastConcSelf(event)) {
						itr = _eventQueue.erase(itr);
						continue;
					}
					break;
				case EventType::DualCastWard:
					if (Animations::DualCastWard(event)) {
						itr = _eventQueue.erase(itr);
						continue;
					}
					break;
				case EventType::Shout:
					if (Animations::Shout(event)) {
						itr = _eventQueue.erase(itr);
						continue;
					}
					break;
				case EventType::Ritual:
					if (Animations::Ritual(event)) {
						itr = _eventQueue.erase(itr);
						continue;
					}
					break;
				}
			}
			itr++;
		}
	}
}
