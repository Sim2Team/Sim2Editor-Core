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

#include "SAVUtils.hpp"
#include <ctime>
#include <unistd.h>

/* Common Related things. */
static constexpr uint8_t GBAIdent[7] = { 0x53, 0x54, 0x57, 0x4E, 0x30, 0x32, 0x34 };
static constexpr uint8_t NDSIdent[8] = { 0x64, 0x61, 0x74, 0x0, 0x20, 0x0, 0x0, 0x0 };

namespace S2Editor {
	SAVType SAVUtils::SAV = SAVType::_NONE;
	std::string SAVUtils::SAVName = "";

	/* GBA Related things. */
	std::unique_ptr<GBASAV> GBASAVUtils::SAV = nullptr;

	/* NDS Related things. */
	std::unique_ptr<NDSSAV> NDSSAVUtils::SAV = nullptr;


	/*
		Detect the SAVType of a SAVFile.

		const std::string &File: Path to the file which to check.
	*/
	SAVType SAVUtils::DetectType(const std::string &File) {
		SAVType ST = SAVType::_NONE;

		if (access(File.c_str(), F_OK) != 0) return ST;
		FILE *In = fopen(File.c_str(), "r");

		if (In) {
			fseek(In, 0, SEEK_END);
			const uint32_t Size = ftell(In);
			fseek(In, 0, SEEK_SET);

			std::unique_ptr<uint8_t[]> Data = nullptr;
			uint8_t Count = 0;

			switch(Size) {
				case 0x10000:
				case 0x20000: // 64, 128 KB is a GBA Size.
					Data = std::make_unique<uint8_t[]>(0x7);
					fread(Data.get(), 1, 0x7, In); // Read the first 0x7 byte (Header).

					for (uint8_t ID = 0; ID < 7; ID++) {
						if (Data.get()[ID] == GBAIdent[ID]) Count++;
					}; // Identifier Check.

					if (Count == 7) ST = SAVType::_GBA; // If Count matches 7, we're good.
					break;

				case 0x40000:
				case 0x80000: // 256, 512 KB is a NDS Size.
					Data = std::make_unique<uint8_t[]>(Size);
					fread(Data.get(), 1, Size, In);

					for (uint8_t Slot = 0; Slot < 5; Slot++) { // Check for all 5 possible Slots.
						Count = 0; // Reset Count here.

						for (uint8_t ID = 0; ID < 8; ID++) {
							if (Data.get()[(Slot * 0x1000) + ID] == NDSIdent[ID]) Count++;
						}; // Identifier Check.

						if (Count == 8) {
							ST = SAVType::_NDS; // It's a NDS SAV.
							break;
						}
					}

					break;
			}

			fclose(In);
		}

		return ST;
	};

	/*
		Detect the SAVType of a SAVFile from rawdata.

		const std::unique_ptr<uint8_t[]> &Data: The raw save data to check.
		const uint32_t Size: The Size of the raw buffer.
	*/
	SAVType SAVUtils::DetectType(const std::unique_ptr<uint8_t[]> &Data, const uint32_t Size) {
		if (!Data) return SAVType::_NONE;
		uint8_t Count = 0;

		switch(Size) {
			case 0x10000:
			case 0x20000: // 64, 128 KB is a GBA Size.
				for (uint8_t ID = 0; ID < 7; ID++) {
					if (Data.get()[ID] == GBAIdent[ID]) Count++;
				}; // Identifier Check.

				if (Count == 7) return SAVType::_GBA; // If Count matches 7, we're good.
				break;

			case 0x40000:
			case 0x80000: // 256, 512 KB is a NDS Size.
				for (uint8_t Slot = 0; Slot < 5; Slot++) { // Check for all 5 possible Slots.
					Count = 0; // Reset Count here.

					for (uint8_t ID = 0; ID < 8; ID++) {
						if (Data.get()[(Slot * 0x1000) + ID] == NDSIdent[ID]) Count++;
					}; // Identifier Check.

					if (Count == 8) return SAVType::_NDS; // It's a NDS SAV.
				}

				break;
		}

		return SAVType::_NONE;
	};


	/*
		Load a SAVFile.

		const std::string &File: Path to the SAVFile.
		const std::string &BasePath: The base path where to create the Backups (Optional).
		const bool DoBackup: If creating a backup or not after loading the SAVFile (Optional).

		Returns True if the Save is Valid and False if Invalid.
	*/
	bool SAVUtils::LoadSAV(const std::string &File, const std::string &BasePath, const bool DoBackup) {
		const SAVType ST = SAVUtils::DetectType(File);
		bool Good = false;

		if (ST != SAVType::_NONE) {
			SAVUtils::SAV = ST; // Set SAVType.
			SAVUtils::SAVName = File; // Set Path.

			/* Load Proper SAV. */
			switch(SAVUtils::SAV) {
				case SAVType::_GBA:
					GBASAVUtils::SAV = std::make_unique<GBASAV>(SAVUtils::SAVName);
					Good = GBASAVUtils::SAV->GetValid();
					break;

				case SAVType::_NDS:
					NDSSAVUtils::SAV = std::make_unique<NDSSAV>(SAVUtils::SAVName);
					Good = NDSSAVUtils::SAV->GetValid();
					break;

				case SAVType::_NONE:
					return false;
			}


			if (DoBackup && Good) SAVUtils::CreateBackup(BasePath); // Create Backup, if true.
			return Good;
		}

		return false;
	};

	/*
		Load a SAV from a raw buffer.

		std::unique_ptr<uint8_t[]> &Data: The Raw Save Buffer.
		const uint32_t Size: The Save Buffer Size.
		const std::string &BasePath: The base path where to create the Backups (Optional).
		const bool DoBackup: If creating a backup or not after loading the SAVFile (Optional).

		Returns True if the Save is Valid and False if Invalid.
	*/
	bool SAVUtils::LoadSAV(std::unique_ptr<uint8_t[]> &Data, const uint32_t Size, const std::string &BasePath, const bool DoBackup) {
		const SAVType ST = SAVUtils::DetectType(Data, Size);
		bool Good = false;

		if (ST != SAVType::_NONE) {
			SAVUtils::SAV = ST; // Set SAVType.

			/* Load Proper SAV. */
			switch(SAVUtils::SAV) {
				case SAVType::_GBA:
					GBASAVUtils::SAV = std::make_unique<GBASAV>(Data, Size);
					Good = GBASAVUtils::SAV->GetValid();
					break;

				case SAVType::_NDS:
					NDSSAVUtils::SAV = std::make_unique<NDSSAV>(Data, Size);
					Good = NDSSAVUtils::SAV->GetValid();
					break;

				case SAVType::_NONE:
					return false;
			}


			if (DoBackup && Good) SAVUtils::CreateBackup(BasePath); // Create Backup, if true.
			return Good;
		}

		return false;
	};

	/*
		Create a Backup of the current loaded SAV.

		const std::string &BasePath: The base path where to create the Backups.

		Backup Format would be: 'Sims2-Year.Month.Day-Hour.Minute.Second.sav'
	*/
	bool SAVUtils::CreateBackup(const std::string &BasePath) {
		std::string BackupPath = BasePath + "/Backups/"; // Base path.
		bool CreateIt = false;

		/* Fetch Time there. */
		time_t Rawtime;
		struct tm *TimeInfo;
		char TimeBuffer[80];
		time(&Rawtime);
		TimeInfo = localtime(&Rawtime);
		strftime(TimeBuffer, sizeof(TimeBuffer),"%Y.%m.%d-%H.%M.%S", TimeInfo); // Get the Time as String.

		switch(SAVUtils::SAV) {
			case SAVType::_GBA:
				if (GBASAVUtils::SAV && GBASAVUtils::SAV->GetValid()) {
					BackupPath += "GBA/Sims2-" + std::string(TimeBuffer) + ".sav";
					CreateIt = true;
				}
				break;

			case SAVType::_NDS:
				if (NDSSAVUtils::SAV && NDSSAVUtils::SAV->GetValid()) {
					BackupPath += "NDS/Sims2-" + std::string(TimeBuffer) + ".sav";
					CreateIt = true;
				}
				break;

			case SAVType::_NONE:
				break;
		}

		if (CreateIt) {
			FILE *Out = fopen(BackupPath.c_str(), "w");
			fwrite((SAVUtils::SAV == SAVType::_GBA ? GBASAVUtils::SAV->GetData() : NDSSAVUtils::SAV->GetData()), 1, (SAVUtils::SAV == SAVType::_GBA ? GBASAVUtils::SAV->GetSize() : NDSSAVUtils::SAV->GetSize()), Out);
			fclose(Out);
		}

		return CreateIt;
	};


	/*
		Finish SAV Editing and unload everything.
	*/
	void SAVUtils::Finish() {
		if (SAVUtils::SAVName == "" || SAVUtils::SAV == SAVType::_NONE) return;

		/* Ensure for validateness. */
		if ((SAVUtils::SAV == SAVType::_GBA ? GBASAVUtils::SAV->GetValid() : NDSSAVUtils::SAV->GetValid())) {
			/* Ensure that we made changes, otherwise writing is useless. */
			if ((SAVUtils::SAV == SAVType::_GBA ? GBASAVUtils::SAV->GetChangesMade() : NDSSAVUtils::SAV->GetChangesMade())) {
				(SAVUtils::SAV == SAVType::_GBA ? GBASAVUtils::SAV->Finish() : NDSSAVUtils::SAV->Finish()); // The Finish action.

				FILE *Out = fopen(SAVUtils::SAVName.c_str(), "rb+");
				fwrite((SAVUtils::SAV == SAVType::_GBA ? GBASAVUtils::SAV->GetData() : NDSSAVUtils::SAV->GetData()), 1, (SAVUtils::SAV == SAVType::_GBA ? GBASAVUtils::SAV->GetSize() : NDSSAVUtils::SAV->GetSize()), Out);
				fclose(Out);
			}
		}

		/* Now at this point, reset the SAVType + unique_ptr of the SAV. */
		SAVUtils::SAVName = "";
		if (SAVUtils::SAV == SAVType::_GBA) GBASAVUtils::SAV = nullptr;
		if (SAVUtils::SAV == SAVType::_NDS) NDSSAVUtils::SAV = nullptr;
		SAVUtils::SAV = SAVType::_NONE;
	};

	/*
		Return, if changes are made.
	*/
	bool SAVUtils::ChangesMade() {
		switch(SAVUtils::SAV) {
			case S2Editor::SAVType::_GBA:
				if (GBASAVUtils::SAV) return GBASAVUtils::SAV->GetChangesMade();
				break;

			case SAVType::_NDS:
				if (NDSSAVUtils::SAV) return NDSSAVUtils::SAV->GetChangesMade();
				break;

			case SAVType::_NONE:
				return false;
		}

		return false;
	};

	/*
		Read a bit from the SAVData.

		const uint32_t Offs: The Offset where to read from.
		const uint8_t BitIndex: The bit index ( 0 - 7 ).
	*/
	const bool GBASAVUtils::ReadBit(const uint32_t Offs, const uint8_t BitIndex) {
		if (!GBASAVUtils::SAV || !GBASAVUtils::SAV->GetValid() || BitIndex > 0x7) return false;

		return DataHelper::ReadBit(GBASAVUtils::SAV->GetData(), Offs, BitIndex);
	};
	/*
		Write a bit to the SAVData.

		const uint32_t Offs: The Offset where to write to.
		const uint8_t BitIndex: The bit index ( 0 - 7 ).
		const bool IsSet: If the bit is set (1) or not (0).
	*/
	void GBASAVUtils::WriteBit(const uint32_t Offs, const uint8_t BitIndex, const bool IsSet) {
		if (!GBASAVUtils::SAV || !GBASAVUtils::SAV->GetValid() || BitIndex > 0x7) return;

		if (DataHelper::WriteBit(GBASAVUtils::SAV->GetData(), Offs, BitIndex, IsSet)) {
			if (!GBASAVUtils::SAV->GetChangesMade()) GBASAVUtils::SAV->SetChangesMade(true);
		}
	};

	/*
		Read Lower / Upperbits from the SAVBuffer.

		const uint32_t Offs: The Offset where to read from.
		const bool First: If reading from the first 4 bits, or the last 4.
	*/
	const uint8_t GBASAVUtils::ReadBits(const uint32_t Offs, const bool First) {
		if (!GBASAVUtils::SAV || !GBASAVUtils::SAV->GetValid()) return 0;

		return DataHelper::ReadBits(GBASAVUtils::SAV->GetData(), Offs, First);
	};
	/*
		Write Lower / Upperbits to the SAVBuffer.

		const uint32_t Offs: The Offset where to write to.
		const bool First: If writing on the first 4 bits, or the last 4.
		const uint8_t Data: The Data to write.
	*/
	void GBASAVUtils::WriteBits(const uint32_t Offs, const bool First, const uint8_t Data) {
		if (!GBASAVUtils::SAV || !GBASAVUtils::SAV->GetValid() || Data > 0xF) return;

		if (DataHelper::WriteBits(GBASAVUtils::SAV->GetData(), Offs, First, Data)) {
			if (!GBASAVUtils::SAV->GetChangesMade()) GBASAVUtils::SAV->SetChangesMade(true);
		}
	};

	/*
		Read a string from the SAVBuffer.

		const uint32_t Offs: The Offset from where to read from.
		const uint32_t Length: The Length to read.
	*/
	const std::string GBASAVUtils::ReadString(const uint32_t Offs, const uint32_t Length) {
		if (!GBASAVUtils::SAV || !GBASAVUtils::SAV->GetValid()) return "";

		return DataHelper::ReadString(GBASAVUtils::SAV->GetData(), Offs, Length);
	}
	/*
		Write a string to the SAVBuffer.

		const uint32_t Offs: The offset from where to write to.
		const uint32_t Length: The length to write.
		const std::string &Str: The string to write.
	*/
	void GBASAVUtils::WriteString(const uint32_t Offs, const uint32_t Length, const std::string &Str) {
		if (!GBASAVUtils::SAV || !GBASAVUtils::SAV->GetValid()) return;

		if (DataHelper::WriteString(GBASAVUtils::SAV->GetData(), Offs, Length, Str)) {
			if (!GBASAVUtils::SAV->GetChangesMade()) GBASAVUtils::SAV->SetChangesMade(true);
		}
	}


	/*
		Read a bit from the SAVData.

		const uint32_t Offs: The Offset where to read from.
		const uint8_t BitIndex: The bit index ( 0 - 7 ).
	*/
	const bool NDSSAVUtils::ReadBit(const uint32_t Offs, const uint8_t BitIndex) {
		if (!NDSSAVUtils::SAV || !NDSSAVUtils::SAV->GetValid() || BitIndex > 0x7) return false;

		return DataHelper::ReadBit(NDSSAVUtils::SAV->GetData(), Offs, BitIndex);
	};
	/*
		Write a bit to the SAVData.

		const uint32_t Offs: The Offset where to write to.
		const uint8_t BitIndex: The bit index ( 0 - 7 ).
		const bool IsSet: If the bit is set (1) or not (0).
	*/
	void NDSSAVUtils::WriteBit(const uint32_t Offs, const uint8_t BitIndex, const bool IsSet) {
		if (!NDSSAVUtils::SAV || !NDSSAVUtils::SAV->GetValid() || BitIndex > 0x7) return;

		if (DataHelper::WriteBit(NDSSAVUtils::SAV->GetData(), Offs, BitIndex, IsSet)) {
			if (!NDSSAVUtils::SAV->GetChangesMade()) NDSSAVUtils::SAV->SetChangesMade(true);
		}
	};

	/*
		Read Lower / Upperbits from the SAVBuffer.

		const uint32_t Offs: The Offset where to read from.
		const bool First: If reading from the first 4 bits, or the last 4.
	*/
	const uint8_t NDSSAVUtils::ReadBits(const uint32_t Offs, const bool First) {
		if (!NDSSAVUtils::SAV || !NDSSAVUtils::SAV->GetValid()) return 0;

		return DataHelper::ReadBits(NDSSAVUtils::SAV->GetData(), Offs, First);
	};
	/*
		Write Lower / Upperbits to the SAVBuffer.

		const uint32_t Offs: The Offset where to write to.
		const bool First: If writing on the first 4 bits, or the last 4.
		const uint8_t Data: The Data to write.
	*/
	void NDSSAVUtils::WriteBits(const uint32_t Offs, const bool First, const uint8_t Data) {
		if (!NDSSAVUtils::SAV || !NDSSAVUtils::SAV->GetValid() || Data > 0xF) return;

		if (DataHelper::WriteBits(NDSSAVUtils::SAV->GetData(), Offs, First, Data)) {
			if (!NDSSAVUtils::SAV->GetChangesMade()) NDSSAVUtils::SAV->SetChangesMade(true);
		}
	};

	/*
		Read a string from the SAVBuffer.

		const uint32_t Offs: The Offset from where to read from.
		const uint32_t Length: The Length to read.
	*/
	const std::string NDSSAVUtils::ReadString(const uint32_t Offs, const uint32_t Length) {
		if (!NDSSAVUtils::SAV || !NDSSAVUtils::SAV->GetValid()) return "";

		return DataHelper::ReadString(NDSSAVUtils::SAV->GetData(), Offs, Length);
	}
	/*
		Write a string to the SAVBuffer.

		const uint32_t Offs: The offset from where to write to.
		const uint32_t Length: The length to write.
		const std::string &Str: The string to write.
	*/
	void NDSSAVUtils::WriteString(const uint32_t Offs, const uint32_t Length, const std::string &Str) {
		if (!NDSSAVUtils::SAV || !NDSSAVUtils::SAV->GetValid()) return;

		if (DataHelper::WriteString(NDSSAVUtils::SAV->GetData(), Offs, Length, Str)) {
			if (!NDSSAVUtils::SAV->GetChangesMade()) NDSSAVUtils::SAV->SetChangesMade(true);
		}
	}
};