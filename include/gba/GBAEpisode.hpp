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

#ifndef _SIM2EDITOR_CPP_CORE_GBA_EPISODE_HPP
#define _SIM2EDITOR_CPP_CORE_GBA_EPISODE_HPP

#include "../shared/CoreCommon.hpp"

class GBAEpisode {
public:
	GBAEpisode(const uint8_t Slot, const uint8_t Episode, const uint8_t Move = 0x0)
		: Episode(Episode), Offs((Slot * 0x1000) + this->SetOffset(std::min<uint8_t>(Move, 10))) { };

	uint8_t Index() const { return this->Episode; };

	uint8_t Rating(const uint8_t Category) const;
	void Rating(const uint8_t Category, const uint8_t V);

	bool State() const;
	void State(const bool V);
private:
	uint8_t Episode = 0;
	uint32_t Offs = 0;

	static constexpr uint32_t EPOffs[11] = { 0x10A, 0x114, 0x128, 0x123, 0x137, 0x12D, 0x150, 0x146, 0x11E, 0x173, 0x16E }; // 11 Episodes.

	/* Sets the base offset for the Episodes. */
	uint32_t SetOffset(const uint8_t Move) const {
		switch(Move) {
			case 1:
				return this->EPOffs[this->Episode];

			case 2:
				return this->EPOffs[this->Episode] + 0x6;
		}

		return this->EPOffs[this->Episode] - 0x6;
	};
};

#endif