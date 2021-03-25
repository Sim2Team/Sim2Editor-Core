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

#ifndef _SIM2EDITOR_CPP_CORE_GBA_SLOT_HPP
#define _SIM2EDITOR_CPP_CORE_GBA_SLOT_HPP

#include "CoreCommon.hpp"
#include "GBACast.hpp"
#include "GBAEpisode.hpp"
#include "GBASocialMove.hpp"

class GBASlot {
public:
	GBASlot(const uint8_t Slot, const uint8_t AddOffs = 0) : Move(AddOffs), Slot(Slot), Offs(Slot * 0x1000) { };

	uint16_t Time() const;
	void Time(const uint16_t V);

	uint32_t Simoleons() const;
	void Simoleons(const uint32_t V);

	std::string Name() const;
	void Name(const std::string &V);

	uint16_t Ratings() const;
	void Ratings(const uint16_t V);

	uint8_t Cans() const;
	void Cans(const uint8_t V);
	uint8_t CansPrice() const;
	void CansPrice(const uint8_t V);

    uint8_t Cowbells() const;
    void Cowbells(const uint8_t V);
    uint8_t CowbellsPrice() const;
    void CowbellsPrice(const uint8_t v);

    uint8_t Spaceship() const;
    void Spaceship(const uint8_t V);
    uint8_t SpaceshipPrice() const;
    void SpaceshipPrice(const uint8_t V);

    uint8_t Fuelrods() const;
    void Fuelrods(const uint8_t V);
    uint8_t FuelrodsPrice() const;
    void FuelrodsPrice(const uint8_t V);

	uint8_t CurrentEpisode() const;
	void CurrentEpisode(const uint8_t V, const bool ValidCheck = true);

	std::unique_ptr<GBAEpisode> Episode(const uint8_t EP) const;
	std::unique_ptr<GBACast> Cast(const uint8_t CST) const;
	std::unique_ptr<GBASocialMove> SocialMove(const uint8_t Move) const;

	bool FixChecksum();
private:
	uint8_t Move = 0;
	uint8_t Slot = 0;
	uint32_t Offs = 0;

	/* The Sims 2 GBA is annoying and their movement crap, so this is necessary. */
	uint32_t Offset(const uint32_t DefaultOffs = 0x0, const uint32_t MoveOffs1 = 0x0, const uint32_t MoveOffs2 = 0x0) const {
		switch(this->Move) {
			case 1:
				return this->Offs + MoveOffs1;

			case 2:
				return this->Offs + MoveOffs2;
		}

		return this->Offs + DefaultOffs;
	};

	/* This contains all official Episode Values found at offset (Slot * 0x1000) + 0x1A9. */
	static constexpr uint8_t EPVals[12] = {
		0x0, 0x1, 0x3, 0x7, // Tutorial + Season 1.
		0x6, 0xA, 0x8, 0xF, // Season 2.
		0xD, 0x5, 0x16, 0x15 // Season 3.
	};
};

#endif