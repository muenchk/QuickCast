#pragma once

#include <semaphore>
#include <set>
#include <functional>

#include "Compatibility.h"
#include "Data.h"
#include "Utility.h"

struct AnimationInfo;

/// <summary>
/// standard namespacee hash function for RE::ActorHandles
/// </summary>
template <>
struct std::hash<RE::ActorHandle>
{
	std::size_t operator()(RE::ActorHandle const& handle) const noexcept
	{
		if (handle._handle.has_value())
			return std::hash<unsigned long long>{}((uintptr_t)handle._handle.value());
		else
			return 0;
	}
};

/// <summary>
/// standard namespace equality comparator for RE::ActorHandles
/// </summary>
template <>
struct std::equal_to<RE::ActorHandle>
{
	std::size_t operator()(RE::ActorHandle const& lhs, RE::ActorHandle const& rhs) const
	{
		return lhs.get().get() == rhs.get().get();
	}
};

namespace Events
{
	class Main
	{
	public:
		static inline Data* data = nullptr;
		static inline Compatibility* comp = nullptr;

	private:
		/// <summary>
		/// determines whether events and functions are run
		/// </summary>
		static inline bool initialized = false;

		//--------------------Lists---------------------------

		/// <summary>
		/// since the TESDeathEvent seems to be able to fire more than once for an actor we need to track the deaths
		/// </summary>
		static inline std::unordered_set<RE::ActorHandle> deads;
		/// <summary>
		/// set of actors temporarily forbidden from processing
		/// </summary>
		static inline std::set<RE::FormID> forbidden;

		//-------------------Processing----------------------

		/// <summary>
		/// signals whether the player has died
		/// </summary>
		static inline bool playerdied = false;

		/// <summary>
		/// enables all active functions
		/// </summary>
		static inline bool enableProcessing = false;

		//--------------------Brawl--------------------------

		static inline RE::TESQuest* DGIntimidate = nullptr;

		//----------------------Main-----------------------

		/// <summary>
		/// Finds actors that are temporarily banned from processing
		/// </summary>
		static void PullForbiddenActors();

	public:


		static void Init();

		//----------------------Processing--------------------------

		/// <summary>
		/// Returns whether processing of actors is allowed
		/// </summary>
		/// <returns></returns>
		static bool CanProcess() { return enableProcessing; }

		/// <summary>
		/// Temporarily locks processing for all functions
		static  /// </summary>
			bool
			LockProcessing()
		{
			bool val = Main::enableProcessing;
			Main::enableProcessing = false;
			return val;
		}

		/// <summary>
		/// Unlocks processing for all functions
		/// </summary>
		static void UnlockProcessing()
		{
			enableProcessing = true;
		}

		//----------------------Support----------------------------

		/// <summary>
		/// Returns whether the actor is dead, or the TESDeathEvent has already been fired for the npc
		/// </summary>
		/// <param name="actor"></param>
		/// <returns></returns>
		static bool IsDead(RE::Actor* actor);

		/// <summary>
		/// Returns whether the TESDeathEvent has already been fired for the npc
		/// </summary>
		/// <param name="actor"></param>
		/// <returns></returns>
		static bool IsDeadEventFired(RE::Actor* actor);

		/// <summary>
		/// Sets that the TESDeathEvent has been fired for the npc
		/// </summary>
		/// <param name="actor"></param>
		/// <returns></returns>
		static void SetDead(RE::ActorHandle actor);

		/// <summary>
		/// initializes important variables, which need to be initialized every time a game is loaded
		/// </summary>
		static void InitializeCompatibilityObjects();

		/// <summary>
		/// Returns whether the player character has died
		/// </summary>
		/// <returns></returns>
		static bool IsPlayerDead() { return playerdied; }
		/// <summary>
		/// Sets the live status of the player character
		/// </summary>
		/// <param name="died"></param>
		static void PlayerDied(bool died) { playerdied = died; }

		//-------------------GameFunctions-------------------------

		/// <summary>
		/// Callback on loading a save game, initializes actor processing
		/// </summary>
		/// <param name=""></param>
		static void LoadGameCallback(SKSE::SerializationInterface* a_intfc);

		/// <summary>
		/// Callback on reverting the game. Disables processing and stops all handlers
		/// </summary>
		/// <param name=""></param>
		static void RevertGameCallback(SKSE::SerializationInterface* a_intfc);
		/// <summary>
		/// Callback on saving the game
		/// </summary>
		/// <param name=""></param>
		static void SaveGameCallback(SKSE::SerializationInterface* a_intfc);

		/// <summary>
		/// Saves the list of already dead actors
		/// </summary>
		/// <param name="a_intfc"></param>
		static long SaveDeadActors(SKSE::SerializationInterface* a_intfc);

		/// <summary>
		/// Read a dead actor record
		/// </summary>
		/// <param name="a_intfc"></param>
		/// <param name="length"></param>
		/// <returns></returns>
		static long ReadDeadActors(SKSE::SerializationInterface* a_intfc, uint32_t length);

		static inline uint64_t frameTimeLast;
		static inline uint64_t frameTime;
		static inline bool paused = false;
		static void OnFrame();
		static inline std::list<std::shared_ptr<AnimationInfo>> _eventQueue;
		static void AddEvent(std::shared_ptr<AnimationInfo> info)
		{
			_eventQueue.push_back(info);
		}
		/// <summary>
		/// game ui
		/// </summary>
		static inline RE::UI* ui;
		/// <summary>
		/// Game audiomanager which plays sounds.
		/// </summary>
		static inline RE::BSAudioManager* audiomanager;

		static void TrySetHotkey(uint32_t key);

		static RE::BSSoundHandle PlaySound(RE::BGSSoundDescriptorForm* soundDesc, std::shared_ptr<ActorInfo> acinfo);
	};

	using EventResult = RE::BSEventNotifyControl;

    class EventHandler :
		public RE::BSTEventSink<RE::TESHitEvent>,
		public RE::BSTEventSink<RE::TESCombatEvent>,
		public RE::BSTEventSink<RE::TESDeathEvent>,
		public RE::BSTEventSink<RE::TESEquipEvent>,
		public RE::BSTEventSink<RE::TESContainerChangedEvent>,
		public RE::BSTEventSink<RE::TESFastTravelEndEvent>,
		public RE::BSTEventSink<RE::TESActivateEvent>,
		public RE::BSTEventSink<RE::InputEvent*>
	{
	public:
		std::unordered_map<RE::FormID, std::chrono::steady_clock::time_point> time_map;

		/// <summary>
		/// returns singleton to the EventHandler
		/// </summary>
		/// <returns></returns>
		static EventHandler* GetSingleton();
		/// <summary>
		/// Registers us for all Events we want to receive
		/// </summary>
		static void Register();

		/// <summary>
		/// Processes TESHitEvents
		/// </summary>
		/// <param name=""></param>
		/// <param name=""></param>
		/// <returns></returns>
		virtual EventResult ProcessEvent(const RE::TESHitEvent* a_event, RE::BSTEventSource<RE::TESHitEvent>* a_eventSource) override;
		/// <summary>
		/// handles TESCombatEvent
		/// registers the actor for tracking and handles giving them potions, poisons and food, beforehand.
		/// also makes then eat food before the fight.
		/// </summary>
		/// <param name="a_event">event parameters like the actor we need to handle</param>
		/// <param name=""></param>
		/// <returns></returns>
		virtual EventResult ProcessEvent(const RE::TESCombatEvent* a_event, RE::BSTEventSource<RE::TESCombatEvent>* a_eventSource) override;  /// <summary>
		/// EventHandler for TESDeathEvent
		/// removed unused potions and poisons from actor, to avoid economy instability
		/// only registered if itemremoval is activated in the settings
		/// </summary>
		/// <param name="a_event"></param>
		/// <param name="a_eventSource"></param>
		/// <returns></returns>
		virtual EventResult ProcessEvent(const RE::TESDeathEvent* a_event, RE::BSTEventSource<RE::TESDeathEvent>* a_eventSource) override;
		/// <summary>
		/// EventHandler for Debug purposes. It calculates the distribution rules for all npcs in the cell
		/// </summary>
		/// <param name="a_event"></param>
		/// <param name="a_eventSource"></param>
		/// <returns></returns>
		virtual EventResult ProcessEvent(const RE::TESEquipEvent* a_event, RE::BSTEventSource<RE::TESEquipEvent>* a_eventSource) override;
		/// <summary>
		/// EventHandler for catching remove item events
		/// </summary>
		/// <param name="a_event"></param>
		/// <param name="a_eventSource"></param>
		/// <returns></returns>
		virtual EventResult ProcessEvent(const RE::TESContainerChangedEvent* a_event, RE::BSTEventSource<RE::TESContainerChangedEvent>* a_eventSource) override;
		/// <summary>
		/// EventHandler for end of fast travel
		/// </summary>
		/// <param name="a_event"></param>
		/// <param name="a_eventSource"></param>
		/// <returns></returns>
		virtual EventResult ProcessEvent(const RE::TESFastTravelEndEvent* a_event, RE::BSTEventSource<RE::TESFastTravelEndEvent>* a_eventSource) override;
		/// <summary>
		/// EventHandler for activate events
		/// </summary>
		/// <param name="a_event"></param>
		/// <param name="a_eventSource"></param>
		/// <returns></returns>
		virtual EventResult ProcessEvent(const RE::TESActivateEvent* a_event, RE::BSTEventSource<RE::TESActivateEvent>* a_eventSource) override;
		virtual EventResult ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>* a_eventSource) override;


		/// <summary>
		/// Handles an item being removed from a container
		/// </summary>
		/// <param name="container">The container the item was removed from</param>
		/// <param name="baseObj">The base object that has been removed</param>
		/// <param name="count">The number of items that have been removed</param>
		/// <param name="destinationContainer">The container the items have been moved to</param>
		/// <param name="a_event">The event information</param>
		void OnItemRemoved(RE::TESObjectREFR* container, RE::TESBoundObject* baseObj, int count, RE::TESObjectREFR* destinationContainer, const RE::TESContainerChangedEvent* a_event);

		/// <summary>
		/// Handles an item being added to a container
		/// </summary>
		/// <param name="container">The container the item is added to</param>
		/// <param name="baseObj">The base object that has been added</param>
		/// <param name="count">The number of items added</param>
		/// <param name="sourceContainer">The container the item was in before</param>
		/// <param name="a_event">The event information</param>
		void OnItemAdded(RE::TESObjectREFR* container, RE::TESBoundObject* baseObj, int count, RE::TESObjectREFR* sourceContainer, const RE::TESContainerChangedEvent* a_event);

	private:
		EventHandler() = default;
		EventHandler(const EventHandler&) = delete;
		EventHandler(EventHandler&&) = delete;
		virtual ~EventHandler() = default;

		EventHandler& operator=(const EventHandler&) = delete;
		EventHandler& operator=(EventHandler&&) = delete;
	};

	/// <summary>
	/// Registers all EventHandlers, if we would have multiple
	/// </summary>
	void RegisterAllEventHandlers();
}
