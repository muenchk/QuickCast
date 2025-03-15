#include "Game.h"
#include "Hooks.h"
#include "Events.h"
#include "Logging.h"

namespace Hooks
{

	uint64_t OnFrameHook::OnFrame(void* unk)
	{
		Events::Main::OnFrame();
		return _OnFrame(unk);
	}

	bool FastTravelConfirmHook::FastTravelConfirm(uint64_t self, uint64_t menu)
	{
		loginfo("Begin FastTravel");
		Game::SetFastTraveling(true);
		return _FastTravelConfirm(self, menu);
	}

	void Papyrus_FastTravelHook::FastTravelBegin()
	{
		loginfo("Begin Fast Travel");
		Game::SetFastTraveling(true);
	}

	void Papyrus_FastTravelHook::FastTravelEnd()
	{
		loginfo("End Fast Travel");
		Game::SetFastTraveling(false);
	}

	void FavoritesHandlerEx::InstallHook()
	{
		REL::Relocation<std::uintptr_t> vTable(RE::VTABLE_FavoritesHandler[0]);
		_ProcessButton = vTable.write_vfunc(0x5, &FavoritesHandlerEx::ProcessButton_Hook);
	}

	void IsMagicHotkey()
	{
		auto acinfo = Events::Main::data->FindActor(RE::PlayerCharacter::GetSingleton());
		auto inventory = acinfo->GetInventory();

		RE::MagicFavorites* magic = RE::MagicFavorites::GetSingleton();
		
		for (auto itr = magic->spells.begin(); itr != magic->spells.end(); itr++)
		{
			auto form = *itr;
			
		}
	}

	bool FavoritesHandlerEx::ProcessButton_Hook(RE::ButtonEvent* a_event)
	{
		//using EHKS::HotkeyManager;

		RE::UserEvents* userEvents = RE::UserEvents::GetSingleton();
		RE::UI* ui = RE::UI::GetSingleton();

		// Note: When used with Skyrim Souls RE, this input handler is already blocked
		if (ui->GameIsPaused() || ui->IsMenuOpen(RE::CraftingMenu::MENU_NAME)) {
			return false;
		}

		if (a_event->userEvent == userEvents->favorites) {
			if (!ui->IsMenuOpen(RE::FavoritesMenu::MENU_NAME)) {
				RE::UIMessageQueue* messageQueue = RE::UIMessageQueue::GetSingleton();
				messageQueue->AddMessage(RE::FavoritesMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, 0);
				return true;
			}
		}

		if (a_event->eventType == RE::INPUT_EVENT_TYPE::kButton) {
			loginfo("Button: 0x{}", Utility::GetHex(a_event->AsButtonEvent()->idCode));
			/*HotkeyManager* hotkeyManager = HotkeyManager::GetSingleton();
			Hotkey* hotkey = nullptr;

			if (a_event->device == RE::INPUT_DEVICE::kGamepad && !EHKS::IsVanillaHotkey(a_event->userEvent)) {
				return false;
			}

			bool isVampire = IsPlayerVampire();
			isVampire ? hotkey = hotkeyManager->GetVampireHotkey(a_event->device.get(), a_event->idCode) : hotkey = hotkeyManager->GetHotkey(a_event->device.get(), a_event->idCode);

			if (hotkey) {
				//Update hotkeys and check again
				hotkeyManager->UpdateHotkeys();

				if (isVampire ? hotkey = hotkeyManager->GetVampireHotkey(a_event->device.get(), a_event->idCode) : hotkey = hotkeyManager->GetHotkey(a_event->device.get(), a_event->idCode)) {
					if (hotkey->type == Hotkey::HotkeyType::kItem) {
						ItemHotkey* itemHotkey = static_cast<ItemHotkey*>(hotkey);
						RE::TESForm* baseForm = itemHotkey->GetBaseForm();
						if (baseForm) {
							RE::ExtraDataList* extraData = itemHotkey->GetExtraData();
							EquipItem(baseForm, extraData);
							return true;
						}
					} else {
						MagicHotkey* magicHotkey = static_cast<MagicHotkey*>(hotkey);
						EquipItem(magicHotkey->form, nullptr);
						return true;
					}
				}
			}*/
		}
		return false;
	}

	void EquipObjectOverRide::Install()
	{
		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(37938, 38894), REL::VariantOffset(0xE5, 0x170, 0xE5) };
		stl::write_thunk_call<EquipObjectOverRide>(target.address());
		loginfo("Ultimate Potion Animation NG - Hook Installed - (The stolen one)");
	}

	
	void EquipObjectOverRide::thunk(RE::ActorEquipManager* a_self, RE::Actor* a_actor, RE::TESBoundObject* a_object, std::uint64_t a_unk)
	{
		if (!Events::Main::CanProcess()) {
			// cant process right now
			func(a_self, a_actor, a_object, a_unk);
			return;
		}

		if (!a_object || !a_actor || !a_self || a_actor->IsPlayerRef()) {
			func(a_self, a_actor, a_object, a_unk);
			return;
		}
	}

	void UIHooks::InputFunc(RE::BSTEventSource<RE::InputEvent*>* a_dispatcher, RE::InputEvent* const* a_events)
	{
		static bool _AltHeld = false;
		static bool _CTRLHeld = false;

		std::vector<uint32_t> buttonsdown;

		if (a_events) {
			for (auto it = *a_events; it; it = it->next) {
				switch (it->GetEventType()) {
				case RE::INPUT_EVENT_TYPE::kButton:
					{
						const auto buttonEvent = static_cast<const RE::ButtonEvent*>(it);

						if (buttonEvent->GetIDCode() == 0x38) // Left ALT
						{
							_AltHeld = buttonEvent->Value() > 0.0f;
						}
						else if (buttonEvent->GetIDCode() == 0x1D)  // Left ALT
						{
							_CTRLHeld = buttonEvent->Value() > 0.0f;
						}
						else if (buttonEvent->value > 0.0f && buttonEvent->HeldDuration() == 0.0f) // Button down event
						{
							buttonsdown.push_back(buttonEvent->GetIDCode());
						}
						break;
					}
				}
			}
		}

		for (auto key : buttonsdown)
		{
			if (_CTRLHeld && key >= 0x2 && key <= 0x0B)  // 1 .. 0 in keys
			{
				loginfo("Pressed CTRL: {}, Key: {}", _CTRLHeld, key);
				Events::Main::TrySetHotkey(key);
			}
		}

		/*if (UI::UIManager::GetSingleton().ShouldConsumeInput()) {
			constexpr RE::InputEvent* const dummy[] = { nullptr };
			_InputFunc(a_dispatcher, dummy);
		} else {
			_InputFunc(a_dispatcher, a_events);
		}*/
		_InputFunc(a_dispatcher, a_events);
	}

	void InstallHooks()
	{
		FastTravelConfirmHook::InstallHook();
		Papyrus_FastTravelHook::InstallHook();
		OnFrameHook::Install();
		//FavoritesHandlerEx::InstallHook();
		UIHooks::Hook();
	}
}
