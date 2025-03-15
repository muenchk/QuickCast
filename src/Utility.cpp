#include "Utility.h"
#include "Compatibility.h"
#include "Logging.h"


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
struct window
{
	std::string sliding;

	int32_t size = 0;
	int32_t maxsize = 0;

	char AddChar(char c)
	{
		char ret = 0;
		if (size == maxsize)
		{
			ret = sliding[0];
			sliding = sliding.substr(1, sliding.length() - 1);
			sliding += c;
		}
		else
		{
			sliding += c;
			size++;
		}
		return ret;
	}
	void Reset()
	{
		sliding = "";
		size = 0;
	}
	std::string GetWindow()
	{
		return sliding;
	}
	int32_t Compare(std::string& other)
	{
		return sliding.compare(other);
	}
};


std::vector<std::string> Utility::SplitString(std::string str, char delimiter, bool removeEmpty, bool escape, char escapesymbol, bool allowdisableescape, char disablechar)
{
	std::vector<std::string> splits;
	if (escape) {
		std::string tmp;
		bool escaped = false;
		char last = 0;
		for (char c : str) {
			if (escaped == false) {
				if (c == delimiter) {
					splits.push_back(tmp);
					tmp = "";
				} else if (c == escapesymbol && (allowdisableescape == false || allowdisableescape && last != disablechar)) {
					escaped = !escaped;
					tmp += c;
				} else
					tmp += c;
			} else {
				if (c == escapesymbol && (allowdisableescape == false || allowdisableescape && last != disablechar))
				{
					escaped = !escaped;
					tmp += c;
				} else
					tmp += c;
			}
			last = c;
		}
		if (tmp.empty() == false || last == delimiter && escaped == false)
			splits.push_back(tmp);
	} else {
		size_t pos = str.find(delimiter);
		while (pos != std::string::npos) {
			splits.push_back(str.substr(0, pos));
			str.erase(0, pos + 1);
			pos = str.find(delimiter);
		}
		splits.push_back(str);
	}
	if (removeEmpty) {
		auto itr = splits.begin();
		while (itr != splits.end()) {
			if (*itr == "") {
				itr = splits.erase(itr);
				continue;
			}
			itr++;
		}
	}
	return splits;
}

std::vector<std::string> Utility::SplitString(std::string str, std::string delimiter, bool removeEmpty, bool escape, char escapesymbol)
{
	std::vector<std::string> splits;
	if (escape) {
		bool escaped = false;
		std::string tmp;
		window slide;
		slide.maxsize = (int)delimiter.size();
		for (char c : str) {
			//logdebug("Char: {}\t Window: {}\t Wsize: {}\t Wmaxsize: {}\t tmp: {}", c, slide.GetWindow(), slide.size, slide.maxsize, tmp);
			if (escape == true && escaped == true) {
				// escaped sequence.
				// just add char to string since we don't use window during escaped sequences
				tmp += c;
				if (c == escapesymbol) {
					escaped = !escaped;
				}
			} else {
				char x = slide.AddChar(c);
				if (x != 0)
					tmp += x;
				if (escape == true && c == escapesymbol) {
					// we are at the beginning of an escaped sequence
					escaped = !escaped;
					// add window to string since we don't use window during escaped sequence
					tmp += slide.GetWindow();
					slide.Reset();
				} else {
					if (slide.Compare(delimiter) == 0) {
						// delimiter matches sliding window
						splits.push_back(tmp);
						tmp = "";
						slide.Reset();
					}
				}
			}
		}
		// assemble last complete string part
		tmp += slide.GetWindow();
		// add as last split
		splits.push_back(tmp);
	}

	/* if (escape) {
		std::string tmp;
		bool escaped = false;
		char llast = 0;
		char last = 0;
		for (char c : str) {
			llast = last;
			last = c;
			if (escaped == false) {
				if ((std::string("") + last) + c == delimiter) {
					splits.push_back(tmp.substr(0, tmp.size() - 2));
					tmp = "";
					last = 0;
				} else if (c == escapesymbol) {
					escaped = !escaped;
					tmp += c;
				} else
					tmp += c;
			} else {
				if (c == escapesymbol) {
					escaped = !escaped;
					tmp += c;
				} else
					tmp += c;
			}
		}
		if (tmp.empty() == false || (std::string("") + llast) + last == delimiter && escaped == false)
			splits.push_back(tmp);
	} */
	else {
		size_t pos = str.find(delimiter);
		while (pos != std::string::npos) {
			splits.push_back(str.substr(0, pos));
			str.erase(0, pos + delimiter.length());
			pos = str.find(delimiter);
		}
		splits.push_back(str);
	}
	if (removeEmpty) {
		auto itr = splits.begin();
		while (itr != splits.end()) {
			if (*itr == "") {
				itr = splits.erase(itr);
				continue;
			}
			itr++;
		}
	}
	return splits;
}

std::string& Utility::RemoveWhiteSpaces(std::string& str, char escape, bool removetab, bool allowdisableescape, char disablechar)
{
	bool escaped = false;
	auto itr = str.begin();
	char last = 0;
	while (itr != str.end()) {
		if (*itr == escape && (allowdisableescape == false || allowdisableescape && last != disablechar))
			escaped = !escaped;
		last = *itr;
		if (!escaped) {
			//char c = *_itr;
			if (*itr == ' ' || (removetab && *itr == '\t')) {
				itr = str.erase(itr);
				continue;
			}
		}
		itr++;
	}
	return str;
}

std::string& Utility::RemoveSymbols(std::string& str, char symbol, bool enableescape, char escape)
{
	bool escaped = false;
	auto itr = str.begin();
	while (itr != str.end()) {
		if (enableescape && *itr == escape)
			escaped = !escaped;
		if (!escaped) {
			char c = *itr;
			if (*itr == symbol) {
				itr = str.erase(itr);
				continue;
			}
		}
		itr++;
	}
	return str;
}

std::string& Utility::RemoveSymbols(std::string& str, char symbol, char disablechar)
{
	auto itr = str.begin();
	char last = 0;
	while (itr != str.end()) {
		if (*itr == symbol && last != disablechar) {
			itr = str.erase(itr);
			continue;
		}
		last = *itr;
		itr++;
	}
	return str;
}

std::vector<std::pair<char, int32_t>> Utility::GetSymbols(std::string str)
{
	std::vector<std::pair<char, int32_t>> result;
	for (char c : str)
	{
		for (size_t i = 0; i < result.size(); i++)
		{
			if (result[i].first == c) {
				result[i] = { c, result[i].second + 1 };
				continue;
			}
		} 
		result.push_back({ c, 1 });
	}
	return result;
}

std::string Utility::ToStringCombatStyle(uint32_t style)
{
	std::string flags = "|";
	if (style & static_cast<int>(CurrentCombatStyle::Spellsword))
		flags += "Spellsword|";
	if (style & static_cast<int>(CurrentCombatStyle::OneHandedShield))
		flags += "OneHandedShield|";
	if (style & static_cast<int>(CurrentCombatStyle::TwoHanded))
		flags += "TwoHanded|";
	if (style & static_cast<int>(CurrentCombatStyle::OneHanded))
		flags += "OneHanded|";
	if (style & static_cast<int>(CurrentCombatStyle::Ranged))
		flags += "Ranged|";
	if (style & static_cast<int>(CurrentCombatStyle::DualWield))
		flags += "DualWield|";
	if (style & static_cast<int>(CurrentCombatStyle::HandToHand))
		flags += "HandToHand|";
	if (style & static_cast<int>(CurrentCombatStyle::Staffsword))
		flags += "Staffsword|";
	if (style & static_cast<int>(CurrentCombatStyle::DualStaff))
		flags += "DualStaff|";
	if (style & static_cast<int>(CurrentCombatStyle::Staff))
		flags += "Staff|";
	if (style & static_cast<int>(CurrentCombatStyle::Mage))
		flags += "Mage|";
	if (style & static_cast<int>(CurrentCombatStyle::MagicDestruction))
		flags += "MagicDestruction|";
	if (style & static_cast<int>(CurrentCombatStyle::MagicConjuration))
		flags += "MagicConjuration|";
	if (style & static_cast<int>(CurrentCombatStyle::MagicAlteration))
		flags += "MagicAlteration|";
	if (style & static_cast<int>(CurrentCombatStyle::MagicIllusion))
		flags += "MagicIllusion|";
	if (style & static_cast<int>(CurrentCombatStyle::MagicRestoration))
		flags += "MagicRestoration|";
	if (style & static_cast<int>(CurrentCombatStyle::MagicDamageFire))
		flags += "MagicDamageFire|";
	if (style & static_cast<int>(CurrentCombatStyle::MagicDamageShock))
		flags += "MagicDamageShock|";
	if (style & static_cast<int>(CurrentCombatStyle::MagicDamageFrost))
		flags += "MagicDamageFrost|";
	return flags;
}

uint32_t Utility::GetCombatData(RE::Actor* actor)
{
	if (actor == nullptr)
		return 0;
	loginfo("");
	uint32_t combatdata = 0;

	RE::TESForm* lefthand = actor->GetEquippedObject(true);
	RE::TESForm* righthand = actor->GetEquippedObject(false);
	RE::TESObjectWEAP* leftweap = lefthand != nullptr ? lefthand->As<RE::TESObjectWEAP>() : nullptr;
	RE::TESObjectWEAP* rightweap = righthand != nullptr ? righthand->As<RE::TESObjectWEAP>() : nullptr;
	RE::SpellItem* leftspell = lefthand != nullptr ? lefthand->As<RE::SpellItem>() : nullptr;
	RE::SpellItem* rightspell = righthand != nullptr ? righthand->As<RE::SpellItem>() : nullptr;
	if (rightweap && (rightweap->GetWeaponType() == RE::WEAPON_TYPE::kTwoHandAxe || rightweap->GetWeaponType() == RE::WEAPON_TYPE::kTwoHandSword)) {
		// twohanded
		combatdata |= static_cast<uint32_t>(CurrentCombatStyle::TwoHanded);
	} else if (rightweap && (rightweap->GetWeaponType() == RE::WEAPON_TYPE::kBow || rightweap->GetWeaponType() == RE::WEAPON_TYPE::kCrossbow)) {
		// ranged
		combatdata |= static_cast<uint32_t>(CurrentCombatStyle::Ranged);
	} else if (rightweap && leftweap) {
		if ((rightweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandSword ||
				rightweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandDagger ||
				rightweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandAxe ||
				rightweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandMace) &&
			(leftweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandSword ||
				leftweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandDagger ||
				leftweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandAxe ||
				leftweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandMace)) {
			// dual wield
			combatdata |= static_cast<uint32_t>(CurrentCombatStyle::DualWield);
		} else if ((rightweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandSword ||
					   rightweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandDagger ||
					   rightweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandAxe ||
					   rightweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandMace ||
					   rightweap->GetWeaponType() == RE::WEAPON_TYPE::kHandToHandMelee) &&
					   leftweap->GetWeaponType() == RE::WEAPON_TYPE::kStaff ||
				   (leftweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandSword ||
					   leftweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandDagger ||
					   leftweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandAxe ||
					   leftweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandMace ||
					   leftweap->GetWeaponType() == RE::WEAPON_TYPE::kHandToHandMelee) &&
					   rightweap->GetWeaponType() == RE::WEAPON_TYPE::kStaff) {
			// spellstaff
			if (rightweap->GetWeaponType() == RE::WEAPON_TYPE::kStaff) {
				// right staff
				if (rightweap->amountofEnchantment > 0) {
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::Staffsword);
					RE::EnchantmentItem* ench = rightweap->formEnchanting;
					if (ench) {
						for (uint32_t i = 0; i < ench->effects.size(); i++) {
							try {
								switch (ench->effects[i]->baseEffect->data.associatedSkill) {
								case RE::ActorValue::kAlteration:
									combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicAlteration);
									break;
								case RE::ActorValue::kDestruction:
									combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDestruction);
									if (ench->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageFrost" }))
										combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageFrost);
									if (ench->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageFire" }))
										combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageFire);
									if (ench->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageShock" }))
										combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageShock);
									break;
								case RE::ActorValue::kConjuration:
									combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicConjuration);
									break;
								case RE::ActorValue::kIllusion:
									combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicIllusion);
									break;
								case RE::ActorValue::kRestoration:
									combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicRestoration);
									break;
								}
							} catch (std::exception&) {
							}
						}
					}
				} else {
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::OneHanded);
				}

			} else {
				// left staff
				if (leftweap->amountofEnchantment > 0) {
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::Staffsword);
					RE::EnchantmentItem* ench = leftweap->formEnchanting;
					if (ench) {
						for (uint32_t i = 0; i < ench->effects.size(); i++) {
							try {
								switch (ench->effects[i]->baseEffect->data.associatedSkill) {
								case RE::ActorValue::kAlteration:
									combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicAlteration);
									break;
								case RE::ActorValue::kDestruction:
									combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDestruction);
									if (ench->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageFrost" }))
										combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageFrost);
									if (ench->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageFire" }))
										combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageFire);
									if (ench->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageShock" }))
										combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageShock);
									break;
								case RE::ActorValue::kConjuration:
									combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicConjuration);
									break;
								case RE::ActorValue::kIllusion:
									combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicIllusion);
									break;
								case RE::ActorValue::kRestoration:
									combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicRestoration);
									break;
								}
							} catch (std::exception&) {
							}
						}
					}
				} else {
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::OneHanded);
				}
				if (rightweap->GetWeaponType() == RE::WEAPON_TYPE::kHandToHandMelee ||
					leftweap->GetWeaponType() == RE::WEAPON_TYPE::kHandToHandMelee) {
					// if one hand is not a sword, but a hand to hand weapon fix that shit
					if (combatdata & static_cast<uint32_t>(CurrentCombatStyle::OneHanded))
						combatdata = (combatdata & (0xffffffff ^ static_cast<uint32_t>(CurrentCombatStyle::OneHanded))) | static_cast<uint32_t>(CurrentCombatStyle::Staff);
					else if (combatdata & static_cast<uint32_t>(CurrentCombatStyle::Staffsword))
						combatdata = (combatdata & (0xffffffff ^ static_cast<uint32_t>(CurrentCombatStyle::Staffsword))) | static_cast<uint32_t>(CurrentCombatStyle::Staff);
				}
			}
		} else if (rightweap->GetWeaponType() == RE::WEAPON_TYPE::kStaff &&
				   leftweap->GetWeaponType() == RE::WEAPON_TYPE::kStaff) {
			combatdata |= static_cast<uint32_t>(CurrentCombatStyle::DualStaff);
			if (leftweap->amountofEnchantment > 0) {
				RE::EnchantmentItem* ench = leftweap->formEnchanting;
				if (ench) {
					for (uint32_t i = 0; i < ench->effects.size(); i++) {
						try {
							switch (ench->effects[i]->baseEffect->data.associatedSkill) {
							case RE::ActorValue::kAlteration:
								combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicAlteration);
								break;
							case RE::ActorValue::kDestruction:
								combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDestruction);
								if (ench->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageFrost" }))
									combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageFrost);
								if (ench->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageFire" }))
									combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageFire);
								if (ench->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageShock" }))
									combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageShock);
								break;
							case RE::ActorValue::kConjuration:
								combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicConjuration);
								break;
							case RE::ActorValue::kIllusion:
								combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicIllusion);
								break;
							case RE::ActorValue::kRestoration:
								combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicRestoration);
								break;
							}
						} catch (std::exception&) {
						}
					}
				}
			}
			if (rightweap->amountofEnchantment > 0) {
				RE::EnchantmentItem* ench = rightweap->formEnchanting;
				if (ench) {
					for (uint32_t i = 0; i < ench->effects.size(); i++) {
						try {
							switch (ench->effects[i]->baseEffect->data.associatedSkill) {
							case RE::ActorValue::kAlteration:
								combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicAlteration);
								break;
							case RE::ActorValue::kDestruction:
								combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDestruction);
								if (ench->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageFrost" }))
									combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageFrost);
								if (ench->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageFire" }))
									combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageFire);
								if (ench->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageShock" }))
									combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageShock);
								break;
							case RE::ActorValue::kConjuration:
								combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicConjuration);
								break;
							case RE::ActorValue::kIllusion:
								combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicIllusion);
								break;
							case RE::ActorValue::kRestoration:
								combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicRestoration);
								break;
							}
						} catch (std::exception&) {
						}
					}
				}
			}
		} else if (leftweap->GetWeaponType() == RE::WEAPON_TYPE::kHandToHandMelee &&
				   rightweap->GetWeaponType() == RE::WEAPON_TYPE::kHandToHandMelee) {
			// fix for weapons that use hand to hand animations
			combatdata |= static_cast<uint32_t>(CurrentCombatStyle::HandToHand);
		} else if ((rightweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandSword ||
					   rightweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandDagger ||
					   rightweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandAxe ||
					   rightweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandMace) &&
					   leftweap->GetWeaponType() == RE::WEAPON_TYPE::kHandToHandMelee ||
				   (leftweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandSword ||
					   leftweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandDagger ||
					   leftweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandAxe ||
					   leftweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandMace) &&
					   rightweap->GetWeaponType() == RE::WEAPON_TYPE::kHandToHandMelee) {
			combatdata |= static_cast<uint32_t>(CurrentCombatStyle::OneHanded);
		}
	} else if (rightweap && leftspell &&
			   (rightweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandSword ||
				   rightweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandDagger ||
				   rightweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandAxe ||
				   rightweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandMace)) {
		// spellsrod
		combatdata |= static_cast<uint32_t>(CurrentCombatStyle::Spellsword);
		for (uint32_t i = 0; i < leftspell->effects.size(); i++) {
			try {
				switch (leftspell->effects[i]->baseEffect->data.associatedSkill) {
				case RE::ActorValue::kAlteration:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicAlteration);
					break;
				case RE::ActorValue::kDestruction:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDestruction);
					if (leftspell->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageFrost" }))
						combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageFrost);
					if (leftspell->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageFire" }))
						combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageFire);
					if (leftspell->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageShock" }))
						combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageShock);
					break;
				case RE::ActorValue::kConjuration:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicConjuration);
					break;
				case RE::ActorValue::kIllusion:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicIllusion);
					break;
				case RE::ActorValue::kRestoration:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicRestoration);
					break;
				}
			} catch (std::exception&) {
			}
		}
	} else if (leftweap && rightspell &&
			   (leftweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandSword ||
				   leftweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandDagger ||
				   leftweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandAxe ||
				   leftweap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandMace)) {
		// spellsword
		combatdata |= static_cast<uint32_t>(CurrentCombatStyle::Spellsword);
		for (uint32_t i = 0; i < rightspell->effects.size(); i++) {
			try {
				switch (rightspell->effects[i]->baseEffect->data.associatedSkill) {
				case RE::ActorValue::kAlteration:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicAlteration);
					break;
				case RE::ActorValue::kDestruction:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDestruction);
					if (rightspell->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageFrost" }))
						combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageFrost);
					if (rightspell->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageFire" }))
						combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageFire);
					if (rightspell->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageShock" }))
						combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageShock);
					break;
				case RE::ActorValue::kConjuration:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicConjuration);
					break;
				case RE::ActorValue::kIllusion:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicIllusion);
					break;
				case RE::ActorValue::kRestoration:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicRestoration);
					break;
				}
			} catch (std::exception&) {
			}
		}
	} else if (rightweap && lefthand && lefthand->As<RE::TESObjectARMO>()) {
		if (rightweap->GetWeaponType() == RE::WEAPON_TYPE::kHandToHandMelee)
			// fix for weapons that use hand to hand animations
			combatdata |= static_cast<uint32_t>(CurrentCombatStyle::HandToHand);
		else
			combatdata |= static_cast<uint32_t>(CurrentCombatStyle::OneHandedShield);
	} else if (leftspell && rightspell) {
		for (uint32_t i = 0; i < rightspell->effects.size(); i++) {
			try {
				switch (rightspell->effects[i]->baseEffect->data.associatedSkill) {
				case RE::ActorValue::kAlteration:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicAlteration);
					break;
				case RE::ActorValue::kDestruction:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDestruction);
					if (rightspell->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageFrost" }))
						combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageFrost);
					if (rightspell->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageFire" }))
						combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageFire);
					if (rightspell->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageShock" }))
						combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageShock);
					break;
				case RE::ActorValue::kConjuration:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicConjuration);
					break;
				case RE::ActorValue::kIllusion:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicIllusion);
					break;
				case RE::ActorValue::kRestoration:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicRestoration);
					break;
				}
			} catch (std::exception&) {
			}
		}
		for (uint32_t i = 0; i < leftspell->effects.size(); i++) {
			try {
				switch (leftspell->effects[i]->baseEffect->data.associatedSkill) {
				case RE::ActorValue::kAlteration:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicAlteration);
					break;
				case RE::ActorValue::kDestruction:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDestruction);
					if (leftspell->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageFrost" }))
						combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageFrost);
					if (leftspell->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageFire" }))
						combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageFire);
					if (leftspell->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageShock" }))
						combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageShock);
					break;
				case RE::ActorValue::kConjuration:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicConjuration);
					break;
				case RE::ActorValue::kIllusion:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicIllusion);
					break;
				case RE::ActorValue::kRestoration:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicRestoration);
					break;
				}
			} catch (std::exception&) {
			}
		}
		combatdata |= static_cast<uint32_t>(CurrentCombatStyle::Mage);
	} else if (leftspell) {
		for (uint32_t i = 0; i < leftspell->effects.size(); i++) {
			try {
				switch (leftspell->effects[i]->baseEffect->data.associatedSkill) {
				case RE::ActorValue::kAlteration:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicAlteration);
					break;
				case RE::ActorValue::kDestruction:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDestruction);
					if (leftspell->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageFrost" }))
						combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageFrost);
					if (leftspell->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageFire" }))
						combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageFire);
					if (leftspell->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageShock" }))
						combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageShock);
					break;
				case RE::ActorValue::kConjuration:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicConjuration);
					break;
				case RE::ActorValue::kIllusion:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicIllusion);
					break;
				case RE::ActorValue::kRestoration:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicRestoration);
					break;
				}
			} catch (std::exception&) {
			}
		}
		combatdata |= static_cast<uint32_t>(CurrentCombatStyle::Mage);
	} else if (rightspell) {
		for (uint32_t i = 0; i < rightspell->effects.size(); i++) {
			try {
				switch (rightspell->effects[i]->baseEffect->data.associatedSkill) {
				case RE::ActorValue::kAlteration:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicAlteration);
					break;
				case RE::ActorValue::kDestruction:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDestruction);
					if (rightspell->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageFrost" }))
						combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageFrost);
					if (rightspell->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageFire" }))
						combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageFire);
					if (rightspell->effects[i]->baseEffect->HasKeyword(std::string_view{ "MagicDamageShock" }))
						combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicDamageShock);
					break;
				case RE::ActorValue::kConjuration:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicConjuration);
					break;
				case RE::ActorValue::kIllusion:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicIllusion);
					break;
				case RE::ActorValue::kRestoration:
					combatdata |= static_cast<uint32_t>(CurrentCombatStyle::MagicRestoration);
					break;
				}
			} catch (std::exception&) {
			}
		}
		combatdata |= static_cast<uint32_t>(CurrentCombatStyle::Mage);
	} else if (leftweap || rightweap) {
		if (leftweap && leftweap->GetWeaponType() == RE::WEAPON_TYPE::kStaff ||
			rightweap && rightweap->GetWeaponType() == RE::WEAPON_TYPE::kStaff)
			combatdata |= static_cast<uint32_t>(CurrentCombatStyle::Mage);
		else if (leftweap && leftweap->GetWeaponType() == RE::WEAPON_TYPE::kHandToHandMelee ||
				 rightweap && rightweap->GetWeaponType() == RE::WEAPON_TYPE::kHandToHandMelee)
			// fix for weapons that use hand to hand animations
			combatdata |= static_cast<uint32_t>(CurrentCombatStyle::HandToHand);
		else
			combatdata |= static_cast<uint32_t>(CurrentCombatStyle::OneHanded);
	} else {
		combatdata |= static_cast<uint32_t>(CurrentCombatStyle::HandToHand);
	}

	return combatdata;
}

int Utility::GetItemType(RE::TESForm* form)
{
	if (form == nullptr)
		return 0; // HandtoHand
	RE::TESObjectWEAP* weap = form->As<RE::TESObjectWEAP>();
	if (weap) {
		if (weap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandSword)
			return 1;
		else if (weap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandDagger)
			return 2;
		else if (weap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandAxe)
			return 3;
		else if (weap->GetWeaponType() == RE::WEAPON_TYPE::kOneHandMace)
			return 4;
		else if (weap->GetWeaponType() == RE::WEAPON_TYPE::kTwoHandSword)
			return 5;
		else if (weap->GetWeaponType() == RE::WEAPON_TYPE::kTwoHandAxe)
			return 6;
		else if (weap->GetWeaponType() == RE::WEAPON_TYPE::kBow)
			return 7;
		else if (weap->GetWeaponType() == RE::WEAPON_TYPE::kStaff)
			return 8;
		else if (weap->GetWeaponType() == RE::WEAPON_TYPE::kCrossbow)
			return 12;
		else if (weap->GetWeaponType() == RE::WEAPON_TYPE::kHandToHandMelee)
			return 0;
	}
	RE::TESObjectARMO* armo = form->As<RE::TESObjectARMO>();
	if (armo)
		return 10;
	RE::SpellItem* spell = form->As<RE::SpellItem>();
	if (spell)
		return 9; // magic spell

	RE::TESObjectMISC* misc = form->As<RE::TESObjectMISC>();
	if (misc && misc->GetFormID() == 0x0001D4EC)
		return 11;

	return 0; // fallback
}

uint32_t Utility::GetArmorData(RE::Actor* actor)
{
	if (actor == nullptr)
		return 0;
	loginfo("");
	static std::vector<RE::BGSBipedObjectForm::BipedObjectSlot> armorSlots{
		RE::BGSBipedObjectForm::BipedObjectSlot::kHead,
		RE::BGSBipedObjectForm::BipedObjectSlot::kBody,
		RE::BGSBipedObjectForm::BipedObjectSlot::kHands,
		RE::BGSBipedObjectForm::BipedObjectSlot::kForearms,
		RE::BGSBipedObjectForm::BipedObjectSlot::kFeet,
		RE::BGSBipedObjectForm::BipedObjectSlot::kCalves,
		RE::BGSBipedObjectForm::BipedObjectSlot::kShield,
		RE::BGSBipedObjectForm::BipedObjectSlot::kModChestPrimary,
		RE::BGSBipedObjectForm::BipedObjectSlot::kModBack,
		RE::BGSBipedObjectForm::BipedObjectSlot::kModPelvisPrimary,
		RE::BGSBipedObjectForm::BipedObjectSlot::kModPelvisSecondary,
		RE::BGSBipedObjectForm::BipedObjectSlot::kModLegRight,
		RE::BGSBipedObjectForm::BipedObjectSlot::kModLegLeft,
		RE::BGSBipedObjectForm::BipedObjectSlot::kModChestSecondary,
		RE::BGSBipedObjectForm::BipedObjectSlot::kModShoulder,
		RE::BGSBipedObjectForm::BipedObjectSlot::kModArmLeft,
		RE::BGSBipedObjectForm::BipedObjectSlot::kModArmRight
	};

	std::set<RE::FormID> visited{};
	unsigned char armorHeavy = 0;
	unsigned char armorLight = 0;
	unsigned char clothing = 0;
	RE::TESObjectARMO* item = nullptr;
	for (int i = 0; i < armorSlots.size(); i++) {
		item = actor->GetWornArmor(armorSlots[i]);
		if (item) {
			if (visited.contains(item->GetFormID()))
				continue;
			visited.insert(item->GetFormID());
			for (uint32_t c = 0; c < item->numKeywords; c++) {
				if (item->keywords[c]) {
					if (item->keywords[c]->GetFormID() == 0x6BBE8) {  // ArmorClothing
						clothing++;
						// every item may only be either clothing, light or heavy armor
						continue;
					} else if (item->keywords[c]->GetFormID() == 0x6BBD2) {  // ArmorHeavy
						armorHeavy++;
						continue;
					} else if (item->keywords[c]->GetFormID() == 0x6BBD3) {  // ArmorLight
						armorLight++;
						continue;
					}
				}
			}
		}
	}
	// we finished every word item in the possible armor slots.
	// armor is recognised as worn, if two or more pieces of the same type are worn.
	uint32_t ret = 0;  // also CurrentArmor::Nothing in case nothing below fires
	if (armorHeavy >= 2)
		ret |= static_cast<uint32_t>(CurrentArmor::HeavyArmor);
	if (armorLight >= 2)
		ret |= static_cast<uint32_t>(CurrentArmor::LightArmor);
	if (clothing >= 2)
		ret |= static_cast<uint32_t>(CurrentArmor::Clothing);
	return ret;
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

std::vector<int> Utility::ParseIntArray(std::string line)
{
	std::vector<int> ret;
	size_t pos = line.find(';');
	while (pos != std::string::npos) {
		try {
			ret.push_back(std::stoi(line.substr(0, pos)));
		} catch (std::out_of_range&) {
			return std::vector<int>();
		} catch (std::invalid_argument&) {
			return std::vector<int>();
		}
		line.erase(0, pos + 1);
		pos = line.find(';');
	}
	if (line.length() != 0) {
		try {
			ret.push_back(std::stoi(line));
		} catch (std::out_of_range&) {
			return std::vector<int>();
		} catch (std::invalid_argument&) {
			return std::vector<int>();
		}
	}
	return ret;
}

RE::TESForm* Utility::GetTESForm(RE::TESDataHandler* datahandler, RE::FormID formid, std::string pluginname, std::string editorid)
{
	RE::TESForm* tmp = nullptr;
	if (pluginname.size() != 0) {
		if (formid != 0) {
			tmp = datahandler->LookupForm(formid, std::string_view{ pluginname });
			if (tmp == nullptr && editorid.size() > 0) {
				tmp = RE::TESForm::LookupByEditorID(std::string_view{ editorid });
			}
		} else if (editorid.size() > 0) {
			tmp = RE::TESForm::LookupByEditorID(std::string_view{ editorid });
		}
		// else we cannot find what we were lookin for
	} else if (formid > 0) {
		// pluginname is not given, so try to find the form by the id itself
		tmp = RE::TESForm::LookupByID(formid);
	} else if (editorid.size() > 0) {
		tmp = RE::TESForm::LookupByEditorID(std::string_view{ editorid });
	}
	return tmp;
}

std::string Utility::Mods::GetPluginName(RE::TESForm* form)
{
	return Utility::Mods::GetPluginNameFromID(form->GetFormID());
}

std::string Utility::Mods::GetPluginNameFromID(RE::FormID formid)
{
	if ((formid >> 24) == 0xFF)
		return "";
	if ((formid >> 24) != 0xFE) {
		auto itr = Settings::pluginIndexMap.find(formid & 0xFF000000);
		if (itr != Settings::pluginIndexMap.end())
			return itr->second;
		return "";
	}
	if ((formid >> 24) == 0x00)
		return "Skyrim.esm";
	// light mod
	auto itr = Settings::pluginIndexMap.find(formid & 0xFFFFF000);
	if (itr != Settings::pluginIndexMap.end())
		return itr->second;
	return "";
}

std::string Utility::Mods::GetPluginName(uint32_t pluginIndex)
{
	auto itr = Settings::pluginIndexMap.find(pluginIndex);
	if (itr != Settings::pluginIndexMap.end())
		return itr->second;
	else
		return "";
}

uint32_t Utility::Mods::GetPluginIndex(std::string pluginname)
{
	auto itr = Settings::pluginNameMap.find(pluginname);
	if (itr != Settings::pluginNameMap.end()) {
		return itr->second;
	} else
		return 0x1;
}

uint32_t Utility::Mods::GetPluginIndex(RE::TESForm* form)
{
	return GetPluginIndex(GetPluginName(form));
}

uint32_t Utility::Mods::GetIndexLessFormID(RE::TESForm* form)
{
	if (form == nullptr)
		return 0;
	if ((form->GetFormID() & 0xFF000000) == 0xFF000000) {
		// temporary id, save whole id
		return form->GetFormID();
	} else if ((form->GetFormID() & 0xFF000000) == 0xFE000000) {
		// only save index in light plugin
		return form->GetFormID() & 0x00000FFF;
	} else {
		// save index in normal plugin
		return form->GetFormID() & 0x00FFFFFF;
	}
}
uint32_t Utility::Mods::GetIndexLessFormID(RE::FormID formid)
{
	if ((formid & 0xFF000000) == 0xFF000000) {
		// temporary id, save whole id
		return formid;
	} else if ((formid & 0xFF000000) == 0xFE000000) {
		// only save index in light plugin
		return formid & 0x00000FFF;
	} else {
		// save index in normal plugin
		return formid & 0x00FFFFFF;
	}
}

bool Utility::VerifyActorInfo(std::shared_ptr<ActorInfo> const& acinfo)
{
	if (acinfo->IsValid() == false || acinfo->GetActor() == nullptr || acinfo->GetActor()->GetFormID() == 0) {
		loginfo("actor info damaged");
		return false;
	}
	return true;
}

bool Utility::ValidateActor(RE::Actor* actor)
{
	if (actor == nullptr || (actor->formFlags & RE::TESForm::RecordFlags::kDeleted) || (actor->inGameFormFlags & RE::TESForm::InGameFormFlag::kRefPermanentlyDeleted) || (actor->inGameFormFlags & RE::TESForm::InGameFormFlag::kWantsDelete) || actor->GetFormID() == 0 || (actor->formFlags & RE::TESForm::RecordFlags::kDisabled))
		return false; 

	return true;
}

Misc::NPCTPLTInfo Utility::ExtractTemplateInfo(RE::TESLevCharacter* lvl)
{
	if (lvl == nullptr)
		return Misc::NPCTPLTInfo{};
	// just try to grab the first entry of the leveled list, since they should all share
	// factions 'n stuff
	if (lvl->entries.size() > 0) {
		uint32_t plugID = Utility::Mods::GetPluginIndex(lvl);
		RE::TESForm* entry = lvl->entries[0].form;
		RE::TESNPC* tplt = entry->As<RE::TESNPC>();
		RE::TESLevCharacter* lev = entry->As<RE::TESLevCharacter>();
		if (tplt)
			return [&tplt, &plugID, &lvl]() { auto info = ExtractTemplateInfo(tplt); if (plugID != 0x1) {info.pluginID = plugID;info.baselvl = lvl;} return info; }();

		else if (lev)
			return [&lev, &plugID, &lvl]() { auto info = ExtractTemplateInfo(lev); if (plugID != 0x1) {info.pluginID = plugID;info.baselvl = lvl;} return info; }();
		else
			;  //loginfo("template invalid");
	}
	return Misc::NPCTPLTInfo{};
}

Misc::NPCTPLTInfo Utility::ExtractTemplateInfo(RE::TESNPC* npc)
{
	Misc::NPCTPLTInfo info;
	if (npc == nullptr)
		return info;
	if (npc->baseTemplateForm == nullptr) {
		// we are at the base, so do the main work
		info.tpltrace = npc->GetRace();
		info.tpltstyle = npc->combatStyle;
		info.tpltclass = npc->npcClass;
		for (uint32_t i = 0; i < npc->numKeywords; i++) {
			if (npc->keywords[i])
				info.tpltkeywords.push_back(npc->keywords[i]);
		}
		for (uint32_t i = 0; i < npc->factions.size(); i++) {
			if (npc->factions[i].faction)
				info.tpltfactions.push_back(npc->factions[i].faction);
		}

		uint32_t plugID = Utility::Mods::GetPluginIndex(npc);
		if (plugID != 0x1) {
			info.pluginID = plugID;
		}
		return info;
	}
	RE::TESNPC* tplt = npc->baseTemplateForm->As<RE::TESNPC>();
	RE::TESLevCharacter* lev = npc->baseTemplateForm->As<RE::TESLevCharacter>();
	Misc::NPCTPLTInfo tpltinfo;
	if (tplt) {
		// get info about template and then integrate into our local information according to what we use
		tpltinfo = ExtractTemplateInfo(tplt);
	} else if (lev) {
		tpltinfo = ExtractTemplateInfo(lev);
	} else {
		//loginfo("template invalid");
	}

	info.pluginID = tpltinfo.pluginID;

	uint32_t plugID = Utility::Mods::GetPluginIndex(npc);
	if (plugID != 0x1) {
		info.pluginID = plugID;
	}
	info.base = tpltinfo.base;
	info.baselvl = tpltinfo.baselvl;
	if ((npc->GetFormID() & 0xFF000000) != 0xFF000000) {
		// if pluginID not runtime, save the current actor as the base actor
		info.base = npc;
	}

	if (npc->actorData.templateUseFlags & RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kFactions) {
		info.tpltfactions = tpltinfo.tpltfactions;
	} else {
		for (uint32_t i = 0; i < npc->factions.size(); i++) {
			if (npc->factions[i].faction)
				info.tpltfactions.push_back(npc->factions[i].faction);
		}
	}
	if (npc->actorData.templateUseFlags & RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kKeywords) {
		info.tpltkeywords = tpltinfo.tpltkeywords;
	} else {
		for (uint32_t i = 0; i < npc->numKeywords; i++) {
			if (npc->keywords[i])
				info.tpltkeywords.push_back(npc->keywords[i]);
		}
	}
	if (npc->actorData.templateUseFlags & RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kTraits) {
		// race
		info.tpltrace = tpltinfo.tpltrace;
	} else {
		info.tpltrace = npc->GetRace();
	}
	if (npc->actorData.templateUseFlags & RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kStats) {
		// class
		info.tpltclass = tpltinfo.tpltclass;
	} else {
		info.tpltclass = npc->npcClass;
	}
	if (npc->actorData.templateUseFlags & RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAIData) {
		// combatstyle
		info.tpltstyle = tpltinfo.tpltstyle;
	} else {
		info.tpltstyle = npc->combatStyle;
	}
	return info;
}

Misc::NPCTPLTInfo Utility::ExtractTemplateInfo(RE::Actor* actor)
{
	if (actor != nullptr) {
		Misc::NPCTPLTInfo tpltinfo = ExtractTemplateInfo(actor->GetActorBase());
		uint32_t plugID = Utility::Mods::GetPluginIndex(actor);
		if (plugID != 0x1) {
			tpltinfo.pluginID = plugID;
		}
		return tpltinfo;
	}
	return Misc::NPCTPLTInfo{};
}
