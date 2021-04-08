/*
*   This file is part of Sim2Editor-CPPCore
*   Copyright (C) 2020-2021 SuperSaiyajinStackZ, Universal-Team
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*   Additional Terms 7.b and 7.c of GPLv3 apply to this file:
*       * Requiring preservation of specified reasonable legal notices or
*         author attributions in that material or in the Appropriate Legal
*         Notices displayed by works containing it.
*       * Prohibiting misrepresentation of the origin of that material,
*         or requiring that modified versions of such material be marked in
*         reasonable ways as different from the original version.
*/

#include "GBASettings.hpp"
#include "../shared/Checksum.hpp"
#include "../shared/SAVUtils.hpp"

/* Get and Set the Sound Effect Volume. */
uint8_t GBASettings::SFX() const { return GBASAVUtils::Read<uint8_t>(0x8); };
void GBASettings::SFX(const uint8_t V) {
	if (V > 10) return; // 0 - 10 only valid.
	GBASAVUtils::Write<uint8_t>(0x8, this->SFXLevels[V]);
};

/* Get and Set the Music Volume. */
uint8_t GBASettings::Music() const { return GBASAVUtils::Read<uint8_t>(0x9); };
void GBASettings::Music(const uint8_t V) {
	if (V > 10) return; // 0 - 10 only valid.
	GBASAVUtils::Write<uint8_t>(0x9, this->MusicLevels[V]);
};

/* Get and Set the Language. */
GBALanguage GBASettings::Language() const {
	if (GBASAVUtils::Read<uint8_t>(0xA) > 5) return GBALanguage::EN; // Technically, that would be a "blank" Language in game, but ehh that's not good.
	return (GBALanguage)GBASAVUtils::Read<uint8_t>(0xA);
};
void GBASettings::Language(const GBALanguage V) {
	GBASAVUtils::Write<uint8_t>(0xA, (uint8_t)V);
};

/* Update the Checksum of the GBA Settings. */
void GBASettings::UpdateChecksum() {
	const uint16_t CurCHKS = GBASAVUtils::Read<uint16_t>(0xE);
	const uint16_t Calced = Checksum::Calc(GBASAVUtils::SAV->GetData(), 0x0, (0x18 / 2), { (0xE / 2) });

	if (Calced != CurCHKS) { // If the calced result is NOT the current checksum.
		GBASAVUtils::Write<uint16_t>(0xE, Calced);
	}
};