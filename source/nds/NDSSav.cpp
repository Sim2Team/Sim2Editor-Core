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

#include "NDSSav.hpp"
#include "../shared/Checksum.hpp"

/*
	Initialize the NDS SAV.

	const std::string &SAVFile: The SAVFile path.
*/
NDSSAV::NDSSAV(const std::string &SAVFile) {
	FILE *SAV = fopen(SAVFile.c_str(), "r");

	if (SAV) {
		fseek(SAV, 0, SEEK_END);
		this->SAVSize = ftell(SAV); // Get the SAVSize.
		fseek(SAV, 0, SEEK_SET);

		this->SAVData = std::make_unique<uint8_t[]>(this->SAVSize);
		fread(this->SAVData.get(), 1, this->SAVSize, SAV);
		fclose(SAV);

		this->SAVValid = true;
		for (uint8_t Idx = 0; Idx < 3; Idx++) this->Slots[Idx] = this->FetchSlot(Idx);
	}
};

/*
	This one is called at NDSSAV class constructor 3 times, to get the proper SAVSlot offset.

	This function has been ported of the LSSD Tool, SuperSaiyajinStackZ created.
*/
int8_t NDSSAV::FetchSlot(const uint8_t SAVSlot) {
	if (!this->SAVValid || !this->SAVData) return -1;

	int8_t LastSavedSlot = -1, IDCount = 0;
	uint32_t SAVCount[5] = { 0x0 };
	bool SAVSlotExist[5] = { false };

	/* Looping through all possible Locations. */
	for (uint8_t Slot = 0; Slot < 5; Slot++) {
		IDCount = 0; // First reset here to 0.

		/* Check for Identifier. */
		for (uint8_t ID = 0; ID < 8; ID++) {
			if (this->SAVData.get()[(Slot * 0x1000) + ID] == this->SlotIdent[ID]) IDCount++;
		}

		/* If 8, then it properly passed the slot existence check. */
		if (IDCount == 8) {
			/* Check, if current slot is also the actual SAVSlot. It seems 0xC and 0xD added is the Slot, however 0xD seems never be touched from the game and hence like all the time 0x0? */
			if ((this->SAVData.get()[(Slot * 0x1000) + 0xC] + this->SAVData.get()[(Slot * 0x1000) + 0xD]) == SAVSlot) {
				/* Now get the SAVCount. */
				SAVCount[Slot] = *reinterpret_cast<uint32_t *>(this->SAVData.get() + (Slot * 0x1000) + 0x8);
				SAVSlotExist[Slot] = true;
			}
		}
	}

	/* Here we check and return the proper last saved Slot. */
	uint32_t HighestCount = 0;

	for (uint8_t Slot = 0; Slot < 5; Slot++) {
		if (SAVSlotExist[Slot]) { // Ensure the Slot existed before.
			if (SAVCount[Slot] > HighestCount) { // Ensure count is higher.
				HighestCount = SAVCount[Slot];
				LastSavedSlot = Slot;
			}
		}
	}

	return LastSavedSlot;
};


/*
	Return a NDSSlot class.

	const uint8_t Slot: The NDSSAV Slot ( 0 - 2 ).
*/
std::unique_ptr<NDSSlot> NDSSAV::Slot(const uint8_t Slot) {
	if (!this->SlotExist(Slot)) return nullptr;

	return std::make_unique<NDSSlot>(this->Slots[Slot]);
};

/*
	Finish call before writting to file.

	Fix the Checksum of all existing Slots, if invalid.
*/
void NDSSAV::Finish() {
	if (!this->GetValid()) return;

	for (uint8_t Slot = 0; Slot < 3; Slot++) {
		if (this->SlotExist(Slot)) this->Slot(Slot)->FixChecksum();
	}
}

/*
	Return, wheter a Slot is valid / exist.

	const uint8_t Slot: The Slot to check.
*/
bool NDSSAV::SlotExist(const uint8_t Slot) {
	if (Slot > 2 || !this->GetValid()) return false;

	return this->Slots[Slot] != -1;
};