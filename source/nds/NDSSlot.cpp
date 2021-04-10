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

#include "NDSSlot.hpp"
#include "../shared/Checksum.hpp"
#include "../shared/SAVUtils.hpp"

namespace S2Editor {
	/* Get and Set Simoleons. */
	uint32_t NDSSlot::Simoleons() const { return NDSSAVUtils::Read<uint32_t>(this->Offs + 0x2C); };
	void NDSSlot::Simoleons(uint32_t V) { NDSSAVUtils::Write<uint32_t>(this->Offs + 0x2C, (std::min<uint32_t>(999999, V))); };

	/* Get and Set Name. */
	std::string NDSSlot::Name() const { return SAVUtils::ReadString(NDSSAVUtils::SAV->GetData(), this->Offs + 0x30, 0x7); };
	void NDSSlot::Name(const std::string &V) { SAVUtils::WriteString(NDSSAVUtils::SAV->GetData(), this->Offs + 0x30, 0x7, V); };

	/* Get and Set Nuclear Fuelrods. */
	uint8_t NDSSlot::Fuelrods() const { return NDSSAVUtils::Read<uint8_t>(this->Offs + 0xBC); };
	void NDSSlot::Fuelrods(const uint8_t V) { NDSSAVUtils::Write<uint8_t>(this->Offs + 0xBC, (std::min<uint8_t>(250, V))); };

	/* Get and Set License Plates. */
	uint8_t NDSSlot::Plates() const { return NDSSAVUtils::Read<uint8_t>(this->Offs + 0xBD); };
	void NDSSlot::Plates(const uint8_t V) { NDSSAVUtils::Write<uint8_t>(this->Offs + 0xBD, (std::min<uint8_t>(250, V))); };

	/* Get and Set Strange Gourds. */
	uint8_t NDSSlot::Gourds() const { return NDSSAVUtils::Read<uint8_t>(this->Offs + 0xBE); };
	void NDSSlot::Gourds(const uint8_t V) { NDSSAVUtils::Write<uint8_t>(this->Offs + 0xBE, (std::min<uint8_t>(250, V))); };

	/* Get and Set Alien Spaceship Parts. */
	uint8_t NDSSlot::Spaceship() const { return NDSSAVUtils::Read<uint8_t>(this->Offs + 0xBF); };
	void NDSSlot::Spaceship(const uint8_t V) { NDSSAVUtils::Write<uint8_t>(this->Offs + 0xBF, (std::min<uint8_t>(250, V))); };

	/* Get and Set Creativity Skill Points. */
	uint8_t NDSSlot::Creativity() const { return NDSSAVUtils::Read<uint8_t>(this->Offs + 0xDF); };
	void NDSSlot::Creativity(const uint8_t V) { NDSSAVUtils::Write<uint8_t>(this->Offs + 0xDF, (std::min<uint8_t>(10, V))); };

	/* Get and Set Business Skill Points. */
	uint8_t NDSSlot::Business() const { return NDSSAVUtils::Read<uint8_t>(this->Offs + 0xE0); };
	void NDSSlot::Business(const uint8_t V) { NDSSAVUtils::Write<uint8_t>(this->Offs + 0xE0, (std::min<uint8_t>(10, V))); };

	/* Get and Set Body Skill Points. */
	uint8_t NDSSlot::Body() const { return NDSSAVUtils::Read<uint8_t>(this->Offs + 0xE1); };
	void NDSSlot::Body(const uint8_t V) { NDSSAVUtils::Write<uint8_t>(this->Offs + 0xE1, (std::min<uint8_t>(10, V))); };

	/* Get and Set Charisma Skill Points. */
	uint8_t NDSSlot::Charisma() const { return NDSSAVUtils::Read<uint8_t>(this->Offs + 0xE2); };
	void NDSSlot::Charisma(const uint8_t V) { NDSSAVUtils::Write<uint8_t>(this->Offs + 0xE2, (std::min<uint8_t>(10, V))); };

	/* Get and Set Mechanical Skill Points. */
	uint8_t NDSSlot::Mechanical() const { return NDSSAVUtils::Read<uint8_t>(this->Offs + 0xE3); };
	void NDSSlot::Mechanical(const uint8_t V) { NDSSAVUtils::Write<uint8_t>(this->Offs + 0xE3, (std::min<uint8_t>(10, V))); };

	/*
		Fix the Checksum of the current Slot, if invalid.

		Returns false if Slot > 4 or already valid, true if got fixed.
	*/
	bool NDSSlot::FixChecksum() {
		if (this->Slot > 4) return false;

		const uint16_t CurCHKS = NDSSAVUtils::Read<uint16_t>(this->Offs + 0x28);
		const std::vector<int> Offs = { ((int)this->Offs + 0x12) / 2, ((int)this->Offs + 0x28) / 2 };
		const uint16_t Calced = Checksum::Calc(NDSSAVUtils::SAV->GetData(), ((this->Offs + 0x10) / 2), ((this->Offs + 0x1000) / 2), Offs);

		if (Calced != CurCHKS) { // If the calced result is NOT the current checksum.
			NDSSAVUtils::Write<uint16_t>(this->Offs + 0x28, Calced);
			return true;
		}

		return false;
	};
};