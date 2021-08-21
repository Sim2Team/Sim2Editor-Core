/*
*   This file is part of Sim2Editor-CPPCore
*   Copyright (C) 2020-2021 SuperSaiyajinStackZ
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

#ifndef _SIM2EDITOR_NDS_CORE_HPP
#define _SIM2EDITOR_NDS_CORE_HPP

#include <cstring> // memcpy(...).
#include <math.h> // std::min<>(), std::max<>(...).
#include <memory> // std::unique_ptr<>.
#include <string> // std::string.
#include <vector> // std::vector<>.

namespace S2NDSCore {
	/* Declare all used enum classes here. */
	enum class SavRegion : uint8_t { UNKNOWN = 0x0, INT = 0x1, JPN = 0x2 };


	/* Declare all used classes here. */
	class SAV;
	class Slot;


	/* A few variables for the S2Editor NDS Core. */
	extern std::unique_ptr<SAV> Sav;


	/*
		The Sims 2 NDS Checksum namespace implementation.
		Main Author: SuperSaiyajinStackZ.

		Just containing a function, which handles the Checksum calculation, that returns an uint16_t.
	*/
	namespace Checksum {
		uint16_t Calc(const uint8_t *Buffer, const uint16_t StartOffs, const uint16_t EndOffs, const std::vector<int> &SkipOffs);
	};


	/*
		The Sims 2 NDS SaveHandler namespace implementation.
		Main Author: SuperSaiyajinStackZ.

		This is used to load a save and some other utility functions.
	*/
	namespace SaveHandler {
		bool LoadSav(const std::string &File);
		bool LoadSav(std::unique_ptr<uint8_t[]> &Data, const uint32_t Size); // Overload function for direct pointer passing.
		bool WriteBack(const std::string &File);
	};


	/*
		The Sims 2 NDS SimUtils namespace implementation.
		Main Author: SuperSaiyajinStackZ.

		This is used for some formatting related things.
	*/
	namespace SimUtils {
		const std::string TimeString(const uint16_t Time, const bool AMPM = false);
		const std::string SimoleonsString(const uint32_t Simoleons);
	};


	/*
		The Sims 2 NDS Strings namespace implementation.
		Main Author: SuperSaiyajinStackZ.

		This is used for some string related things.
	*/
	namespace Strings {
		extern const std::vector<std::string> SkillPointNames; // Skill Points.
	};


	/*
		The Sims 2 NDS SAV Save Editing class implementation.
		Main Author: SuperSaiyajinStackZ.

		NOTE: NEVER ACCESS THIS CLASS AND OTHER SUB CLASSES OUTSIDE THE S2NDSCore::SaveHandler AND S2NDSCore::Sav CALL!!!
	*/
	class SAV {
	public:
		SAV(const std::string &File); // Way 1 with a file.
		SAV(std::unique_ptr<uint8_t[]> &Data, const uint32_t Size) :
			SavData(std::move(Data)), SavSize(Size) { this->SavValid = this->ValidationCheck(); }; // Way 2 with a Buffer.

		/* Validation check for the SaveData. */
		bool ValidationCheck();

		/* Return some stuff. */
		int8_t FetchSlot(const uint8_t SavSlot, const uint8_t Reg);
		uint8_t *GetData() const { return this->SavData.get(); };
		uint32_t GetSize() const { return this->SavSize; };
		SavRegion GetRegion() const { return this->Region; };
		bool GetChangesMade() const { return this->ChangesMade; };
		bool GetValid() const { return this->SavValid; };

		/* Reads stuff from the Save Buffer. */
		template<class T> T Read(const uint32_t Offs, const bool Fetcher = false) const {
			if ((!this->GetValid() && !Fetcher) || !this->GetData()) return 0;

			T Temp;
			memcpy(&Temp, this->SavData.get() + Offs, sizeof(T));
			return Temp;
		};

		/* Writes stuff to the Save Buffer. */
		template<class T> void Write(const uint32_t Offs, T Data) {
			if (!this->GetValid() || !this->GetData()) return; // Do nothing.

			/* NOTE: This only works with `>>` operator types. */
			for (size_t Idx = 0; Idx < sizeof(T); Idx++) {
				this->SavData.get()[Offs + Idx] = (uint8_t)Data;
				Data >>= 8; // Go to the next byte.
			};

			if (!this->ChangesMade) this->ChangesMade = true;
		};

		/* Some other Read and Writes. */
		bool ReadBit(const uint32_t Offs, const uint8_t BitIndex) const;
		void WriteBit(const uint32_t Offs, const uint8_t BitIndex, const bool IsSet);
		uint8_t ReadBits(const uint32_t Offs, const bool First) const;
		void WriteBits(const uint32_t Offs, const bool First, const uint8_t Data);
		std::string ReadString(const uint32_t Offs, const uint32_t Length) const;
		void WriteString(const uint32_t Offs, const uint32_t Length, const std::string &Str);

		bool SlotExist(const uint8_t Slot) const;
		std::unique_ptr<Slot> _Slot(const uint8_t Slt) const;
		void Finish();
	private:
		std::unique_ptr<uint8_t[]> SavData;
		uint32_t SavSize = 0;
		bool ChangesMade = false, SavValid = false;
		SavRegion Region = SavRegion::UNKNOWN;
		static constexpr uint8_t SlotIdent[8] = { 0x64, 0x61, 0x74, 0x0, 0x1F, 0x0, 0x0, 0x0 };
		int8_t Slots[3] = { -1, -1, -1 };
	};


	/*
		The Sims 2 NDS Slot Save Editing class implementation.
		Main Author: SuperSaiyajinStackZ.
	*/
	class Slot {
	public:
		Slot(const uint8_t Slt) : Slt(Slt), Offs(Slt * 0x1000) { };

		/* Main things. */
		uint32_t Simoleons() const;
		void Simoleons(const uint32_t V);
		std::string Name() const;
		void Name(const std::string &V);

		/* Collectables. */
		uint8_t Fuelrods() const;
		void Fuelrods(const uint8_t V);
		uint8_t Plates() const;
		void Plates(const uint8_t V);
		uint8_t Gourds() const;
		void Gourds(const uint8_t V);
		uint8_t Spaceship() const;
		void Spaceship(const uint8_t V);

		/* Skill Points. */
		uint8_t Creativity() const;
		void Creativity(const uint8_t V);
		uint8_t Business() const;
		void Business(const uint8_t V);
		uint8_t Body() const;
		void Body(const uint8_t V);
		uint8_t Charisma() const;
		void Charisma(const uint8_t V);
		uint8_t Mechanical() const;
		void Mechanical(const uint8_t V);

		/* Pocket Items + Count. */
		uint8_t PocketCount() const;
		void PocketCount(const uint8_t V);
		uint16_t PocketID(const uint8_t Index) const;
		void PocketID(const uint8_t Index, const uint16_t V);

		bool FixChecksum();
	private:
		uint8_t Slt = 0;
		uint32_t Offs = 0;
	};
};

#endif