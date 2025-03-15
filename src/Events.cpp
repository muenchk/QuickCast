#include <string.h>
#include <chrono>
#include <thread>
#include <forward_list>
#include <semaphore>
#include <stdlib.h>
#include <time.h>
#include <random>
#include <fstream>
#include <iostream>
#include <limits>
#include <filesystem>
#include <deque>

#include "Events.h"
#include "Game.h"
#include "Settings.h"
#include "Threading.h"
#include "Utility.h"
#include "VM.h"
#include "SpellInfo.h"
#include "AnimationEvents.h"
		
namespace Events
{
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

#define Look(s) RE::TESForm::LookupByEditorID(s)

#pragma region Data

#define EvalProcessing()   \
	if (!enableProcessing) \
		return;
#define GetProcessing() \
	enableProcessing
#define WaitProcessing() \
	while (!enableProcessing) \
		std::this_thread::sleep_for(100ms);
#define EvalProcessingEvent() \
	if (!Main::CanProcess())    \
		return EventResult::kContinue;

#define CheckDeadEvent                       \
	LOG1_1("{}[PlayerDead] {}", playerdied); \
	if (playerdied == true) {                \
		return EventResult::kContinue;       \
	}

#define ReEvalPlayerDeath                                         \
	if (RE::PlayerCharacter::GetSingleton()->IsDead() == false) { \
		playerdied = false;                                       \
	}

#define CheckDeadCheckHandlerLoop \
	if (playerdied) {             \
		break;                    \
	}                                              


#pragma endregion

	/// <summary>
	/// Processes TESHitEvents
	/// </summary>
	/// <param name=""></param>
	/// <param name=""></param>
	/// <returns></returns>
	EventResult EventHandler::ProcessEvent(const RE::TESHitEvent* a_event, RE::BSTEventSource<RE::TESHitEvent>*)
	{
		EvalProcessingEvent();
		
		if (a_event && a_event->target.get()) {
			RE::Actor* actor = a_event->target.get()->As<RE::Actor>();
			if (actor) {
			}
		}

		return EventResult::kContinue;
	}

	/// <summary>
	/// EventHandler for TESDeathEvent
	/// removed unused potions and poisons from actor, to avoid economy instability
	/// only registered if itemremoval is activated in the settings
	/// </summary>
	/// <param name="a_event"></param>
	/// <param name="a_eventSource"></param>
	/// <returns></returns>
	EventResult EventHandler::ProcessEvent(const RE::TESDeathEvent* a_event, RE::BSTEventSource<RE::TESDeathEvent>*)
	{
		EvalProcessingEvent();
		StartProfiling;
		loginfo("[TESDeathEvent]");
		Main::InitializeCompatibilityObjects();
		RE::Actor* actor = nullptr;
		if (a_event == nullptr || a_event->actorDying == nullptr) {
			loginfo("[TESDeathEvent] Died due to invalid event");
			goto TESDeathEventEnd;
		}
		actor = a_event->actorDying->As<RE::Actor>();
		if (!Utility::ValidateActor(actor)) {
			loginfo("[TESDeathEvent] Died due to actor validation fail");
			goto TESDeathEventEnd;
		}
		if (Utility::ValidateActor(actor)) {
			if (actor->IsPlayerRef()) {
				loginfo("[TESDeathEvent] player died");
				Main::PlayerDied(true);
			} else {
				// if not already dead, do stuff
				if (Main::IsDeadEventFired(actor) == false) {
					EvalProcessingEvent();
					EvalProcessingEvent();
					// invalidate actor
					std::shared_ptr<ActorInfo> acinfo = Main::data->FindActorExisting(actor->GetFormID());
					// if invalid return
					if (!acinfo->IsValid())
						return EventResult::kContinue;
					Main::SetDead(acinfo->GetHandle());
					acinfo->SetDead();
					// delete actor from data
					Main::data->DeleteActor(acinfo->GetFormID());
					acinfo->SetInvalid();
				}
			}
		}
TESDeathEventEnd:
		profile(TimeProfiling, "[TESDeathEvent] event execution time.");
		return EventResult::kContinue;
	}

	/// <summary>
	/// handles TESCombatEvent
	/// registers the actor for tracking and handles giving them potions, poisons and food, beforehand.
	/// also makes then eat food before the fight.
	/// </summary>
	/// <param name="a_event">event parameters like the actor we need to handle</param>
	/// <param name=""></param>
	/// <returns></returns>
	EventResult EventHandler::ProcessEvent(const RE::TESCombatEvent* a_event, RE::BSTEventSource<RE::TESCombatEvent>*)
	{
		EvalProcessingEvent();
		StartProfiling;
		loginfo("[TESCombatEvent]");
		Main::InitializeCompatibilityObjects();
		auto actor = a_event->actor->As<RE::Actor>();
		if (Utility::ValidateActor(actor) && !Main::IsDead(actor) && actor != RE::PlayerCharacter::GetSingleton() && actor->IsChild() == false) {
			// register / unregister
			if (a_event->newState == RE::ACTOR_COMBAT_STATE::kCombat || a_event->newState == RE::ACTOR_COMBAT_STATE::kSearching) {
				// register for tracking
				
			} else {
				
			}

			// save combat state of npc
			std::shared_ptr<ActorInfo> acinfo = Main::data->FindActor(actor);
			if (a_event->newState == RE::ACTOR_COMBAT_STATE::kCombat)
				acinfo->SetCombatState(CombatState::InCombat);
			else if (a_event->newState == RE::ACTOR_COMBAT_STATE::kSearching)
				acinfo->SetCombatState(CombatState::Searching);
			else if (a_event->newState == RE::ACTOR_COMBAT_STATE::kNone)
				acinfo->SetCombatState(CombatState::OutOfCombat);

		}
		profile(TimeProfiling, "[TESCombatEvent] event execution time");
		return EventResult::kContinue;
	}

	/// <summary>
	/// EventHandler for Debug purposes. It calculates the distribution rules for all npcs in the cell
	/// </summary>
	/// <param name="a_event"></param>
	/// <param name="a_eventSource"></param>
	/// <returns></returns>
	EventResult EventHandler::ProcessEvent(const RE::TESEquipEvent* a_event, RE::BSTEventSource<RE::TESEquipEvent>*)
	{
		EvalProcessingEvent();
		StartProfiling;
		if (a_event->actor.get()) {
			if (a_event->actor->IsPlayerRef()) {
				auto audiomanager = RE::BSAudioManager::GetSingleton();

				RE::AlchemyItem* obj = RE::TESForm::LookupByID<RE::AlchemyItem>(a_event->baseObject);
				if (obj) {
					if (obj->data.consumptionSound) {
						RE::BSSoundHandle handle;
						audiomanager->BuildSoundDataFromDescriptor(handle, obj->data.consumptionSound->soundDescriptor);
						handle.SetObjectToFollow(a_event->actor->Get3D());
						handle.SetVolume(1.0);
						handle.Play();
					}
				}
				profile(TimeProfiling, "[TESEquipEvent] event execution time.");
			}
		}

		return EventResult::kContinue;
	}

	/// <summary>
	/// Handles an item being removed from a container
	/// </summary>
	/// <param name="container">The container the item was removed from</param>
	/// <param name="baseObj">The base object that has been removed</param>
	/// <param name="count">The number of items that have been removed</param>
	/// <param name="destinationContainer">The container the items have been moved to</param>
	/// <param name="a_event">The event information</param>
	void EventHandler::OnItemRemoved(RE::TESObjectREFR* container, RE::TESBoundObject* baseObj, int /*count*/, RE::TESObjectREFR* /*destinationContainer*/, const RE::TESContainerChangedEvent* /*a_event*/)
	{
		loginfo("[OnItemRemovedEvent] {} removed from {}", Utility::PrintForm(baseObj), Utility::PrintForm(container));
		RE::Actor* actor = container->As<RE::Actor>();
		if (actor) {
			// handle event for an actor
			//std::shared_ptr<ActorInfo> acinfo = data->FindActor(actor);
			/* if (comp->LoadedAnimatedPoisons()) {
				// handle removed poison
				RE::AlchemyItem* alch = baseObj->As<RE::AlchemyItem>();
				if (alch && alch->IsPoison()) {
					LOG_1("[OnItemRemovedEvent] AnimatedPoison animation");

					//ACM::AnimatedPoison_ApplyPoison(acinfo, alch);

					//std::string AnimationEventString = "poisondamagehealth02";
					//acinfo->actor->NotifyAnimationGraph(AnimationEventString);
						
					//RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> point(nullptr);
					//a_vm->DispatchStaticCall("Debug", "SendAnimationEvent", SKSE::Impl::VMArg(actor, RE::BSFixedString("poisondamagehealth02")), point);
					//RE::MakeFunctionArguments(actor, RE::BSFixedString("poisondamagehealth02"));
				}
			}
			*/
		}
		
		// handle event for generic reference
		
		return;
	}

	/// <summary>
	/// Handles an item being added to a container
	/// </summary>
	/// <param name="container">The container the item is added to</param>
	/// <param name="baseObj">The base object that has been added</param>
	/// <param name="count">The number of items added</param>
	/// <param name="sourceContainer">The container the item was in before</param>
	/// <param name="a_event">The event information</param>
	void EventHandler::OnItemAdded(RE::TESObjectREFR* container, RE::TESBoundObject* baseObj, int /*count*/, RE::TESObjectREFR* sourceContainer, const RE::TESContainerChangedEvent* /*a_event*/)
	{
		loginfo("[OnItemAddedEvent] {} added to {}", Utility::PrintForm(baseObj), Utility::PrintForm(container));
		RE::Actor* actor = container->As<RE::Actor>();
		if (actor) {
			// handle event for an actor
			//std::shared_ptr<ActorInfo> acinfo = data->FindActor(actor);
			
			// if actor is the player character
			if (actor->IsPlayerRef()) {
				if (sourceContainer != nullptr) {
					if (auto itr = time_map.find(sourceContainer->GetFormID()); itr != time_map.end()) {
						if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - itr->second).count() > 180) {
							time_map.insert({ sourceContainer->GetFormID(), std::chrono::steady_clock::now() });
							SKSE::ModCallbackEvent* ev = new SKSE::ModCallbackEvent();
							ev->eventName = RE::BSFixedString("NPCsUsePotions_quicklooteventcontainer");
							ev->strArg = "";
							ev->numArg = 0.0f;
							ev->sender = sourceContainer;
							SKSE::GetModCallbackEventSource()->SendEvent(ev);
						}
					} else {
						time_map.insert({ sourceContainer->GetFormID(), std::chrono::steady_clock::now() });
						SKSE::ModCallbackEvent* ev = new SKSE::ModCallbackEvent();
						ev->eventName = RE::BSFixedString("NPCsUsePotions_quicklooteventcontainer");
						ev->strArg = "";
						ev->numArg = 0.0f;
						ev->sender = sourceContainer;
						SKSE::GetModCallbackEventSource()->SendEvent(ev);
					}
				}
			}
		}
		
		// handle event for generic objects
		return;
	}

	EventResult EventHandler::ProcessEvent(const RE::TESContainerChangedEvent* a_event, RE::BSTEventSource<RE::TESContainerChangedEvent>* /*a_eventSource*/)
	{
		// this event handles all object transfers between containers in the game
		// this can be deived into multiple base events: OnItemRemoved and OnItemAdded
		EvalProcessingEvent();

		if (a_event && a_event->baseObj != 0 && a_event->itemCount != 0) {
			RE::TESObjectREFR* oldCont = RE::TESForm::LookupByID<RE::TESObjectREFR>(a_event->oldContainer);
			RE::TESObjectREFR* newCont = RE::TESForm::LookupByID<RE::TESObjectREFR>(a_event->newContainer);
			RE::TESBoundObject* baseObj = RE::TESForm::LookupByID<RE::TESBoundObject>(a_event->baseObj);
			if (baseObj && oldCont) {
				OnItemRemoved(oldCont, baseObj, a_event->itemCount, newCont, a_event);
			}
			if (baseObj && newCont) {
				OnItemAdded(newCont, baseObj, a_event->itemCount, oldCont, a_event);
			}
		}

		return EventResult::kContinue;
	}

	/// <summary>
	/// EventHandler for end of fast travel
	/// </summary>
	/// <param name="a_event"></param>
	/// <param name="a_eventSource"></param>
	/// <returns></returns>
	EventResult EventHandler::ProcessEvent(const RE::TESFastTravelEndEvent* a_event, RE::BSTEventSource<RE::TESFastTravelEndEvent>*)
	{
		// very important event. Allows to catch actors and other stuff that gets deleted, without dying, which could cause CTDs otherwise
		
		loginfo("[TESFastTravelEndEvent]");

		Game::SetFastTraveling(false);

		return EventResult::kContinue;
	}

	EventResult EventHandler::ProcessEvent(const RE::TESActivateEvent* a_event, RE::BSTEventSource<RE::TESActivateEvent>*)
	{
		// currently unused since another solution for fasttravel tracking has been found

		if (a_event && a_event->actionRef.get() && a_event->objectActivated.get()) {
			if (a_event->actionRef->IsPlayerRef())
				loginfo("[TESActivateEvent] Activated {}", Utility::PrintForm(a_event->objectActivated.get()));
			// objects valid
			static RE::TESQuest* DialogueCarriageSystem = RE::TESForm::LookupByID<RE::TESQuest>(0x17F01);
			if (a_event->actionRef->IsPlayerRef() && a_event->objectActivated.get()->GetBaseObject()->GetFormID() == 0x103445) {
				loginfo("[TESActivateEvent] Activated Carriage Marker");
				auto scriptobj = ScriptObject::FromForm(DialogueCarriageSystem, "CarriageSystemScript");
				if (scriptobj.get()) {
					auto currentDestination = scriptobj->GetProperty("currentDestination");
					if (currentDestination) {
						if (currentDestination->GetSInt() != 0 && currentDestination->GetSInt() != -1) {
							loginfo("[TESActivateEvent] Recognized player fasttraveling via carriage");
							Game::SetFastTraveling(true);
						}
					}
				}
			}
		}

		return EventResult::kContinue;
	}

	
	RE::Actor* target = nullptr;
	RE::TESObjectREFR* targetobj = nullptr;

	EventResult EventHandler::ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>* /*a_eventSource*/)
	{
		using EventType = RE::INPUT_EVENT_TYPE;
		using DeviceType = RE::INPUT_DEVICE;

		if (!a_event) {
			return EventResult::kContinue;
		}

		std::set<uint32_t> recurring;

		for (auto event = *a_event; event; event = event->next) {
			if (event->eventType != EventType::kButton) {
				continue;
			}

			auto& userEvent = event->QUserEvent();
			auto userEvents = RE::UserEvents::GetSingleton();

			RE::UI* ui = RE::UI::GetSingleton();

			auto button = static_cast<RE::ButtonEvent*>(event);
			if (button->IsDown()) {
				auto key = button->idCode;
				switch (button->device.get()) {
				case DeviceType::kMouse:
					key += 256;
					break;
				case DeviceType::kKeyboard:
					key += 0;
					break;
				default:
					continue;
				}
				// skip recurring keys
				if (recurring.contains(key))
					continue;
				recurring.insert(key);

				/* if (key == 0x26)  // L
				{
					try {
						if (ui->IsMenuOpen(RE::MagicMenu::MENU_NAME)) {
							auto* magMenu = static_cast<RE::MagicMenu*>(ui->GetMenu(RE::MagicMenu::MENU_NAME).get());
							if (!magMenu) {
								loginfo("failed to get inventory menu");
								return EventResult::kContinue;
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
								return EventResult::kContinue;
							}
							switch (form->GetFormType()) {
							case RE::FormType::Spell:
								{
									loginfo("{}", Utility::PrintForm(form->As<RE::SpellItem>()));
								}
								break;
							case RE::FormType::Shout:
								{
									loginfo("{}", Utility::PrintForm(form->As<RE::TESShout>()));
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
				/*}
					catch (std::exception& e)
					{
						loginfo("Exception: {}", e.what());
					}
				}*/

				// handle key here
				/*auto console = RE::ConsoleLog::GetSingleton();
				if (key == 0x22)  // G
				{
					RE::SpellItem* spell = Main::data->FindSpell(0x1c789, "Skyrim.esm");
					if (!spell) {
						RE::DebugNotification("Spell not found");
						logwarn("spell not found");
						return EventResult::kContinue;
					}
					auto spellinfo = Main::data->FindSpellInfo(spell);
					if (!spellinfo->IsValid()) {
						RE::DebugNotification("spellinfo invalid");
						logwarn("spellinfo invalid");
						return EventResult::kContinue;
					}
					auto playerinfo = Main::data->FindActor(RE::PlayerCharacter::GetSingleton());
					if (!playerinfo->IsValid()) {
						logwarn("playerinfo invalid");
						return EventResult::kContinue;
					}
					loginfo("casting spell");
					spellinfo->CastSpell(spellinfo, playerinfo, CastingOrigin::kLeftHand, false);
				} else if (key == 0x23) {  // H
					RE::SpellItem* spell = Main::data->FindSpell(0x1c789, "Skyrim.esm");
					if (!spell) {
						RE::DebugNotification("Spell not found");
						logwarn("spell not found");
						return EventResult::kContinue;
					}
					auto spellinfo = Main::data->FindSpellInfo(spell);
					if (!spellinfo->IsValid()) {
						RE::DebugNotification("spellinfo invalid");
						logwarn("spellinfo invalid");
						return EventResult::kContinue;
					}
					auto playerinfo = Main::data->FindActor(RE::PlayerCharacter::GetSingleton());
					if (!playerinfo->IsValid()) {
						logwarn("playerinfo invalid");
						return EventResult::kContinue;
					}
					loginfo("casting spell");
					spellinfo->CastSpell(spellinfo, playerinfo, CastingOrigin::kRightHand, false);
					return EventResult::kContinue;
				} else if (key == 0x25) {  // K
					RE::SpellItem* spell = Main::data->FindSpell(0x1c789, "Skyrim.esm");
					if (!spell) {
						RE::DebugNotification("Spell not found");
						logwarn("spell not found");
						return EventResult::kContinue;
					}
					auto spellinfo = Main::data->FindSpellInfo(spell);
					if (!spellinfo->IsValid()) {
						RE::DebugNotification("spellinfo invalid");
						logwarn("spellinfo invalid");
						return EventResult::kContinue;
					}
					auto playerinfo = Main::data->FindActor(RE::PlayerCharacter::GetSingleton());
					if (!playerinfo->IsValid()) {
						logwarn("playerinfo invalid");
						return EventResult::kContinue;
					}
					loginfo("casting spell");
					spellinfo->CastSpell(spellinfo, playerinfo, CastingOrigin::kLeftHand, true);
					return EventResult::kContinue;
				}*/

				if (ui->GameIsPaused() == false) {
					auto hotkey = Main::data->FindHotkey(key);
					if (hotkey) {
						RE::SpellItem* spell = hotkey->form->As<RE::SpellItem>();
						RE::TESShout* shout = hotkey->form->As<RE::TESShout>();
						if (spell)
						{
							auto spellinfo = Main::data->FindSpellInfo(spell);
							if (!spellinfo->IsValid())
							{
								RE::DebugNotification("spellinfo invalid");
								logwarn("spellinfo invalid");
								return EventResult::kContinue;
							}
							auto playerinfo = Main::data->FindActor(RE::PlayerCharacter::GetSingleton());
							if (!playerinfo->IsValid()) {
								logwarn("playerinfo invalid");
								return EventResult::kContinue;
							}
							loginfo("casting spell");
							spellinfo->CastSpell(spellinfo, playerinfo, hotkey->origin, hotkey->dualCast);
							return EventResult::kContinue;
						}
						else if (shout)
						{

						}
					}
				}
			}
		}
		return EventResult::kContinue;
	}

    /// <summary>
    /// returns singleton to the EventHandler
    /// </summary>
    /// <returns></returns>
    EventHandler* EventHandler::GetSingleton()
    {
        static EventHandler singleton;
        return std::addressof(singleton);
    }


    /// <summary>
    /// Registers us for all Events we want to receive
    /// </summary>
	void EventHandler::Register()
	{
		auto scriptEventSourceHolder = RE::ScriptEventSourceHolder::GetSingleton();
		scriptEventSourceHolder->GetEventSource<RE::TESHitEvent>()->AddEventSink(EventHandler::GetSingleton());
		loginfo("Registered {}", typeid(RE::TESHitEvent).name());
		scriptEventSourceHolder->GetEventSource<RE::TESCombatEvent>()->AddEventSink(EventHandler::GetSingleton());
		loginfo("Registered {}", typeid(RE::TESCombatEvent).name());
		scriptEventSourceHolder->GetEventSource<RE::TESDeathEvent>()->AddEventSink(EventHandler::GetSingleton());
		loginfo("Registered {}", typeid(RE::TESDeathEvent).name());
		scriptEventSourceHolder->GetEventSource<RE::TESEquipEvent>()->AddEventSink(EventHandler::GetSingleton());
		loginfo("Registered {}", typeid(RE::TESEquipEvent).name());
		scriptEventSourceHolder->GetEventSource<RE::TESContainerChangedEvent>()->AddEventSink(EventHandler::GetSingleton());
		loginfo("Registered {}", typeid(RE::TESContainerChangedEvent).name())
		scriptEventSourceHolder->GetEventSource<RE::TESFastTravelEndEvent>()->AddEventSink(EventHandler::GetSingleton());
		loginfo("Registered {}", typeid(RE::TESFastTravelEndEvent).name())
		//scriptEventSourceHolder->GetEventSource<RE::TESActivateEvent>()->AddEventSink(EventHandler::GetSingleton());
		//LOG1_1("Registered {}", typeid(RE::TESActivateEvent).name())
		RE::BSInputDeviceManager::GetSingleton()->AddEventSink(EventHandler::GetSingleton());
		loginfo("Registered {}", typeid(RE::InputEvent).name());
		Game::SaveLoad::GetSingleton()->RegisterForLoadCallback(0xFF000001, Main::LoadGameCallback);
		loginfo("Registered {}", typeid(Main::LoadGameCallback).name());
		Game::SaveLoad::GetSingleton()->RegisterForRevertCallback(0xFF000002, Main::RevertGameCallback);
		loginfo("Registered {}", typeid(Main::RevertGameCallback).name());
		Game::SaveLoad::GetSingleton()->RegisterForSaveCallback(0xFF000003, Main::SaveGameCallback);
		loginfo("Registered {}", typeid(Main::SaveGameCallback).name());
		Main::data = Data::GetSingleton();
		Main::comp = Compatibility::GetSingleton();
		Main::audiomanager = RE::BSAudioManager::GetSingleton();
	}

	/// <summary>
	/// Registers all EventHandlers, if we would have multiple
	/// </summary>
	void RegisterAllEventHandlers()
	{
		EventHandler::Register();
		loginfo("Registered all event handlers"sv);
	}
}
