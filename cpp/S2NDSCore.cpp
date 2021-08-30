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

#include "S2NDSCore.hpp" // main include.
#include <unistd.h> // access().

/*
	---------------------------------------------
	The Sims 2 Nintendo DS Save File Editing Core
	---------------------------------------------

	File: ASJP.sav
	Authors: SuperSaiyajinStackZ, Sim2Team
	Version: 0.2
	Purpose: Easy editing of a The Sims 2 Nintendo DS Savefile.
	Category: Save File Editing Core
	Last Updated: 21 August 2021
	---------------------------------------------

	Research used from here: https://github.com/Sim2Team/Sim2Research.


	-----------------------
	Explanation of the Core
	-----------------------

	* Use S2NDSCore::SaveHandler::LoadSav(const std::string &) to load a Savefile from your SD Card.
	* Use S2NDSCore::SaveHandler::LoadSav(std::unique_ptr<uint8_t[]> &, const uint32_t) to load a Savefile from an already existing Buffer.
	* Use S2NDSCore::SaveHandler::WriteBack(const std::string &) to update the checksums + write your changes back to the Savefile.
	* Use S2NDSCore::Sav to access the Save Pointer and with that.. all the sub classes if needed. DO NOT ACCESS THOSE OUTSIDE, BECAUSE THEY RELY ON S2NDSCore::Sav's POINTER!!!

	Another Note about THIS Core:
		THIS IS NOT THREAD-SAFE!!!, because I don't care about it, since I don't really work with Threads anyways.

	To compile, you need to compile this with C++17 or above.
*/

namespace S2NDSCore {
	std::unique_ptr<SAV> Sav = nullptr;

	/*
		////////////////////////////////////////////////

		The Sims 2 NDS Checksum namespace implementation.
		Main Author: SuperSaiyajinStackZ.

		////////////////////////////////////////////////
	*/

	/*
		I rewrote the Checksum calculation function, to WORK with both, GBA and NDS versions.

		const uint8_t *Buffer: The Save Buffer.
		const uint16_t StartOffs: The Start offset. (NOTE: You'll have to do '/ 2', because it's 2 byte based).
		const uint16_t EndOffs: The End offset. Same NOTE as above applies here as well.
		const std::vector<int> &Skipoffs:
			The Offsets which to skip (Only needed on the NDS version, also same NOTE as above applies as well).
	*/
	uint16_t Checksum::Calc(const uint8_t *Buffer, const uint16_t StartOffs, const uint16_t EndOffs, const std::vector<int> &SkipOffs) {
		if (!Buffer) return 0;

		uint8_t Byte1 = 0, Byte2 = 0;
		bool Skip = false;

		for (uint16_t Idx = StartOffs; Idx < EndOffs; Idx++) {
			if (!SkipOffs.empty()) { // Only do this, if it isn't empty.
				for (uint8_t I = 0; I < SkipOffs.size(); I++) {
					if (Idx == SkipOffs[I]) {
						Skip = true; // We'll skip those bytes here.
						break;
					}
				}
			}

			if (Skip) {
				Skip = false;
				continue;
			}

			if (Buffer[(Idx * 2)] + Byte1 > 255) Byte2++;
			Byte1 += Buffer[(Idx * 2)];
			Byte2 += Buffer[(Idx * 2) + 1];
		}

		Byte2++;
		return (256 * (uint8_t)-Byte2) + (uint8_t)-Byte1;
	};


	/*
		////////////////////////////////////////////////////

		The Sims 2 NDS SaveHandler namespace implementation.
		Main Author: SuperSaiyajinStackZ.

		////////////////////////////////////////////////////
	*/

	/*
		Main Save Loading method by passing a path to a Savefile.

		const std::string &File: The Savefile path.

		Returns true, if the save is valid.
	*/
	bool SaveHandler::LoadSav(const std::string &File) {
		S2NDSCore::Sav = std::make_unique<S2NDSCore::SAV>(File);

		return S2NDSCore::Sav->GetValid();
	};

	/*
		Another Method to load the Savefile by passing the direct Save Buffer over.

		std::unique_ptr<uint8_t[]> &Data: The Save Buffer.
		const uint32_t Size: The Savesize.

		Returns true, if the save is valid.
	*/
	bool SaveHandler::LoadSav(std::unique_ptr<uint8_t[]> &Data, const uint32_t Size) {
		/* 256 and 512 KB are valid sizes for it. */
		if (Size == 0x40000 || Size == 0x80000) {
			S2NDSCore::Sav = std::make_unique<S2NDSCore::SAV>(Data, Size);

			return S2NDSCore::Sav->GetValid();
		}

		return false;
	};

	/*
		Writes the changes back to the Savefile, if changes are made and the save is valid.

		const std::string &File: The Savefile path.

		Returns true, if the save got successfully written back.
	*/
	bool SaveHandler::WriteBack(const std::string &File) {
		bool Res = false;
		if (access(File.c_str(), F_OK) != 0) return Res;

		if (S2NDSCore::Sav && S2NDSCore::Sav->GetData() && S2NDSCore::Sav->GetValid() && S2NDSCore::Sav->GetChangesMade()) {
			FILE *Out = fopen(File.c_str(), "rb+");

			if (Out) {
				S2NDSCore::Sav->Finish(); // Update Checksum, if necessary.
				fwrite(S2NDSCore::Sav->GetData(), 1, S2NDSCore::Sav->GetSize(), Out);
				fclose(Out);

				Res = true;
			}
		}

		return Res;
	};


	/*
		/////////////////////////////////////////////////

		The Sims 2 NDS SimUtils namespace implementation.
		Main Author: SuperSaiyajinStackZ.

		/////////////////////////////////////////////////
	*/

	/*
		Returns the current time as a 24 Hour or 12 Hour string.

		const uint16_t Time: The current time as an uint16_t.
		const bool AMPM: If using AM / PM or 24 Hours.

		This Results in: '13:44' or '01:44 PM'.
	*/
	const std::string SimUtils::TimeString(const uint16_t Time, const bool AMPM) {
		char TimeBuffer[(AMPM ? 11 : 8)];
		const uint8_t Minute = (uint8_t)(Time >> 8), Hour = (uint8_t)Time;

		if (AMPM) snprintf(TimeBuffer, sizeof(TimeBuffer), "%02d:%02d %s", (Hour > 11 ? Hour - 12 : Hour), Minute, (Hour > 11 ? "PM" : "AM"));
		else snprintf(TimeBuffer, sizeof(TimeBuffer), "%02d:%02d", Hour, Minute);

		return TimeBuffer;
	};

	/*
		Returns the current Simoleon amount as a string.

		const uint32_t Simoleons: The current Simoleons.

		This results in 123.456$.
	*/
	const std::string SimUtils::SimoleonsString(const uint32_t Simoleons) {
		std::string SString = std::to_string(Simoleons);

		/* Here we'll add the periods. */
		switch(SString.size()) {
			case 4:
			case 5:
			case 6:
				SString.insert(SString.end() - 3, '.');
				break;

			case 7: // Technically, 7 Digits are possible too for the Save, but that should never happen.
			case 8:
			case 9:
				SString.insert(SString.end() - 6, '.');
				SString.insert(SString.end() - 3, '.');
				break;

			default:
				break;
		}

		SString += "§"; // Simoleons sign.
		return SString;
	};


	/*
		////////////////////////////////////////////////

		The Sims 2 NDS Strings namespace implementation.
		Main Author: SuperSaiyajinStackZ.

		/////////////////////////////////////////////////
	*/
	const std::vector<std::string> Strings::SkillPointNames = { "Creativity", "Business", "Body", "Charisma", "Mechanical" };


	/*
		///////////////////////////////////////////////////

		The Sims 2 NDS SAV Save Editing class implementation.
		Main Author: SuperSaiyajinStackZ.

		///////////////////////////////////////////////////
	*/

	/*
		Main Save Constructor by passing over the path to the Savefile.

		const std::string &File: The Savefile path.
	*/
	SAV::SAV(const std::string &File) {
		if (access(File.c_str(), F_OK) != 0) { // File doesn't exist.
			this->SavValid = false;
			this->SavSize = 0;
			return;
		}

		FILE *In = fopen(File.c_str(), "r");
		if (In) {
			fseek(In, 0, SEEK_END);
			this->SavSize = ftell(In);
			fseek(In, 0, SEEK_SET);

			/* 256 and 512 KB are valid sizes for it. */
			if (this->SavSize == 0x40000 || this->SavSize == 0x80000) {
				this->SavData = std::make_unique<uint8_t[]>(this->SavSize);
				fread(this->SavData.get(), 1, this->SavSize, In);
				this->SavValid = this->ValidationCheck();

			} else {
				this->SavValid = false;
			}

			fclose(In);
		}
	};

	/*
		Some Save Validation checks.

		Returns true if the Save is valid, false if not.
	*/
	bool SAV::ValidationCheck() {
		if (!this->GetData()) return false;

		bool Res = false;
		uint8_t Count = 0, Region = 0;

		for (uint8_t Slot = 0; Slot < 5; Slot++) { // Check for all 5 possible Slots.
			Count = 0; // Reset Count here.

			for (uint8_t ID = 0; ID < 8; ID++) {
				if (ID == 4) { // 4 is region specific.
					for (Region = 0; Region < 3; Region++) {
						if (this->GetData()[(Slot * 0x1000) + ID] == this->SlotIdent[ID] + Region) {
							Count++;
							break;
						}
					}

				} else {
					if (this->GetData()[(Slot * 0x1000) + ID] == this->SlotIdent[ID]) Count++;
				}
			}

			/* If Count == 8, then the header was right. */
			if (Count == 8) {
				this->Region = (Region == 2 ? S2NDSCore::SavRegion::JPN : S2NDSCore::SavRegion::INT);
				Res = true;
				break;
			}
		}

		/* If good -> Fetch all active Slots. */
		if (Res) {
			for (uint8_t Idx = 0; Idx < 3; Idx++) this->Slots[Idx] = this->FetchSlot(Idx, Region); // Fetch Slot Locations.
		}

		return Res;
	};

	/*
		This one is called at the SAV's Validation Check function, to get the proper Save Slot Locations.

		This function has been ported of the LSSD Tool, SuperSaiyajinStackZ created.
	*/
	int8_t SAV::FetchSlot(const uint8_t SavSlot, const uint8_t Reg) {
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
					/* Now get the SAVCount. */
					SavCount[Slot] = this->Read<uint32_t>((Slot * 0x1000) + 0x8, true);
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
		Return a bit from the Save Buffer.

		const uint32_t Offs: The Offset to read from.
		const uint8_t BitIndex: The Bit index ( 0 - 7 ).
	*/
	bool SAV::ReadBit(const uint32_t Offs, const uint8_t BitIndex) const {
		if (!this->GetValid() || !this->GetData() || BitIndex > 7) return false;

		return (this->GetData()[Offs] >> BitIndex & 1) != 0;
	};
	/*
		Set a bit to the Save Buffer.

		const uint32_t Offs: The Offset to write to.
		const uint8_t BitIndex: The Bit index ( 0 - 7 ).
		const bool IsSet: If it's set (1) or not (0).
	*/
	void SAV::WriteBit(const uint32_t Offs, const uint8_t BitIndex, const bool IsSet) {
		if (!this->GetValid() || !this->GetData() || BitIndex > 7) return;

		this->GetData()[Offs] &= ~(1 << BitIndex);
		this->GetData()[Offs] |= (IsSet ? 1 : 0) << BitIndex;

		if (!this->ChangesMade) this->ChangesMade = true;
	};

	/*
		Read Lower / Upper Bits from the Save Buffer.

		const uint32_t Offs: The offset where to read from.
		const bool First: If Reading from the first four bits, or second.
	*/
	uint8_t SAV::ReadBits(const uint32_t Offs, const bool First) const {
		if (!this->GetValid() || !this->GetData()) return 0x0;

		if (First) return (this->GetData()[Offs] & 0xF); // Bit 0 - 3.
		else return (this->GetData()[Offs] >> 4); // Bit 4 - 7.
	};
	/*
		Write Lower / Upper Bits to the Save Buffer.

		const uint32_t Offs: The offset where to write to.
		const bool First: If Writing on the first four bits, or second.
		const uint8_t Data: The Data to write.
	*/
	void SAV::WriteBits(const uint32_t Offs, const bool First, const uint8_t Data) {
		if (!this->GetValid() || !this->GetData() || Data > 0xF) return;

		if (First) this->GetData()[Offs] = (this->GetData()[Offs] & 0xF0) | (Data & 0xF); // Bit 0 - 3.
		else this->GetData()[Offs] = (this->GetData()[Offs] & 0x0F) | (Data << 4); // Bit 4 - 7.

		if (!this->ChangesMade) this->ChangesMade = true;
	};

	/*
		Read a string from the Save Buffer.

		const uint32_t Offs: The Offset from where to read from.
		const uint32_t Length: The Length to read.

		TODO: Handle Japanese special signs.
	*/
	std::string SAV::ReadString(const uint32_t Offs, const uint32_t Length) const {
		if (!this->GetValid() || !this->GetData()) return "";

		std::string Str;

		for (int Idx = 0; Idx < (int)Length; Idx++) {
			if (this->GetData()[Offs + Idx] == 0x0) break; // 0x0 -> End.

			Str += this->GetData()[Offs + Idx];
		}

		return Str;
	};
	/*
		Write a string to the Save Buffer.

		const uint32_t Offs: The offset from where to write to.
		const uint32_t Length: The length to write.
		const std::string &Str: The string to write.

		TODO: Handle Japanese special signs.
	*/
	void SAV::WriteString(const uint32_t Offs, const uint32_t Length, const std::string &Str) {
		if (!this->GetValid() || !this->GetData()) return;

		for (int Idx = 0; Idx < (int)Length; Idx++) {
			if (Idx < (int)Str.size()) this->GetData()[Offs + Idx] = Str[Idx]; // The index is inside the string length, so write that.
			else this->GetData()[Offs + Idx] = 0; // Index outside the string length.. so write 0.
		}

		if (!this->ChangesMade) this->ChangesMade = true;
	};


	/*
		Return, wheter a Slot is valid / exist.

		const uint8_t Slot: The Slot to check ( 0 - 2 ).
	*/
	bool SAV::SlotExist(const uint8_t Slot) const {
		if (Slot > 2 || !this->GetValid()) return false;

		return this->Slots[Slot] != -1;
	};

	/*
		Return a Slot class.

		const uint8_t Slt: The Slot ( 0 - 2 ).
	*/
	std::unique_ptr<Slot> SAV::_Slot(const uint8_t Slt) const {
		if (!this->SlotExist(Slt)) return nullptr;

		return std::make_unique<Slot>(this->Slots[Slt]);
	};

	/*
		The Save Finish call, which updates the checksum and such.
	*/
	void SAV::Finish() {
		if (!this->GetValid()) return;

		for (uint8_t Slot = 0; Slot < 3; Slot++) {
			if (this->SlotExist(Slot)) this->_Slot(Slot)->FixChecksum();
		}
	};


	/*
		////////////////////////////////////////////////////

		The Sims 2 NDS Slot Save Editing class implementation.
		Main Author: SuperSaiyajinStackZ.

		////////////////////////////////////////////////////
	*/

	/* Get and Set Simoleons. */
	uint32_t Slot::Simoleons() const { return S2NDSCore::Sav->Read<uint32_t>(this->Offs + 0x2C); };
	void Slot::Simoleons(uint32_t V) { S2NDSCore::Sav->Write<uint32_t>(this->Offs + 0x2C, (std::min<uint32_t>(999999, V))); };

	/* Get and Set Name. */
	std::string Slot::Name() const { return S2NDSCore::Sav->ReadString(this->Offs + 0x30, 0x7); };
	void Slot::Name(const std::string &V) { S2NDSCore::Sav->WriteString(this->Offs + 0x30, 0x7, V); };

	/* Get and Set Nuclear Fuelrods. */
	uint8_t Slot::Fuelrods() const { return S2NDSCore::Sav->Read<uint8_t>(this->Offs + 0xBC); };
	void Slot::Fuelrods(const uint8_t V) { S2NDSCore::Sav->Write<uint8_t>(this->Offs + 0xBC, (std::min<uint8_t>(250, V))); };

	/* Get and Set License Plates. */
	uint8_t Slot::Plates() const { return S2NDSCore::Sav->Read<uint8_t>(this->Offs + 0xBD); };
	void Slot::Plates(const uint8_t V) { S2NDSCore::Sav->Write<uint8_t>(this->Offs + 0xBD, (std::min<uint8_t>(250, V))); };

	/* Get and Set Strange Gourds. */
	uint8_t Slot::Gourds() const { return S2NDSCore::Sav->Read<uint8_t>(this->Offs + 0xBE); };
	void Slot::Gourds(const uint8_t V) { S2NDSCore::Sav->Write<uint8_t>(this->Offs + 0xBE, (std::min<uint8_t>(250, V))); };

	/* Get and Set Alien Spaceship Parts. */
	uint8_t Slot::Spaceship() const { return S2NDSCore::Sav->Read<uint8_t>(this->Offs + 0xBF); };
	void Slot::Spaceship(const uint8_t V) { S2NDSCore::Sav->Write<uint8_t>(this->Offs + 0xBF, (std::min<uint8_t>(250, V))); };

	/* Get and Set Creativity Skill Points. */
	uint8_t Slot::Creativity() const { return S2NDSCore::Sav->Read<uint8_t>(this->Offs + 0xDF); };
	void Slot::Creativity(const uint8_t V) { S2NDSCore::Sav->Write<uint8_t>(this->Offs + 0xDF, (std::min<uint8_t>(10, V))); };

	/* Get and Set Business Skill Points. */
	uint8_t Slot::Business() const { return S2NDSCore::Sav->Read<uint8_t>(this->Offs + 0xE0); };
	void Slot::Business(const uint8_t V) { S2NDSCore::Sav->Write<uint8_t>(this->Offs + 0xE0, (std::min<uint8_t>(10, V))); };

	/* Get and Set Body Skill Points. */
	uint8_t Slot::Body() const { return S2NDSCore::Sav->Read<uint8_t>(this->Offs + 0xE1); };
	void Slot::Body(const uint8_t V) { S2NDSCore::Sav->Write<uint8_t>(this->Offs + 0xE1, (std::min<uint8_t>(10, V))); };

	/* Get and Set Charisma Skill Points. */
	uint8_t Slot::Charisma() const { return S2NDSCore::Sav->Read<uint8_t>(this->Offs + 0xE2); };
	void Slot::Charisma(const uint8_t V) { S2NDSCore::Sav->Write<uint8_t>(this->Offs + 0xE2, (std::min<uint8_t>(10, V))); };

	/* Get and Set Mechanical Skill Points. */
	uint8_t Slot::Mechanical() const { return S2NDSCore::Sav->Read<uint8_t>(this->Offs + 0xE3); };
	void Slot::Mechanical(const uint8_t V) { S2NDSCore::Sav->Write<uint8_t>(this->Offs + 0xE3, (std::min<uint8_t>(10, V))); };

	/* Get and Set the Pocket Item Count. */
	uint8_t Slot::PocketCount() const { return S2NDSCore::Sav->Read<uint8_t>(this->Offs + 0xCF); };
	void Slot::PocketCount(const uint8_t V) { S2NDSCore::Sav->Write<uint8_t>(this->Offs + 0xCF, std::min<uint8_t>(6, V)); };

	/* Get and Set the Pocket Item IDs. */
	uint16_t Slot::PocketID(const uint8_t Index) const { return S2NDSCore::Sav->Read<uint16_t>(this->Offs + 0xC3 + (std::min<uint8_t>(6, Index) * 2)); };
	void Slot::PocketID(const uint8_t Index, const uint16_t V) {
		S2NDSCore::Sav->Write<uint8_t>(this->Offs + 0xC3 + (std::min<uint8_t>(6, Index) * 2), V);

		uint8_t Count = 0;
		for (uint8_t Idx = 0; Idx < 6; Idx++) {
			if (this->PocketID(Idx) != 0x0) Count++; // Is that the proper way? TODO: More research for actual empty IDs.
		}

		this->PocketCount(Count);
	};

	/*
		Fix the Checksum of the current Slot, if invalid.

		Returns false if already valid, true if got fixed.
	*/
	bool Slot::FixChecksum() {
		const uint16_t CurCHKS = S2NDSCore::Sav->Read<uint16_t>(this->Offs + 0x28);
		const std::vector<int> Offs = { ((int)this->Offs + 0x12) / 2, ((int)this->Offs + 0x28) / 2 };
		const uint16_t Calced = Checksum::Calc(S2NDSCore::Sav->GetData(), (this->Offs + 0x10) / 2, (this->Offs + 0x1000) / 2, Offs);

		if (Calced != CurCHKS) { // If the calced result is NOT the current checksum.
			S2NDSCore::Sav->Write<uint16_t>(this->Offs + 0x28, Calced);
			return true;
		}

		return false;
	};
};