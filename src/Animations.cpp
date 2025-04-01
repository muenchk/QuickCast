#include "Animations.h"
#include "Logging.h"
#include "AnimationEvents.h"
#include "SpellInfo.h"
#include "Settings.h"
#include "ActorInfo.h"
#include "Events.h"

void Animations::CastLeft_CastSpell(std::shared_ptr<AnimationInfo> event)
{
	if (event->acinfo->GetAV(RE::ActorValue::kMagicka) > event->spell->GetMagickaCost(event->acinfo)) {
		event->consumedMagicka = event->spell->GetMagickaCost(event->acinfo);
		event->acinfo->DamageActorValue(RE::ActorValue::kMagicka, event->consumedMagicka);

		event->acinfo->CastSpellImmediate(event->spell->GetSpell(), event->acinfo->GetActor(), RE::MagicSystem::CastingSource::kLeftHand);
		loginfo("{} CastSpellImmediate", Utility::GetHexFill(event->identifier));

		// reset consumed magicka, since the spell is cast
		event->consumedMagicka = 0;
	} else
		// abort if the actor doens't have enough magicka
		return;

	// for requipping last object
	if (event->equippedleft != nullptr) {
		event->callback = EquippedPostLeft;
		event->waitinganim = "MLh_Equipped_Event";
		PlayerAnimationHandler::GetSingleton()->AddEvent(event, Events::Main::frameTime + TimeMultMillis * 1000);
	} else if (event->equippedTwoHanded && event->equippedright) {
		event->callback = EquippedPostRight;
		event->waitinganim = "MLh_Equipped_Event";
		PlayerAnimationHandler::GetSingleton()->AddEvent(event, Events::Main::frameTime + TimeMultMillis * 1000);
	}
}
void Animations::CastRight_CastSpell(std::shared_ptr<AnimationInfo> event)
{
	if (event->acinfo->GetAV(RE::ActorValue::kMagicka) > event->spell->GetMagickaCost(event->acinfo)) {
		event->consumedMagicka = event->spell->GetMagickaCost(event->acinfo);
		event->acinfo->DamageActorValue(RE::ActorValue::kMagicka, event->consumedMagicka);

		event->acinfo->CastSpellImmediate(event->spell->GetSpell(), event->acinfo->GetActor(), RE::MagicSystem::CastingSource::kRightHand);
		loginfo("{} CastSpellImmediate", Utility::GetHexFill(event->identifier));

		// reset consumed magicka, since the spell is cast
		event->consumedMagicka = 0;
	} else
		// abort if the actor doens't have enough magicka
		return;

	// for reequipping last object
	if (event->equippedright != nullptr) {
		event->callback = EquippedPostRight;
		event->waitinganim = "MRh_Equipped_Event";
		PlayerAnimationHandler::GetSingleton()->AddEvent(event, Events::Main::frameTime + TimeMultMillis * 1000);
	}
}

void Animations::EquippedPreLeft(std::shared_ptr<AnimationInfo> event)
{
	auto left = event->acinfo->GetActor()->GetEquippedObject(true);
	if (left && left->GetFormID() == event->spell->GetSpell()->GetFormID())
	{
		Events::Main::AddEvent(event);
	} else {
		logwarn("equipped left-hand object is not the desired spell, aborting. desired: {}, actual: {}", Utility::PrintForm(left), Utility::PrintForm(event->spell->GetSpell()));
	}
}

void Animations::EquippedPreRight(std::shared_ptr<AnimationInfo> event)
{
	auto right = event->acinfo->GetActor()->GetEquippedObject(false);
	if (right && right->GetFormID() == event->spell->GetSpell()->GetFormID()) {
		Events::Main::AddEvent(event);
	} else {
		logwarn("equipped left-hand object is not the desired spell, aborting. desired: {}, actual: {}", Utility::PrintForm(right), Utility::PrintForm(event->spell->GetSpell()));
	}
}

void Animations::EquippedPostLeft(std::shared_ptr<AnimationInfo> event)
{
	loginfo("");

	if (event->sheathed)
	{
		if (auto act = event->acinfo->GetActor(); act != nullptr)
		{
			act->DrawWeaponMagicHands(false);
			event->sheathed = false;
		}
	}

	if (event->equippedleft) {
		if (auto spell = event->equippedleft->As<RE::SpellItem>(); spell != nullptr) {
			loginfo("{}", Utility::PrintForm(spell));
			event->acinfo->EquipSpell(spell, Settings::Equip_LeftHand);
		}
		else if (auto bound = event->equippedleft->As<RE::TESBoundObject>(); bound != nullptr) {
			loginfo("{}", Utility::PrintForm(bound));
			event->acinfo->EquipItem(true, bound);
		}
	}

	event->acinfo->ResetAnimationStatusLeft();
	event->Clear();
}
void Animations::EquippedPostRight(std::shared_ptr<AnimationInfo> event)
{
	loginfo("");

	if (event->sheathed) {
		if (auto act = event->acinfo->GetActor(); act != nullptr) {
			act->DrawWeaponMagicHands(false);
			event->sheathed = false;
		}
	}

	if (event->equippedright) {
		if (auto spell = event->equippedright->As<RE::SpellItem>(); spell != nullptr) {
			loginfo("{}", Utility::PrintForm(spell));
			event->acinfo->EquipSpell(spell, Settings::Equip_RightHand);
		} else if (auto bound = event->equippedright->As<RE::TESBoundObject>(); bound != nullptr) {
			loginfo("{}", Utility::PrintForm(bound));
			event->acinfo->EquipItem(false, bound);
		}
	}

	event->acinfo->ResetAnimationStatusRight();
	event->Clear();
}

void Animations::EquippedPostDual(std::shared_ptr<AnimationInfo> event)
{
	if (event->sheathed) {
		if (auto act = event->acinfo->GetActor(); act != nullptr) {
			act->DrawWeaponMagicHands(false);
			event->sheathed = false;
		}
	}

	event->acinfo->ResetAnimationStatusLeft();
	event->acinfo->ResetAnimationStatusRight();
	event->Clear();
}

/// <summary>
/// 
/// </summary>
/// <param name="event"></param>
/// <returns>whether the spell must be equipped</returns>
bool SpellHandlerLeft(std::shared_ptr<AnimationInfo> event)
{
	auto leftobject = event->acinfo->GetEquippedObject(true);
	loginfo("got left object");

	event->acinfo->UpdateWeaponsDrawn();
	if (event->acinfo->IsWeaponDrawn() == false)
	{
		if (auto act = event->acinfo->GetActor(); act != nullptr) {
			event->sheathed = true;
			act->DrawWeaponMagicHands(true);
		}
	}

	if (leftobject && leftobject->GetFormID() == event->spell->GetSpell()->GetFormID()) {
		// already equipped
		loginfo("Spell already equipped");
		return false;
	} else {
		loginfo("equip spell");
		// we need to equip the spell before we can continue
		// save the equipment in the left hand
		//auto left = event->acinfo->GetEquippedEntryData(true);
		auto rightobject = event->acinfo->GetEquippedObject(false);
		if (rightobject && rightobject->As<RE::SpellItem>() && rightobject->As<RE::SpellItem>()->GetEquipSlot() == Settings::Equip_BothHands) {
			// ritual spell equipped
			event->equippedTwoHanded = true;
			event->equippedright = rightobject;
		}
		auto right = event->acinfo->GetEquippedEntryData(false);
		if (event->equippedTwoHanded == false && right) {
			auto weap = right->object->As<RE::TESObjectWEAP>();
			if (weap) {
				// check weapon type to see if its
				switch (weap->GetWeaponType()) {
				case RE::WEAPON_TYPE::kTwoHandSword:
				case RE::WEAPON_TYPE::kTwoHandAxe:
				case RE::WEAPON_TYPE::kBow:
				case RE::WEAPON_TYPE::kCrossbow:
					event->equippedTwoHanded = true;
					event->equippedright = rightobject;
					break;
				default:
					break;
				}
			}
		}
		if (event->equippedTwoHanded == false && leftobject)
			event->equippedleft = leftobject;
		
		return true;
	}
}

bool SpellHandlerRight(std::shared_ptr<AnimationInfo> event)
{
	auto rightobject = event->acinfo->GetEquippedObject(false);
	loginfo("got right object");

	event->acinfo->UpdateWeaponsDrawn();
	if (event->acinfo->IsWeaponDrawn() == false) {
		if (auto act = event->acinfo->GetActor(); act != nullptr) {
			event->sheathed = true;
			act->DrawWeaponMagicHands(true);
		}
	}

	if (rightobject && rightobject->GetFormID() == event->spell->GetSpell()->GetFormID()) {
		// already equipped
		loginfo("Spell already equipped");
		return false;
	} else {
		loginfo("equip spell");
		event->equippedright = rightobject;
		return true;
	}
}

bool Animations::CastAimedLeft(std::shared_ptr<AnimationInfo> event)
{
	if (event->acinfo->GetAV(RE::ActorValue::kMagicka) > event->spell->GetMagickaCost(event->acinfo)) {
		switch (event->animstage) {
		case 0:
			if (SpellHandlerLeft(event)) {
				// equip spell
				event->acinfo->EquipSpell(event->spell->GetSpell(), Settings::Equip_LeftHand);
				// remove event from main handler and add it the the animation event handler
				// so we only continue after we have the spell equipped

				event->callback = EquippedPreLeft;
				//event->waitinganim = "Magic_Equip_Out";
				event->waitinganim = "EnableBumper";
				event->animstage++;
				PlayerAnimationHandler::GetSingleton()->AddEvent(event, Events::Main::frameTime + TimeMultMillis * 2000);

				return true;
			} else {
				event->animstage++;
			}
			break;
		case 1:
			// begin animation
			event->acinfo->NotifyAnimationGraph("BeginCastLeft");
			event->acinfo->NotifyAnimationGraph("MLh_SpellAimedStart");

			if (event->activeSound.IsValid() && event->activeSound.IsPlaying())
				event->activeSound.Stop();
			event->activeSound = Events::Main::PlaySound(event->spell->GetSound(RE::MagicSystem::SoundID::kCharge), event->acinfo);

			loginfo("{} MLh_SpellAimedStart", Utility::GetHexFill(event->identifier));
			event->animstage++;
			QueryUnbiasedInterruptTime(&event->ticks);
			event->ticks += TimeMultMillis * Settings::CastOptions::_castingScaling * event->spell->GetChargeTime();
			break;
		case 2:
			// spell ready animation
			event->acinfo->NotifyAnimationGraph("MLh_SpellReady_Event");

			if (event->activeSound.IsValid() && event->activeSound.IsPlaying())
				event->activeSound.Stop();
			event->activeSound = Events::Main::PlaySound(event->spell->GetSound(RE::MagicSystem::SoundID::kReadyLoop), event->acinfo);

			loginfo("{} MLh_SpellReady_Event", Utility::GetHexFill(event->identifier));
			event->animstage++;
			(&event->ticks);
			event->ticks += TimeMultMillis * Settings::CastOptions::_readyTime.count();
			break;
		case 3:
			// release spell
			event->acinfo->NotifyAnimationGraph("MLh_SpellRelease_Event");
			loginfo("{} MLh_SpellRelease_Event", Utility::GetHexFill(event->identifier));
			event->animstage++;

			if (event->activeSound.IsValid() && event->activeSound.IsPlaying())
				event->activeSound.Stop();
			event->activeSound = Events::Main::PlaySound(event->spell->GetSound(RE::MagicSystem::SoundID::kRelease), event->acinfo);

			// for actual spell cast
			event->callback = CastLeft_CastSpell;
			event->waitinganim = "MLh_SpellFire_Event";
			PlayerAnimationHandler::GetSingleton()->AddEvent(event, Events::Main::frameTime + TimeMultMillis * 1000);
			return true;
			break;
		}
		return false;
	} else {
		// if we have unequipped the weapons, change them back
		if (event->animstage > 0)
			CastLeft_CastSpell(event);
		return true;
	}
}

bool Animations::CastAimedRight(std::shared_ptr<AnimationInfo> event)
{
	if (event->acinfo->GetAV(RE::ActorValue::kMagicka) > event->spell->GetMagickaCost(event->acinfo)) {
		switch (event->animstage) {
		case 0:
			if (SpellHandlerRight(event)) {
				// equip spell
				event->acinfo->EquipSpell(event->spell->GetSpell(), Settings::Equip_RightHand);
				// remove event from main handler and add it the the animation event handler
				// so we only continue after we have the spell equipped

				event->callback = EquippedPreRight;
				//event->waitinganim = "Magic_Equip_Out";
				event->waitinganim = "EnableBumper";
				event->animstage++;
				PlayerAnimationHandler::GetSingleton()->AddEvent(event, Events::Main::frameTime + TimeMultMillis * 1000);

				return true;
			} else {
				event->animstage++;
			}
			break;
		case 1:
			event->acinfo->NotifyAnimationGraph("BeginCastRight");
			event->acinfo->NotifyAnimationGraph("MRh_SpellAimedStart");

			if (event->activeSound.IsValid() && event->activeSound.IsPlaying())
				event->activeSound.Stop();
			event->activeSound = Events::Main::PlaySound(event->spell->GetSound(RE::MagicSystem::SoundID::kCharge), event->acinfo);

			loginfo("{} MRh_SpellAimedStart", Utility::GetHexFill(event->identifier));
			event->animstage++;
			QueryUnbiasedInterruptTime(&event->ticks);
			event->ticks += TimeMultMillis * Settings::CastOptions::_castingScaling * event->spell->GetChargeTime();
			break;
		case 2:
			event->acinfo->NotifyAnimationGraph("MRh_SpellReady_Event");

			if (event->activeSound.IsValid() && event->activeSound.IsPlaying())
				event->activeSound.Stop();
			event->activeSound = Events::Main::PlaySound(event->spell->GetSound(RE::MagicSystem::SoundID::kReadyLoop), event->acinfo);

			loginfo("{} MRh_SpellReady_Event", Utility::GetHexFill(event->identifier));
			event->animstage++;
			(&event->ticks);
			event->ticks += TimeMultMillis * Settings::CastOptions::_readyTime.count();
			break;
		case 3:
			event->acinfo->NotifyAnimationGraph("MRh_SpellRelease_Event");

			if (event->activeSound.IsValid() && event->activeSound.IsPlaying())
				event->activeSound.Stop();
			event->activeSound = Events::Main::PlaySound(event->spell->GetSound(RE::MagicSystem::SoundID::kRelease), event->acinfo);

			loginfo("{} MRh_SpellRelease_Event", Utility::GetHexFill(event->identifier));
			event->animstage++;

			event->callback = CastRight_CastSpell;
			event->waitinganim = "MRh_SpellFire_Event";
			PlayerAnimationHandler::GetSingleton()->AddEvent(event, Events::Main::frameTime + TimeMultMillis * 1000);
			return true;
			break;
		}
		return false;
	} else {
		// if we have unequipped the weapons, change them back
		if (event->animstage > 0)
			CastRight_CastSpell(event);
		return true;
	}
}



bool Animations::CastSelfLeft(std::shared_ptr<AnimationInfo> event)
{
	if (event->acinfo->GetAV(RE::ActorValue::kMagicka) > event->spell->GetMagickaCost(event->acinfo)) {
		switch (event->animstage) {
		case 0:
			if (SpellHandlerLeft(event)) {
				// equip spell
				event->acinfo->EquipSpell(event->spell->GetSpell(), Settings::Equip_LeftHand);
				// remove event from main handler and add it the the animation event handler
				// so we only continue after we have the spell equipped

				event->callback = EquippedPreLeft;
				//event->waitinganim = "Magic_Equip_Out";
				event->waitinganim = "EnableBumper";
				event->animstage++;
				PlayerAnimationHandler::GetSingleton()->AddEvent(event, Events::Main::frameTime + TimeMultMillis * 1000);

				return true;
			} else {
				event->animstage++;
			}
			break;
		case 1:
			event->acinfo->NotifyAnimationGraph("BeginCastLeft");
			event->acinfo->NotifyAnimationGraph("MLh_SpellSelfStart");

			if (event->activeSound.IsValid() && event->activeSound.IsPlaying())
				event->activeSound.Stop();
			event->activeSound = Events::Main::PlaySound(event->spell->GetSound(RE::MagicSystem::SoundID::kCharge), event->acinfo);

			loginfo("{} MLh_SpellSelfStart", Utility::GetHexFill(event->identifier));
			event->animstage++;
			QueryUnbiasedInterruptTime(&event->ticks);
			event->ticks += TimeMultMillis * Settings::CastOptions::_castingScaling * event->spell->GetChargeTime();
			break;
		case 2:
			event->acinfo->NotifyAnimationGraph("MLh_SpellReady_Event");

			if (event->activeSound.IsValid() && event->activeSound.IsPlaying())
				event->activeSound.Stop();
			event->activeSound = Events::Main::PlaySound(event->spell->GetSound(RE::MagicSystem::SoundID::kReadyLoop), event->acinfo);

			loginfo("{} MLh_SpellReady_Event", Utility::GetHexFill(event->identifier));
			event->animstage++;
			(&event->ticks);
			event->ticks += TimeMultMillis * Settings::CastOptions::_readyTime.count();
			break;
		case 3:
			event->acinfo->NotifyAnimationGraph("MLh_SpellRelease_Event");

			if (event->activeSound.IsValid() && event->activeSound.IsPlaying())
				event->activeSound.Stop();
			event->activeSound = Events::Main::PlaySound(event->spell->GetSound(RE::MagicSystem::SoundID::kRelease), event->acinfo);

			loginfo("{} MLh_SpellRelease_Event", Utility::GetHexFill(event->identifier));
			event->animstage++;

			event->callback = CastLeft_CastSpell;
			event->waitinganim = "MLh_SpellFire_Event";
			PlayerAnimationHandler::GetSingleton()->AddEvent(event, Events::Main::frameTime + TimeMultMillis * 1000);
			return true;
			break;
		}
		return false;
	} else {
		// if we have unequipped the weapons, change them back
		if (event->animstage > 0)
			CastRight_CastSpell(event);
		return true;
	}
}
bool Animations::CastSelfRight(std::shared_ptr<AnimationInfo> event)
{
	if (event->acinfo->GetAV(RE::ActorValue::kMagicka) > event->spell->GetMagickaCost(event->acinfo)) {
		switch (event->animstage) {
		case 0:
			if (SpellHandlerRight(event)) {
				// equip spell
				event->acinfo->EquipSpell(event->spell->GetSpell(), Settings::Equip_RightHand);
				// remove event from main handler and add it the the animation event handler
				// so we only continue after we have the spell equipped

				event->callback = EquippedPreRight;
				//event->waitinganim = "Magic_Equip_Out";
				event->waitinganim = "EnableBumper";
				event->animstage++;
				PlayerAnimationHandler::GetSingleton()->AddEvent(event, Events::Main::frameTime + TimeMultMillis * 1000);

				return true;
			} else {
				event->animstage++;
			}
			break;
		case 1:
			event->acinfo->NotifyAnimationGraph("BeginCastRight");
			event->acinfo->NotifyAnimationGraph("MRh_SpellSelfStart");

			if (event->activeSound.IsValid() && event->activeSound.IsPlaying())
				event->activeSound.Stop();
			event->activeSound = Events::Main::PlaySound(event->spell->GetSound(RE::MagicSystem::SoundID::kCharge), event->acinfo);

			loginfo("{} MRh_SpellSelfStart", Utility::GetHexFill(event->identifier));
			event->animstage++;
			QueryUnbiasedInterruptTime(&event->ticks);
			event->ticks += TimeMultMillis * Settings::CastOptions::_castingScaling * event->spell->GetChargeTime();
			break;
		case 2:
			event->acinfo->NotifyAnimationGraph("MRh_SpellReady_Event");

			if (event->activeSound.IsValid() && event->activeSound.IsPlaying())
				event->activeSound.Stop();
			event->activeSound = Events::Main::PlaySound(event->spell->GetSound(RE::MagicSystem::SoundID::kReadyLoop), event->acinfo);

			loginfo("{} MRh_SpellReady_Event", Utility::GetHexFill(event->identifier));
			event->animstage++;
			(&event->ticks);
			event->ticks += TimeMultMillis * Settings::CastOptions::_readyTime.count();
			break;
		case 3:
			event->acinfo->NotifyAnimationGraph("MRh_SpellRelease_Event");

			if (event->activeSound.IsValid() && event->activeSound.IsPlaying())
				event->activeSound.Stop();
			event->activeSound = Events::Main::PlaySound(event->spell->GetSound(RE::MagicSystem::SoundID::kRelease), event->acinfo);

			loginfo("{} MRh_SpellRelease_Event", Utility::GetHexFill(event->identifier));
			event->animstage++;

			event->callback = CastRight_CastSpell;
			event->waitinganim = "MRh_SpellFire_Event";
			PlayerAnimationHandler::GetSingleton()->AddEvent(event, Events::Main::frameTime + TimeMultMillis * 1000);
			return true;
			break;
		}
		return false;
	} else {
		// if we have unequipped the weapons, change them back
		if (event->animstage > 0)
			CastRight_CastSpell(event);
		return true;
	}
}

bool Animations::CastConcAimedLeft(std::shared_ptr<AnimationInfo> event)
{
	return true;
}
bool Animations::CastConcAimedRight(std::shared_ptr<AnimationInfo> event)
{
	return true;
}

bool Animations::CastConcSelfLeft(std::shared_ptr<AnimationInfo> event)
{
	
	
	switch (event->animstage) {
	case 0:
		event->acinfo->CastSpellImmediate(event->spell->GetSpell(), event->acinfo->GetActor(), RE::MagicSystem::CastingSource::kLeftHand);
		event->animstage++;
		QueryUnbiasedInterruptTime(&event->ticks);
		event->ticks += TimeMultMillis * 4000;
		break;
	case 1:
		if (event->acinfo->GetActor()->IsCasting(event->spell->GetSpell()))
			event->acinfo->GetActor()->InterruptCast(false);
		return true;
		break;
	}
	return false;
}
bool Animations::CastConcSelfRight(std::shared_ptr<AnimationInfo> event)
{
	return true;
}

bool Animations::CastWardRight(std::shared_ptr<AnimationInfo> event)
{
	return true;
}
bool Animations::CastWardLeft(std::shared_ptr<AnimationInfo> event)
{
	return true;
}

bool Animations::DualCastAimed(std::shared_ptr<AnimationInfo> event)
{
	switch (event->animstage) {
	case 0:
		event->acinfo->NotifyAnimationGraph("BeginCast");
		event->acinfo->NotifyAnimationGraph("BeginCastLeft");
		event->acinfo->NotifyAnimationGraph("BeginCastRight");

		loginfo("{} BeginCastLeft", Utility::GetHexFill(event->identifier));
		event->animstage++;
		QueryUnbiasedInterruptTime(&event->ticks);
		event->ticks += TimeMultMillis * 50;
	case 1:
		event->acinfo->NotifyAnimationGraph("DualMagic_SpellAimedStart");
		loginfo("{} DualMagic_SpellAimedStart", Utility::GetHexFill(event->identifier));
		event->animstage++;
		QueryUnbiasedInterruptTime(&event->ticks);
		event->ticks += TimeMultMillis * Settings::CastOptions::_castingScaling * event->spell->GetChargeTime();
	case 2:
		event->acinfo->NotifyAnimationGraph("MLh_SpellReady_Event");
		loginfo("{} MLh_SpellReady_Event", Utility::GetHexFill(event->identifier));
		event->animstage++;
		(&event->ticks);
		event->ticks += TimeMultMillis * Settings::CastOptions::_readyTime.count();
		break;
	case 3:

		event->acinfo->NotifyAnimationGraph("MLh_SpellRelease_Event");
		loginfo("{} MLh_SpellRelease_Event", Utility::GetHexFill(event->identifier));
		event->acinfo->CastSpellImmediate(event->spell->GetSpell(), event->acinfo->GetActor(), RE::MagicSystem::CastingSource::kLeftHand);
		event->acinfo->ResetAnimationStatusLeft();
		event->acinfo->ResetAnimationStatusRight();

		event->Clear();
		return true;
		break;
	}
	return false;
}
bool Animations::DualCastSelf(std::shared_ptr<AnimationInfo> event)
{
	return true;
}
bool Animations::DualCastConcAimed(std::shared_ptr<AnimationInfo> event)
{
	return true;
}
bool Animations::DualCastConcSelf(std::shared_ptr<AnimationInfo> event)
{
	return true;
}
bool Animations::DualCastWard(std::shared_ptr<AnimationInfo> event)
{
	return true;
}

bool Animations::Shout(std::shared_ptr<AnimationInfo> event)
{
	switch (event->animstage) {
	case 0:
		event->acinfo->NotifyAnimationGraph("BeginCastVoice");
		event->acinfo->NotifyAnimationGraph("ShoutStart");
		event->animstage++;
		break;
	case 1:
		event->acinfo->NotifyAnimationGraph("shoutRelease");
		event->animstage++;
	case 2:
		event->acinfo->NotifyAnimationGraph("shoutStop");
		event->acinfo->ResetAnimationStatusLeft();
		event->acinfo->ResetAnimationStatusRight();

		return true;
		break;
	}
	return false;
}

bool Animations::Ritual(std::shared_ptr<AnimationInfo> event)
{
	return true;
}
