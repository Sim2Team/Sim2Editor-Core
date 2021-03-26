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

#include "GBASav.hpp"
#include "../shared/Checksum.hpp"

/*
	Initialize the GBA SAV.

	const std::string &SAVFile: The SAVFile path.
*/
GBASAV::GBASAV(const std::string &SAVFile) {
	FILE *SAV = fopen(SAVFile.c_str(), "r");

	if (SAV) {
		fseek(SAV, 0, SEEK_END);
		this->SAVSize = ftell(SAV); // Get the SAVSize.
		fseek(SAV, 0, SEEK_SET);

		this->SAVData = std::make_unique<uint8_t[]>(this->SAVSize);
		fread(this->SAVData.get(), 1, this->SAVSize, SAV);
		fclose(SAV);

		this->SAVValid = true;
		if (this->SAVData.get()[0xA] > 5) { // Language Index is 6 or larger, which is "blank" and can break the game.
			this->SAVData.get()[0xA] = 0; // English.
			this->SetChangesMade(true);
		}
	}
};

/*
	Return, wheter a Slot is valid / exist.

	const uint8_t Slot: The Slot to check.
*/
bool GBASAV::SlotExist(const uint8_t Slot) {
	if (Slot < 1 || Slot > 4 || !this->GetValid()) return false;

	for (uint8_t Idx = 0; Idx < 10; Idx++) {
		if (this->SAVData.get()[(Slot * 0x1000) + Idx] != 0) return true;
	}

	return false;
};

/*
	Return a GBASlot class.

	const uint8_t Slot: The GBASAV Slot ( 1 - 4 ).
*/
std::unique_ptr<GBASlot> GBASAV::Slot(const uint8_t Slot) {
	if (!this->SlotExist(Slot)) return nullptr;

	/*
		The second parameter are the Item count inside the House.
		This seems to affect things after the House Items to move for 0x6 per House Item.
		Max Count seems to be 0xC, sooo.. `DefaultOffs + (0xC * 0x6)` which is 0x48 / 72.
	*/
	return std::make_unique<GBASlot>(Slot, this->SAVData.get()[(Slot * 0x1000) + 0xD6]);
};

/* Get a Settings class. */
std::unique_ptr<GBASettings> GBASAV::Settings() const { return std::make_unique<GBASettings>(); };

/*
	Finish call before writting to file.

	Fix the Checksum of all existing Slots and the Settings, if invalid.
*/
void GBASAV::Finish() {
	if (!this->GetValid()) return;

	for (uint8_t Slot = 1; Slot < 5; Slot++) {
		if (this->SlotExist(Slot)) {
			if (!Checksum::GBASlotChecksumValid(this->SAVData.get(), Slot, *reinterpret_cast<uint16_t *>(this->SAVData.get() + (Slot * 0x1000) + 0xFFE))) {
				*reinterpret_cast<uint16_t *>(this->SAVData.get() + (Slot * 0x1000) + 0xFFE) = Checksum::CalcGBASlot(this->SAVData.get(), Slot);
			}
		}
	}

	/* Do the same with the Settings. */
	this->Settings()->UpdateChecksum();
};