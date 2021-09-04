/*
*   This file is part of Sim2Editor-CPPCore
*   Copyright (C) 2020-2021 Sim2Team
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

#include "NDSSav.hpp"
#include "../shared/Checksum.hpp"
#include "../shared/DataHelper.hpp"


namespace S2Editor {
	/*
		Initialize the NDS SAV.

		const std::string &SAVFile: The SAVFile path.
	*/
	NDSSAV::NDSSAV(const std::string &SAVFile) {
		FILE *SAV = fopen(SAVFile.c_str(), "r");

		if (SAV) {
			fseek(SAV, 0, SEEK_END);
			this->SavSize = ftell(SAV);
			fseek(SAV, 0, SEEK_SET);

			if (this->SavSize == 0x40000 || this->SavSize == 0x80000) {
				this->SavData = std::make_unique<uint8_t[]>(this->GetSize());
				fread(this->SavData.get(), 1, this->GetSize(), SAV);

				this->ValidationCheck();
			}

			fclose(SAV);
		}
	};

	/*
		A second way for the Initializer.

		std::unique_ptr<uint8_t[]> &Data: The raw Save Buffer.
		const uint32_t Size: The size of the Save Buffer.
	*/
	NDSSAV::NDSSAV(std::unique_ptr<uint8_t[]> &Data, const uint32_t Size) {
		if (Size == 0x40000 || Size == 0x80000) {
			this->SavData = std::move(Data);
			this->SavSize = Size;

			this->ValidationCheck();
		}
	};

	/* Some Save Validation checks. */
	void NDSSAV::ValidationCheck() {
		if (!this->GetData()) return;

		uint8_t Count = 0, Reg = 0;
		for (uint8_t Slot = 0; Slot < 5; Slot++) { // Check for all 5 possible Slots.
			Count = 0; // Reset Count here.

			for (uint8_t ID = 0; ID < 8; ID++) {
				if (ID == 0x4) {
					for (Reg = 0; Reg < 3; Reg++) {
						if (this->GetData()[(Slot * 0x1000) + ID] == this->SlotIdent[ID] + Reg) {
							Count++;
							break;
						}
					}

				} else {
					if (this->GetData()[(Slot * 0x1000) + ID] == this->SlotIdent[ID]) Count++;
				}
			}

			if (Count == 8) {
				this->SavValid = true;
				this->Region = (Reg == 2 ? NDSSavRegion::Jpn : NDSSavRegion::Int);
				break;
			}
		}

		if (this->GetValid()) {
			for (uint8_t Idx = 0; Idx < 3; Idx++) this->Slots[Idx] = this->FetchSlot(Idx, Reg); // Fetch Slot Locations.
		}
	};

	/*
		This one is called at NDSSAV class constructor 3 times, to get the proper SAVSlot offset.

		This function has been ported of the LSSD Tool, SuperSaiyajinStackZ created.
	*/
	int8_t NDSSAV::FetchSlot(const uint8_t SavSlot, const uint8_t Reg) {
		if (!this->GetData()) return -1;

		int8_t LastSavedSlot = -1, IDCount = 0;
		uint32_t SavCount[5] = { 0x0 };
		bool SavSlotExist[5] = { false };

		/* Looping through all possible Locations. */
		for (uint8_t Slot = 0; Slot < 5; Slot++) {
			IDCount = 0; // First reset here to 0.

			/* Check for Identifier. */
			for (uint8_t ID = 0; ID < 8; ID++) {
				if (this->GetData()[(Slot * 0x1000) + ID] == this->SlotIdent[ID] + (ID == 0x4 ? Reg : 0x0)) IDCount++;
			}

			/* If 8, then it properly passed the slot existence check. */
			if (IDCount == 8) {
				/* Check, if current slot is also the actual SAVSlot. It seems 0xC and 0xD added is the Slot, however 0xD seems never be touched from the game and hence like all the time 0x0? */
				if ((this->GetData()[(Slot * 0x1000) + 0xC] + this->GetData()[(Slot * 0x1000) + 0xD]) == SavSlot) {
					/* Now get the SavCount. */
					SavCount[Slot] = DataHelper::Read<uint32_t>(this->GetData(), (Slot * 0x1000) + 0x8);
					SavSlotExist[Slot] = true;
				}
			}
		}

		/* Here we check and return the proper last saved Slot. */
		uint32_t HighestCount = 0;

		for (uint8_t Slot = 0; Slot < 5; Slot++) {
			if (SavSlotExist[Slot]) { // Ensure the Slot existed before.
				if (SavCount[Slot] > HighestCount) { // Ensure count is higher.
					HighestCount = SavCount[Slot];
					LastSavedSlot = Slot;
				}
			}
		}

		return LastSavedSlot;
	};

	/*
		Return a NDSPainting class.

		const uint8_t Idx: The Painting Index ( 0 - 19 ).
	*/
	std::unique_ptr<NDSPainting> NDSSAV::Painting(const uint8_t Idx) const {
		if (Idx >= 20) return nullptr;

		return std::make_unique<NDSPainting>(Idx);
	};

	/*
		Return a NDSSlot class.

		const uint8_t Slot: The NDSSAV Slot ( 0 - 2 ).
	*/
	std::unique_ptr<NDSSlot> NDSSAV::Slot(const uint8_t Slot) const {
		if (!this->SlotExist(Slot)) return nullptr;

		return std::make_unique<NDSSlot>(this->Slots[Slot], this->Region);
	};

	/*
		Finish call before writting to file.

		Fix the Checksum of all existing Slots, if invalid.
	*/
	void NDSSAV::Finish() {
		if (!this->GetValid()) return;

		/* Update the Checksum of the Sav Slots. */
		for (uint8_t Slot = 0; Slot < 3; Slot++) {
			if (this->SlotExist(Slot)) this->Slot(Slot)->FixChecksum();
		}

		/* Update the Checksum of the Paintings. */
		for (uint8_t Idx = 0; Idx < 20; Idx++) {
			std::unique_ptr<NDSPainting> PTG = this->Painting(Idx);

			if (PTG->Valid()) PTG->UpdateChecksum();
		}
	};

	/*
		Return, wheter a Slot is valid / exist.

		const uint8_t Slot: The Slot to check.
	*/
	bool NDSSAV::SlotExist(const uint8_t Slot) const {
		if (Slot > 2 || !this->GetValid()) return false;

		return this->Slots[Slot] != -1;
	};
};