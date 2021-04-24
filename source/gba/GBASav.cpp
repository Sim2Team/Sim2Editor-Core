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

namespace S2Editor {
	/*
		Initialize the GBA SAV.

		const std::string &SAVFile: The SAVFile path.
	*/
	GBASAV::GBASAV(const std::string &SAVFile) {
		FILE *SAV = fopen(SAVFile.c_str(), "r");

		if (SAV) {
			fseek(SAV, 0, SEEK_END);
			this->SavSize = ftell(SAV); // Get the SAVSize.
			fseek(SAV, 0, SEEK_SET);

			this->SavData = std::make_unique<uint8_t[]>(this->GetSize());
			fread(this->SavData.get(), 1, this->GetSize(), SAV);
			fclose(SAV);

			this->ValidationCheck();
		}
	};

	/*
		Some Save Validation checks.
	*/
	void GBASAV::ValidationCheck() {
		if (!this->GetData()) return;

		/* Now do the Validation check through the Save Header with the GBAIdents. */
		bool Res = true;
		for (uint8_t Idx = 0; Idx < 7; Idx++) {
			if (this->GetData()[Idx] != this->GBAIdent[Idx]) {
				Res = false;
				break;
			}
		}

		/* Language Checks as well, because why not. */
		if (this->GetData()[0xA] > 5) { // Language Index is 6 or larger, which is "blank" and can break the game.
			this->GetData()[0xA] = 0; // English.
			this->ChangesMade = true;
		}

		this->SavValid = Res;
	};

	/*
		Return, wheter a Slot is valid / exist.

		const uint8_t Slot: The Slot to check.
	*/
	bool GBASAV::SlotExist(const uint8_t Slot) {
		if (Slot < 1 || Slot > 4 || !this->GetValid()) return false;

		for (uint8_t Idx = 0; Idx < 10; Idx++) {
			if (this->GetData()[(Slot * 0x1000) + Idx] != 0) return true;
		}

		return false;
	};

	/*
		Return a GBASlot class.

		const uint8_t Slot: The GBASAV Slot ( 1 - 4 ).
	*/
	std::unique_ptr<GBASlot> GBASAV::Slot(const uint8_t Slot) {
		if (!this->SlotExist(Slot)) return nullptr;

		return std::make_unique<GBASlot>(Slot);
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
			if (this->SlotExist(Slot)) this->Slot(Slot)->FixChecksum();
		}

		/* Do the same with the Settings. */
		this->Settings()->UpdateChecksum();
	};
};