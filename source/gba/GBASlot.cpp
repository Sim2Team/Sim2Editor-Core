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

#include "GBASlot.hpp"
#include "../shared/Checksum.hpp"
#include "../shared/SAVUtils.hpp"

/* Get and Set Time. */
uint16_t GBASlot::Time() const { return GBASAVUtils::Read<uint16_t>(this->Offs + 0x2); };
void GBASlot::Time(const uint16_t V) { GBASAVUtils::Write<uint16_t>(this->Offs + 0x2, V); };

/* Get and Set Simoleons. */
uint32_t GBASlot::Simoleons() const { return GBASAVUtils::Read<uint32_t>(this->Offs + 0x5) >> 8; };
void GBASlot::Simoleons(uint32_t V) { GBASAVUtils::Write<uint32_t>(this->Offs + 0x5, (std::min<uint32_t>(999999, V) << 8)); };

/* Get and Set Name. */
std::string GBASlot::Name() const { return SAVUtils::ReadString(GBASAVUtils::SAV->GetData(), this->Offs + 0xD, 0x8); };
void GBASlot::Name(const std::string &V) { SAVUtils::WriteString(GBASAVUtils::SAV->GetData(), this->Offs + 0xD, 0x8, V); };

/* Get and Set Ratings. */
uint16_t GBASlot::Ratings() const { return GBASAVUtils::Read<uint16_t>(this->Offs + 0xA); };
void GBASlot::Ratings(const uint16_t V) { GBASAVUtils::Write<uint16_t>(this->Offs + 0xA, std::min<uint16_t>(9999, V)); };

/* Get and Set Empty Chug-Chug Cola Cans Amount. */
uint8_t GBASlot::Cans() const { return GBASAVUtils::Read<uint8_t>(this->Offset(0xF6, 0xFC, 0x102)); };
void GBASlot::Cans(const uint8_t V) { GBASAVUtils::Write<uint8_t>(this->Offset(0xF6, 0xFC, 0x102), std::min<uint8_t>(250, V)); };
/* Get and Set Empty Chug-Chug Cola Cans Sell price. */
uint8_t GBASlot::CansPrice() const { return GBASAVUtils::Read<uint8_t>(this->Offset(0xFA, 0x100, 0x106)); };
void GBASlot::CansPrice(const uint8_t V) { GBASAVUtils::Write<uint8_t>(this->Offset(0xFA, 0x100, 0x106), V); };

/* Get and Set Cowbells Amount. */
uint8_t GBASlot::Cowbells() const { return GBASAVUtils::Read<uint8_t>(this->Offset(0xF7, 0xFD, 0x103)); };
void GBASlot::Cowbells(const uint8_t V) { GBASAVUtils::Write<uint8_t>(this->Offset(0xF7, 0xFD, 0x103), std::min<uint8_t>(250, V)); };
/* Get and Set the Cowbells Sell price. */
uint8_t GBASlot::CowbellsPrice() const { return GBASAVUtils::Read<uint8_t>(this->Offset(0xFB, 0x101, 0x107)); };
void GBASlot::CowbellsPrice(const uint8_t V) { GBASAVUtils::Write<uint8_t>(this->Offset(0xFB, 0x101, 0x107), V); };

/* Get and Set Alien Spaceship Parts Amount. */
uint8_t GBASlot::Spaceship() const { return GBASAVUtils::Read<uint8_t>(this->Offset(0xF8, 0xFE, 0x104)); };
void GBASlot::Spaceship(const uint8_t V) { GBASAVUtils::Write<uint8_t>(this->Offset(0xF8, 0xFE, 0x104), std::min<uint8_t>(250, V)); };
/* Get and Set Alien Spaceship Parts Sell price. */
uint8_t GBASlot::SpaceshipPrice() const { return GBASAVUtils::Read<uint8_t>(this->Offset(0xFC, 0x102, 0x108)); };
void GBASlot::SpaceshipPrice(const uint8_t V) { GBASAVUtils::Write<uint8_t>(this->Offset(0xFC, 0x102, 0x108), V); };

/* Get and Set Nuclear Fuelrods Amount. */
uint8_t GBASlot::Fuelrods() const { return GBASAVUtils::Read<uint8_t>(this->Offset(0xF9, 0xFF, 0x105)); };
void GBASlot::Fuelrods(const uint8_t V) { GBASAVUtils::Write<uint8_t>(this->Offset(0xF9, 0xFF, 0x105), std::min<uint8_t>(250, V)); };
/* Get and Set Nuclear Fuelrods Sell price. */
uint8_t GBASlot::FuelrodsPrice() const { return GBASAVUtils::Read<uint8_t>(this->Offset(0xFD, 0x103, 0x109)); };
void GBASlot::FuelrodsPrice(const uint8_t V) { GBASAVUtils::Write<uint8_t>(this->Offset(0xFD, 0x103, 0x109), V); };

/* Get the Current Episode you are in. */
uint8_t GBASlot::CurrentEpisode() const {
	for (uint8_t Idx = 0; Idx < 12; Idx++) {
		if (GBASAVUtils::Read<uint8_t>(this->Offset(0x1A3, 0x1A9, 0x1AF)) == this->EPVals[Idx]) return Idx;
	}

	return 12; // 12 -> "Unofficial Episode". ;P
};

/*
	Set the Current Episode.

	const uint8_t V: The Episode.
	const bool ValidCheck: If checking for official Episodes (valid) or not. It is recommended to have this to TRUE.
*/
void GBASlot::CurrentEpisode(const uint8_t V, const bool ValidCheck) {
	if (!ValidCheck) { // In case we're not checking for validateness, Set it without checks.
		GBASAVUtils::Write<uint8_t>(this->Offset(0x1A3, 0x1A9, 0x1AF), V);
		GBASAVUtils::Write<uint8_t>(this->Offs + 0x9, V); // It's better to set that to 0x9 as well for display.
		return;
	}

	for (uint8_t Idx = 0; Idx < 12; Idx++) {
		if (V == this->EPVals[Idx]) {
			GBASAVUtils::Write<uint8_t>(this->Offset(0x1A3, 0x1A9, 0x1AF), V);
			GBASAVUtils::Write<uint8_t>(this->Offs + 0x9, V); // It's better to set that to 0x9 as well for display.
			break;
		}
	}
};

/* Get an Episode class. */
std::unique_ptr<GBAEpisode> GBASlot::Episode(const uint8_t EP) const { return std::make_unique<GBAEpisode>(this->Slot, EP, this->Move); };

/* Get a Cast class. */
std::unique_ptr<GBACast> GBASlot::Cast(const uint8_t CST) const {
	return std::make_unique<GBACast>(this->Offset(0x466, 0x46C, 0x472) + (std::min<uint8_t>(25, CST)) * 0xA, CST);
};

/* Get a Social Move class. */
std::unique_ptr<GBASocialMove> GBASlot::SocialMove(const uint8_t Move) const {
	return std::make_unique<GBASocialMove>(this->Offset(0x0, 0x3F0, 0x3F6) + (std::min<uint8_t>(14, Move)) * 0x8, Move);
};

/*
	Fix the Checksum of the current Slot, if invalid.

	Returns false if Slot < 0 or > 4 or already valid, true if got fixed.
*/
bool GBASlot::FixChecksum() {
	if (this->Slot < 1 || this->Slot > 4) return false;

	if (!Checksum::GBASlotChecksumValid(GBASAVUtils::SAV->GetData(), this->Slot, GBASAVUtils::Read<uint16_t>((this->Slot * 0x1000) + 0xFFE))) {
		GBASAVUtils::Write<uint16_t>((this->Slot * 0x1000) + 0xFFE, Checksum::CalcGBASlot(GBASAVUtils::SAV->GetData(), this->Slot));
		return true;
	}

	return false;
};