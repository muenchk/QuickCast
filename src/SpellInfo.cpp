#include "SpellInfo.h"
#include "Settings.h"
#include "ActorInfo.h"
#include "Logging.h"
#include "Events.h"
#include "Animations.h"

SpellInfo::SpellInfo(RE::SpellItem* spell)
{
	if (spell == nullptr) {
		_valid = false;
		return;
	}
	_spell = spell;
	_formid = spell->GetFormID();

	switch (spell->GetSpellType())
	{
	case RE::MagicSystem::SpellType::kSpell:
		_spellType = SpellType::kSpell;
		break;
	case RE::MagicSystem::SpellType::kPower:
		_spellType = SpellType::kPower;
		break;
	case RE::MagicSystem::SpellType::kLesserPower:
		_spellType = SpellType::kLesserPower;
		break;
	case RE::MagicSystem::SpellType::kVoicePower:
		_spellType = SpellType::kVoicePower;
		break;
	case RE::MagicSystem::SpellType::kScroll:
		_spellType = SpellType::kScroll;
		break;
	case RE::MagicSystem::SpellType::kDisease:
	case RE::MagicSystem::SpellType::kAbility:
	case RE::MagicSystem::SpellType::kPoison:
	case RE::MagicSystem::SpellType::kEnchantment:
	case RE::MagicSystem::SpellType::kPotion: // == Alchemy
	case RE::MagicSystem::SpellType::kWortCraft: // == Ingredient
	case RE::MagicSystem::SpellType::kLeveledSpell:
	case RE::MagicSystem::SpellType::kAddiction:
	case RE::MagicSystem::SpellType::kStaffEnchantment:
		_valid = false;
		return;
	}

	switch (spell->GetCastingType()) {
	case RE::MagicSystem::CastingType::kConstantEffect:
		_castingType = CastingType::kConstantEffect;
		break;
	case RE::MagicSystem::CastingType::kConcentration:
		_castingType = CastingType::kConcentration;
		break;
	case RE::MagicSystem::CastingType::kFireAndForget:
		_castingType = CastingType::kFireAndForget;
		break;
	case RE::MagicSystem::CastingType::kScroll:
		_castingType = CastingType::kScroll;
		break;
	}

	switch (spell->GetDelivery()) {
	case RE::MagicSystem::Delivery::kTouch:
	case RE::MagicSystem::Delivery::kTargetActor:
	case RE::MagicSystem::Delivery::kTargetLocation:
	case RE::MagicSystem::Delivery::kAimed:
		if (_castingType == CastingType::kConcentration)
			_deliveryType = DeliverooType::kConcAimed;
		else if (_castingType == CastingType::kFireAndForget)
		{
			_deliveryType = DeliverooType::kAimed;
		}
		else if (_castingType == CastingType::kScroll)
		{
			_deliveryType = DeliverooType::kAimed;
		}
		else if (_castingType == CastingType::kConstantEffect) {
			// doesn't happen
			_deliveryType = DeliverooType::kSelf;
			return;
		}
		break;
	case RE::MagicSystem::Delivery::kSelf:
		if (_castingType == CastingType::kConcentration) {
			// split for ward and non-ward
			if (Utility::HasArchetype(spell, RE::EffectArchetype::kAccumulateMagnitude, true, RE::ActorValue::kWardPower)) {
				_deliveryType = DeliverooType::kWard;
			} else {
				_deliveryType = DeliverooType::kConcSelf;
			}
		}
		else if (_castingType == CastingType::kFireAndForget) {
			_deliveryType = DeliverooType::kSelf;
		} else if (_castingType == CastingType::kScroll) {
			_deliveryType = DeliverooType::kSelf;
		} else if (_castingType == CastingType::kConstantEffect) {
			_deliveryType = DeliverooType::kSelf;
			return;
		}
		break;
	}

	_equipSlot = spell->GetEquipSlot();
	loginfo("{}", Utility::PrintForm(spell));
	loginfo("{}", Utility::PrintForm(_equipSlot));
	_chargeTime = (long long)(spell->GetChargeTime() * 1000);

	if (_equipSlot == Settings::Equip_BothHands)
		_isRitual = true;



	_valid = true;
}

RE::BGSSoundDescriptorForm* SpellInfo::GetSound(RE::MagicSystem::SoundID soundID)
{
	if (_valid) {
		if (_spell->effects.size() > 0) {
			RE::EffectSetting* sett = _spell->effects[0]->baseEffect;
			if (sett)
			{
				for (auto itr = sett->effectSounds.begin(); itr != sett->effectSounds.end(); itr++)
				{
					if (itr->id == soundID)
						return itr->sound;
				}
			}
		}
	}
	return nullptr;
}


float SpellInfo::GetMagickaCost(std::shared_ptr<ActorInfo> caster)
{
	if (_valid && caster->IsValid())
	{
		auto cost = _spell->CalculateMagickaCost(caster->GetActor());
		loginfo("SpellCost: {}", cost);
		return cost;
	}
	return 0;
}

void SpellInfo::DoCast(std::shared_ptr<ActorInfo> caster, CastingOrigin origin, std::shared_ptr<AnimationInfo> info)
{
	switch (origin) {
	case CastingOrigin::kLeftHand:
		if (info->GetInterrupt()) {
			loginfo("{} interrupted", Utility::GetHexFill(info->identifier));
			return;
		}  
		//caster->EquipSpell(_spell, Settings::Equip_LeftHand);
		std::this_thread::sleep_for(std::chrono::milliseconds(600));
		caster->GetActor()->SetGraphVariableBool("bWantCastLeft", true);
		SKSE::GetTaskInterface()->AddTask([caster]() {
			caster->GetActor()->NotifyAnimationGraph("MLh_SpellAimedStart");
		});
		loginfo("{} MLh_SpellAimedStart", Utility::GetHexFill(info->identifier));
		if (info->GetInterrupt()) {
			loginfo("{} interrupted", Utility::GetHexFill(info->identifier));
			return;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds((long long)(Settings::CastOptions::_castingScaling * _chargeTime)));
		SKSE::GetTaskInterface()->AddTask([caster]() { caster->GetActor()->NotifyAnimationGraph("MLh_SpellReady_Event"); });
		loginfo("{} MLh_SpellReady_Event", Utility::GetHexFill(info->identifier));
		if (info->GetInterrupt()) {
			loginfo("{} interrupted", Utility::GetHexFill(info->identifier));
			return;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(Settings::CastOptions::_readyTime));
		SKSE::GetTaskInterface()->AddTask([caster]() { caster->GetActor()->NotifyAnimationGraph("MLh_SpellRelease_Event"); });
		loginfo("{} MLh_SpellRelease_Event", Utility::GetHexFill(info->identifier));
		if (info->GetInterrupt()) {
			loginfo("{} interrupted", Utility::GetHexFill(info->identifier));
			return;
		}
		SKSE::GetTaskInterface()->AddTask([caster, spell = _spell]() {
			caster->GetActor()->GetMagicCaster(RE::MagicSystem::CastingSource::kLeftHand)->CastSpellImmediate(spell, false, caster->GetActor(), 100, false, false, caster->GetActor());
		});
		caster->ResetAnimationStatusLeft();
		break;
	case CastingOrigin::kRightHand:
		if (info->GetInterrupt()) {
			loginfo("{} interrupted", Utility::GetHexFill(info->identifier));
			return;
		}
		caster->GetActor()->SetGraphVariableBool("bWantCastRight", true);
		SKSE::GetTaskInterface()->AddTask([caster]() {
			caster->GetActor()->NotifyAnimationGraph("MRh_SpellAimedStart");
		});
		loginfo("{} MRh_SpellAimedStart", Utility::GetHexFill(info->identifier));
		if (info->GetInterrupt()) {
			loginfo("{} interrupted", Utility::GetHexFill(info->identifier));
			return;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds((long long)(Settings::CastOptions::_castingScaling * _chargeTime)));
		SKSE::GetTaskInterface()->AddTask([caster]() { caster->GetActor()->NotifyAnimationGraph("MRh_SpellReady_Event"); });
		loginfo("{} MRh_SpellReady_Event", Utility::GetHexFill(info->identifier));
		if (info->GetInterrupt()) {
			loginfo("{} interrupted", Utility::GetHexFill(info->identifier));
			return;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(Settings::CastOptions::_readyTime));
		SKSE::GetTaskInterface()->AddTask([caster]() { caster->GetActor()->NotifyAnimationGraph("MRh_SpellRelease_Event"); });
		loginfo("{} MRh_SpellRelease_Event", Utility::GetHexFill(info->identifier));
		if (info->GetInterrupt()) {
			loginfo("{} interrupted", Utility::GetHexFill(info->identifier));
			return;
		}
		SKSE::GetTaskInterface()->AddTask([caster, spell = _spell]() {
			caster->GetActor()->GetMagicCaster(RE::MagicSystem::CastingSource::kRightHand)->CastSpellImmediate(spell, false, caster->GetActor(), 100, false, false, caster->GetActor());
		});
		caster->ResetAnimationStatusRight();
		break;
	case CastingOrigin::kVoice:
		if (info->GetInterrupt()) {
			loginfo("{} interrupted", Utility::GetHexFill(info->identifier));
			return;
		}
		caster->GetActor()->SetGraphVariableBool("bWantCastVoice", true);
		SKSE::GetTaskInterface()->AddTask([caster]() {
			caster->GetActor()->NotifyAnimationGraph("shoutStart");
		});
		loginfo("{} shoutStart", Utility::GetHexFill(info->identifier));
		if (info->GetInterrupt()) {
			loginfo("{} interrupted", Utility::GetHexFill(info->identifier));
			return;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds((long long)(1000 /*Settings::CastOptions::_castingScaling * _chargeTime*/)));
		SKSE::GetTaskInterface()->AddTask([caster]() { caster->GetActor()->NotifyAnimationGraph("shoutRelease"); });
		loginfo("{} shoutRelease", Utility::GetHexFill(info->identifier));
		if (info->GetInterrupt()) {
			loginfo("{} interrupted", Utility::GetHexFill(info->identifier));
			return;
		}
		SKSE::GetTaskInterface()->AddTask([caster]() { caster->GetActor()->NotifyAnimationGraph("shoutStop"); });
		loginfo("{} shoutStop", Utility::GetHexFill(info->identifier));
		if (info->GetInterrupt()) {
			loginfo("{} interrupted", Utility::GetHexFill(info->identifier));
			return;
		}
		SKSE::GetTaskInterface()->AddTask([caster, spell = _spell]() {
			caster->GetActor()->GetMagicCaster(RE::MagicSystem::CastingSource::kOther)->CastSpellImmediate(spell, false, caster->GetActor(), 100, false, false, caster->GetActor());
		});
		caster->ResetAnimationStatusRight();
		caster->ResetAnimationStatusLeft();
	}
}

void SpellInfo::CastSpell(std::shared_ptr<SpellInfo> self, std::shared_ptr<ActorInfo> caster, CastingOrigin origin, bool dualCast)
{
	if (_valid && caster->IsValid()) {
		// sanity checks
		if (_equipSlot == Settings::Equip_Voice && origin != CastingOrigin::kVoice) {
			logwarn("trying to cast voice spell from another equipslot. {}", Utility::PrintForm(_equipSlot));
			return;
		}
		if ((origin == CastingOrigin::kLeftHand &&
				(_equipSlot == Settings::Equip_EitherHand ||
					_equipSlot == Settings::Equip_LeftHand)) ||
			origin != CastingOrigin::kLeftHand) {
		} else {
			logwarn("trying to cast a spell from the left hand that cannot be cast from the lefthand. {}", Utility::PrintForm(_equipSlot));
			return;
		}
		if (origin == CastingOrigin::kRightHand &&
				(_equipSlot == Settings::Equip_EitherHand ||
					_equipSlot == Settings::Equip_RightHand) ||
			origin != CastingOrigin::kRightHand) {
		} else {
			logwarn("trying to cast a spell from the right hand that cannot be cast from the right hand. {}", Utility::PrintForm(_equipSlot));
			return;
		}

		if (caster->GetAV(RE::ActorValue::kMagicka) < GetMagickaCost(caster)) {
			loginfo("Caster has too few magicka left");
			return;
		}
		RE::MagicCaster* magiccaster = nullptr;
		std::shared_ptr<AnimationInfo> info = std::make_shared<AnimationInfo>();
		switch (origin) {
		case CastingOrigin::kLeftHand:
			magiccaster = caster->GetActor()->GetMagicCaster(RE::MagicSystem::CastingSource::kLeftHand);
			if (caster->IsAnimationBusyLeft()) {
				caster->InterruptAnimationLeft();
				//caster->GetActor()->NotifyAnimationGraph("CastStop");
			}
			caster->SetAnimationBusyLeft(true);
			caster->SetAnimationLeft(info);
			break;
		case CastingOrigin::kRightHand:
			magiccaster = caster->GetActor()->GetMagicCaster(RE::MagicSystem::CastingSource::kRightHand);
			if (caster->IsAnimationBusyRight()) {
				caster->InterruptAnimationRight();
				//caster->GetActor()->NotifyAnimationGraph("CastStop");
			}
			caster->SetAnimationBusyRight(true);
			caster->SetAnimationRight(info);
			break;
		case CastingOrigin::kVoice:
			magiccaster = caster->GetActor()->GetMagicCaster(RE::MagicSystem::CastingSource::kOther);
			if (caster->IsAnimationBusyLeft() || caster->IsAnimationBusyRight()) {
				caster->InterruptAnimationLeft();
				caster->InterruptAnimationRight();
				//caster->GetActor()->NotifyAnimationGraph("CastStop");
			}
			caster->SetAnimationBusyLeft(true);
			caster->SetAnimationBusyRight(true);
			caster->SetAnimationLeft(info);
			caster->SetAnimationRight(info);
			break;
		}
		auto state = magiccaster->state;
		if (state & RE::MagicCaster::State::kCasting || state & RE::MagicCaster::State::kCharging) {
			loginfo("Currently casting");
			return;
		}

		//caster->GetActor()->GetMagicCaster(RE::MagicSystem::CastingSource::kLeftHand)->CastSpellImmediate(_spell, true, caster->GetActor(), 100, false, false, caster->GetActor());
		//caster->GetActor()->GetMagicCaster(RE::MagicSystem::CastingSource::kLeftHand)->SpellCast(true, 0, _spell);
		//caster->EquipSpell(_spell, Settings::Equip_LeftHand);
		/*magiccaster->SetCurrentSpell(_spell);
		magiccaster->SetDualCasting(dualCast);
		magiccaster->SetSkipCheckCast();
		magiccaster->SetCastingTimerForCharge();
		magiccaster->StartCastImpl();*/

		loginfo("sending animation event");
		/*if (Settings::CastOptions::_doCast) {
			std::thread(std::bind(&SpellInfo::DoCast, this, caster, origin, info)).detach();
		} else {
			magiccaster->CastSpellImmediate(_spell, false, caster->GetActor(), 100, false, false, caster->GetActor());
		}*/
		if (Settings::CastOptions::_doCast) {
			if (origin == CastingOrigin::kVoice) {
				//SetCastVars(caster, origin, info);
				//std::thread(std::bind(&SpellInfo::DoCast, this, caster, origin, info)).detach();
				info->acinfo = caster;
				info->spell = self;
				info->ticks = 0;
				info->animstage = 0;
				info->event = EventType::Shout;
				Events::Main::AddEvent(info);
			} else if (_isRitual) {
				// ritual spells have unique animations
				info->acinfo = caster;
				info->spell = self;
				info->ticks = 0;
				info->animstage = 0;
				info->event = EventType::Ritual;
				Events::Main::AddEvent(info);
			} else if (dualCast) {
				// dual cast has unique animation
				info->acinfo = caster;
				info->spell = self;
				info->ticks = 0;
				info->animstage = 0;
				switch (_deliveryType) {
				case DeliverooType::kAimed:
					info->event = EventType::DualCastAimed;
					break;
				case DeliverooType::kSelf:
					info->event = EventType::DualCastSelf;
					break;
				case DeliverooType::kWard:
					info->event = EventType::DualCastWard;
					break;
				case DeliverooType::kConcAimed:
					info->event = EventType::DualCastConcAimed;
					break;
				case DeliverooType::kConcSelf:
					info->event = EventType::DualCastConcSelf;
					break;
				}
				Events::Main::AddEvent(info);
			} else if (origin == CastingOrigin::kRightHand) {
				info->acinfo = caster;
				info->spell = self;
				info->ticks = 0;
				info->animstage = 0;
				switch (_deliveryType) {
				case DeliverooType::kAimed:
					info->event = EventType::CastAimedRight;
					break;
				case DeliverooType::kSelf:
					info->event = EventType::CastSelfRight;
					break;
				case DeliverooType::kWard:
					info->event = EventType::CastWardRight;
					break;
				case DeliverooType::kConcAimed:
					info->event = EventType::CastConcAimedRight;
					break;
				case DeliverooType::kConcSelf:
					info->event = EventType::CastConcAimedLeft;
					break;
				}
				Events::Main::AddEvent(info);
			} else {  // lefthand
				info->acinfo = caster;
				info->spell = self;
				info->ticks = 0;
				info->animstage = 0;
				switch (_deliveryType) {
				case DeliverooType::kAimed:
					info->event = EventType::CastAimedLeft;
					break;
				case DeliverooType::kSelf:
					info->event = EventType::CastSelfLeft;
					break;
				case DeliverooType::kWard:
					info->event = EventType::CastWardLeft;
					break;
				case DeliverooType::kConcAimed:
					info->event = EventType::CastConcAimedLeft;
					break;
				case DeliverooType::kConcSelf:
					info->event = EventType::CastConcSelfLeft;
					break;
				}
				Events::Main::AddEvent(info);
			}
			//caster->SetGraphVariableBool("bWantCastLeft", true);
			//caster->NotifyAnimationGraph("MLh_SpellSelfStart");
			//info->waitinganim = "EnableBumper";
			//info->callback = std::bind(&SpellInfo::DoCast, this, caster, origin, info);
			//info->callbackpre = std::bind(&SpellInfo::SetCastVars, this, caster, origin, info);
			//PlayerAnimationHandler::GetSingleton()->AddEvent(info);
		} else {
			magiccaster->CastSpellImmediate(_spell, false, caster->GetActor(), 100, false, false, caster->GetActor());
		}
	}
}
