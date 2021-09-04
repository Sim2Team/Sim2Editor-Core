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

#include "SavUtils.hpp"
#include <ctime>
#include <unistd.h>


/* Common Related things. */
static constexpr uint8_t GBAIdent[7] = { 0x53, 0x54, 0x57, 0x4E, 0x30, 0x32, 0x34 };
static constexpr uint8_t NDSIdent[8] = { 0x64, 0x61, 0x74, 0x0, 0x20, 0x0, 0x0, 0x0 };

namespace S2Editor {
	SavType SavUtils::Sav = SavType::_NONE;
	std::string SavUtils::SavName = "";

	/* GBA Related things. */
	std::unique_ptr<GBASav> GBASavUtils::Sav = nullptr;

	/* NDS Related things. */
	std::unique_ptr<NDSSav> NDSSavUtils::Sav = nullptr;


	/*
		Detect the SavType of a SavFile.

		const std::string &File: Path to the file which to check.
	*/
	SavType SavUtils::DetectType(const std::string &File) {
		SavType ST = SavType::_NONE;

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

					if (Count == 7) ST = SavType::_GBA; // If Count matches 7, we're good.
					break;

				case 0x40000:
				case 0x80000: // 256, 512 KB is a NDS Size.
					Data = std::make_unique<uint8_t[]>(Size);
					fread(Data.get(), 1, Size, In);

					for (uint8_t Slot = 0; Slot < 5; Slot++) { // Check for all 5 possible Slots.
						Count = 0; // Reset Count here.

						for (uint8_t ID = 0; ID < 8; ID++) {
							if (ID == 0x4) {
								for (uint8_t Reg = 0; Reg < 3; Reg++) {
									if (Data.get()[(Slot * 0x1000) + ID] == NDSIdent[ID] + Reg) {
										Count++;
										break;
									}
								}
							
							} else {
								if (Data.get()[(Slot * 0x1000) + ID] == NDSIdent[ID]) Count++;
							}
						} // Identifier Check.

						if (Count == 8) {
							ST = SavType::_NDS; // It's a NDS Sav.
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
		Detect the SavType of a SavFile from rawdata.

		const std::unique_ptr<uint8_t[]> &Data: The raw Save data to check.
		const uint32_t Size: The Size of the raw buffer.
	*/
	SavType SavUtils::DetectType(const std::unique_ptr<uint8_t[]> &Data, const uint32_t Size) {
		if (!Data) return SavType::_NONE;
		uint8_t Count = 0;

		switch(Size) {
			case 0x10000:
			case 0x20000: // 64, 128 KB is a GBA Size.
				for (uint8_t ID = 0; ID < 7; ID++) {
					if (Data.get()[ID] == GBAIdent[ID]) Count++;
				} // Identifier Check.

				if (Count == 7) return SavType::_GBA; // If Count matches 7, we're good.
				break;

			case 0x40000:
			case 0x80000: // 256, 512 KB is a NDS Size.
				for (uint8_t Slot = 0; Slot < 5; Slot++) { // Check for all 5 possible Slots.
					Count = 0; // Reset Count here.

					for (uint8_t ID = 0; ID < 8; ID++) {
						if (ID == 0x4) {
							for (uint8_t Reg = 0; Reg < 3; Reg++) {
								if (Data.get()[(Slot * 0x1000) + ID] == NDSIdent[ID] + Reg) {
									Count++;
									break;
								}
							}
							
						} else {
							if (Data.get()[(Slot * 0x1000) + ID] == NDSIdent[ID]) Count++;
						}
					} // Identifier Check.

					if (Count == 8) return SavType::_NDS; // It's a NDS Sav.
				}

				break;
		}

		return SavType::_NONE;
	};


	/*
		Load a SavFile.

		const std::string &File: Path to the SavFile.
		const std::string &BasePath: The base path where to create the Backups (Optional).
		const bool DoBackup: If creating a backup or not after loading the SavFile (Optional).

		Returns True if the Save is Valid and False if Invalid.
	*/
	bool SavUtils::LoadSav(const std::string &File, const std::string &BasePath, const bool DoBackup) {
		const SavType ST = SavUtils::DetectType(File);
		bool Good = false;

		if (ST != SavType::_NONE) {
			SavUtils::Sav = ST; // Set SavType.
			SavUtils::SavName = File; // Set Path.

			/* Load Proper Sav. */
			switch(SavUtils::Sav) {
				case SavType::_GBA:
					GBASavUtils::Sav = std::make_unique<GBASav>(SavUtils::SavName);
					Good = GBASavUtils::Sav->GetValid();
					break;

				case SavType::_NDS:
					NDSSavUtils::Sav = std::make_unique<NDSSav>(SavUtils::SavName);
					Good = NDSSavUtils::Sav->GetValid();
					break;

				case SavType::_NONE:
					return false;
			}


			if (DoBackup && Good) SavUtils::CreateBackup(BasePath); // Create Backup, if true.
			return Good;
		}

		return false;
	};

	/*
		Load a Sav from a raw buffer.

		std::unique_ptr<uint8_t[]> &Data: The Raw Save Buffer.
		const uint32_t Size: The Save Buffer Size.
		const std::string &BasePath: The base path where to create the Backups (Optional).
		const bool DoBackup: If creating a backup or not after loading the SavFile (Optional).

		Returns True if the Save is Valid and False if Invalid.
	*/
	bool SavUtils::LoadSav(std::unique_ptr<uint8_t[]> &Data, const uint32_t Size, const std::string &BasePath, const bool DoBackup) {
		const SavType ST = SavUtils::DetectType(Data, Size);
		bool Good = false;

		if (ST != SavType::_NONE) {
			SavUtils::Sav = ST; // Set SavType.

			/* Load Proper Sav. */
			switch(SavUtils::Sav) {
				case SavType::_GBA:
					GBASavUtils::Sav = std::make_unique<GBASav>(Data, Size);
					Good = GBASavUtils::Sav->GetValid();
					break;

				case SavType::_NDS:
					NDSSavUtils::Sav = std::make_unique<NDSSav>(Data, Size);
					Good = NDSSavUtils::Sav->GetValid();
					break;

				case SavType::_NONE:
					return false;
			}


			if (DoBackup && Good) SavUtils::CreateBackup(BasePath); // Create Backup, if true.
			return Good;
		}

		return false;
	};

	/*
		Create a Backup of the current loaded Sav.

		const std::string &BasePath: The base path where to create the Backups.

		Backup Format would be: 'Sims2-Year.Month.Day-Hour.Minute.Second.Sav'
	*/
	bool SavUtils::CreateBackup(const std::string &BasePath) {
		std::string BackupPath = BasePath + "/Backups/"; // Base path.
		bool CreateIt = false;

		/* Fetch Time there. */
		time_t Rawtime;
		struct tm *TimeInfo;
		char TimeBuffer[80];
		time(&Rawtime);
		TimeInfo = localtime(&Rawtime);
		strftime(TimeBuffer, sizeof(TimeBuffer),"%Y.%m.%d-%H.%M.%S", TimeInfo); // Get the Time as String.

		switch(SavUtils::Sav) {
			case SavType::_GBA:
				if (GBASavUtils::Sav && GBASavUtils::Sav->GetValid()) {
					BackupPath += "GBA/Sims2-" + std::string(TimeBuffer) + ".Sav";
					CreateIt = true;
				}
				break;

			case SavType::_NDS:
				if (NDSSavUtils::Sav && NDSSavUtils::Sav->GetValid()) {
					BackupPath += "NDS/Sims2-" + std::string(TimeBuffer) + ".Sav";
					CreateIt = true;
				}
				break;

			case SavType::_NONE:
				break;
		}

		if (CreateIt) {
			FILE *Out = fopen(BackupPath.c_str(), "w");
			fwrite((SavUtils::Sav == SavType::_GBA ? GBASavUtils::Sav->GetData() : NDSSavUtils::Sav->GetData()), 1, (SavUtils::Sav == SavType::_GBA ? GBASavUtils::Sav->GetSize() : NDSSavUtils::Sav->GetSize()), Out);
			fclose(Out);
		}

		return CreateIt;
	};


	/*
		Finish Sav Editing and unload everything.
	*/
	void SavUtils::Finish() {
		if (SavUtils::SavName == "" || SavUtils::Sav == SavType::_NONE) return;

		/* Ensure for validateness. */
		if ((SavUtils::Sav == SavType::_GBA ? GBASavUtils::Sav->GetValid() : NDSSavUtils::Sav->GetValid())) {
			/* Ensure that we made changes, otherwise writing is useless. */
			if ((SavUtils::Sav == SavType::_GBA ? GBASavUtils::Sav->GetChangesMade() : NDSSavUtils::Sav->GetChangesMade())) {
				(SavUtils::Sav == SavType::_GBA ? GBASavUtils::Sav->Finish() : NDSSavUtils::Sav->Finish()); // The Finish action.

				FILE *Out = fopen(SavUtils::SavName.c_str(), "rb+");
				fwrite((SavUtils::Sav == SavType::_GBA ? GBASavUtils::Sav->GetData() : NDSSavUtils::Sav->GetData()), 1, (SavUtils::Sav == SavType::_GBA ? GBASavUtils::Sav->GetSize() : NDSSavUtils::Sav->GetSize()), Out);
				fclose(Out);
			}
		}

		/* Now at this point, reset the SavType + unique_ptr of the Sav. */
		SavUtils::SavName = "";
		if (SavUtils::Sav == SavType::_GBA) GBASavUtils::Sav = nullptr;
		if (SavUtils::Sav == SavType::_NDS) NDSSavUtils::Sav = nullptr;
		SavUtils::Sav = SavType::_NONE;
	};

	/*
		Return, if changes are made.
	*/
	bool SavUtils::ChangesMade() {
		switch(SavUtils::Sav) {
			case S2Editor::SavType::_GBA:
				if (GBASavUtils::Sav) return GBASavUtils::Sav->GetChangesMade();
				break;

			case SavType::_NDS:
				if (NDSSavUtils::Sav) return NDSSavUtils::Sav->GetChangesMade();
				break;

			case SavType::_NONE:
				return false;
		}

		return false;
	};

	/*
		Read a bit from the SavData.

		const uint32_t Offs: The Offset where to read from.
		const uint8_t BitIndex: The bit index ( 0 - 7 ).
	*/
	const bool GBASavUtils::ReadBit(const uint32_t Offs, const uint8_t BitIndex) {
		if (!GBASavUtils::Sav || !GBASavUtils::Sav->GetValid() || BitIndex > 0x7) return false;

		return DataHelper::ReadBit(GBASavUtils::Sav->GetData(), Offs, BitIndex);
	};
	/*
		Write a bit to the SavData.

		const uint32_t Offs: The Offset where to write to.
		const uint8_t BitIndex: The bit index ( 0 - 7 ).
		const bool IsSet: If the bit is set (1) or not (0).
	*/
	void GBASavUtils::WriteBit(const uint32_t Offs, const uint8_t BitIndex, const bool IsSet) {
		if (!GBASavUtils::Sav || !GBASavUtils::Sav->GetValid() || BitIndex > 0x7) return;

		if (DataHelper::WriteBit(GBASavUtils::Sav->GetData(), Offs, BitIndex, IsSet)) {
			if (!GBASavUtils::Sav->GetChangesMade()) GBASavUtils::Sav->SetChangesMade(true);
		}
	};

	/*
		Read Lower / Upperbits from the SavBuffer.

		const uint32_t Offs: The Offset where to read from.
		const bool First: If reading from the first 4 bits, or the last 4.
	*/
	const uint8_t GBASavUtils::ReadBits(const uint32_t Offs, const bool First) {
		if (!GBASavUtils::Sav || !GBASavUtils::Sav->GetValid()) return 0;

		return DataHelper::ReadBits(GBASavUtils::Sav->GetData(), Offs, First);
	};
	/*
		Write Lower / Upperbits to the SavBuffer.

		const uint32_t Offs: The Offset where to write to.
		const bool First: If writing on the first 4 bits, or the last 4.
		const uint8_t Data: The Data to write.
	*/
	void GBASavUtils::WriteBits(const uint32_t Offs, const bool First, const uint8_t Data) {
		if (!GBASavUtils::Sav || !GBASavUtils::Sav->GetValid() || Data > 0xF) return;

		if (DataHelper::WriteBits(GBASavUtils::Sav->GetData(), Offs, First, Data)) {
			if (!GBASavUtils::Sav->GetChangesMade()) GBASavUtils::Sav->SetChangesMade(true);
		}
	};

	/*
		Read a string from the SavBuffer.

		const uint32_t Offs: The Offset from where to read from.
		const uint32_t Length: The Length to read.
	*/
	const std::string GBASavUtils::ReadString(const uint32_t Offs, const uint32_t Length) {
		if (!GBASavUtils::Sav || !GBASavUtils::Sav->GetValid()) return "";

		return DataHelper::ReadString(GBASavUtils::Sav->GetData(), Offs, Length);
	};
	/*
		Write a string to the SavBuffer.

		const uint32_t Offs: The offset from where to write to.
		const uint32_t Length: The length to write.
		const std::string &Str: The string to write.
	*/
	void GBASavUtils::WriteString(const uint32_t Offs, const uint32_t Length, const std::string &Str) {
		if (!GBASavUtils::Sav || !GBASavUtils::Sav->GetValid()) return;

		if (DataHelper::WriteString(GBASavUtils::Sav->GetData(), Offs, Length, Str)) {
			if (!GBASavUtils::Sav->GetChangesMade()) GBASavUtils::Sav->SetChangesMade(true);
		}
	};


	/*
		Read a bit from the SavData.

		const uint32_t Offs: The Offset where to read from.
		const uint8_t BitIndex: The bit index ( 0 - 7 ).
	*/
	const bool NDSSavUtils::ReadBit(const uint32_t Offs, const uint8_t BitIndex) {
		if (!NDSSavUtils::Sav || !NDSSavUtils::Sav->GetValid() || BitIndex > 0x7) return false;

		return DataHelper::ReadBit(NDSSavUtils::Sav->GetData(), Offs, BitIndex);
	};
	/*
		Write a bit to the SavData.

		const uint32_t Offs: The Offset where to write to.
		const uint8_t BitIndex: The bit index ( 0 - 7 ).
		const bool IsSet: If the bit is set (1) or not (0).
	*/
	void NDSSavUtils::WriteBit(const uint32_t Offs, const uint8_t BitIndex, const bool IsSet) {
		if (!NDSSavUtils::Sav || !NDSSavUtils::Sav->GetValid() || BitIndex > 0x7) return;

		if (DataHelper::WriteBit(NDSSavUtils::Sav->GetData(), Offs, BitIndex, IsSet)) {
			if (!NDSSavUtils::Sav->GetChangesMade()) NDSSavUtils::Sav->SetChangesMade(true);
		}
	};

	/*
		Read Lower / Upperbits from the SavBuffer.

		const uint32_t Offs: The Offset where to read from.
		const bool First: If reading from the first 4 bits, or the last 4.
	*/
	const uint8_t NDSSavUtils::ReadBits(const uint32_t Offs, const bool First) {
		if (!NDSSavUtils::Sav || !NDSSavUtils::Sav->GetValid()) return 0;

		return DataHelper::ReadBits(NDSSavUtils::Sav->GetData(), Offs, First);
	};
	/*
		Write Lower / Upperbits to the SavBuffer.

		const uint32_t Offs: The Offset where to write to.
		const bool First: If writing on the first 4 bits, or the last 4.
		const uint8_t Data: The Data to write.
	*/
	void NDSSavUtils::WriteBits(const uint32_t Offs, const bool First, const uint8_t Data) {
		if (!NDSSavUtils::Sav || !NDSSavUtils::Sav->GetValid() || Data > 0xF) return;

		if (DataHelper::WriteBits(NDSSavUtils::Sav->GetData(), Offs, First, Data)) {
			if (!NDSSavUtils::Sav->GetChangesMade()) NDSSavUtils::Sav->SetChangesMade(true);
		}
	};

	/*
		Read a string from the SavBuffer.

		const uint32_t Offs: The Offset from where to read from.
		const uint32_t Length: The Length to read.
	*/
	const std::string NDSSavUtils::ReadString(const uint32_t Offs, const uint32_t Length) {
		if (!NDSSavUtils::Sav || !NDSSavUtils::Sav->GetValid()) return "";

		return DataHelper::ReadString(NDSSavUtils::Sav->GetData(), Offs, Length);
	};
	/*
		Write a string to the SavBuffer.

		const uint32_t Offs: The offset from where to write to.
		const uint32_t Length: The length to write.
		const std::string &Str: The string to write.
	*/
	void NDSSavUtils::WriteString(const uint32_t Offs, const uint32_t Length, const std::string &Str) {
		if (!NDSSavUtils::Sav || !NDSSavUtils::Sav->GetValid()) return;

		if (DataHelper::WriteString(NDSSavUtils::Sav->GetData(), Offs, Length, Str)) {
			if (!NDSSavUtils::Sav->GetChangesMade()) NDSSavUtils::Sav->SetChangesMade(true);
		}
	};

	/*
		Return the detected Save Region from the Savefile.
	*/
	NDSSavRegion NDSSavUtils::GetRegion() {
		if (!NDSSavUtils::Sav || !NDSSavUtils::Sav->GetValid()) return NDSSavRegion::Unknown;

		return NDSSavUtils::Sav->GetRegion();
	};
};