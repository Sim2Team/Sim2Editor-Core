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

#ifndef _SIM2EDITOR_CPP_CORE_SAV_UTILS_HPP
#define _SIM2EDITOR_CPP_CORE_SAV_UTILS_HPP

#include "DataHelper.hpp"
#include "../gba/GBASav.hpp"
#include "../nds/NDSSav.hpp"


namespace S2Editor {
	/*
		SavUtils for common things.

		Used for SavType Detection and various other common things.
	*/
	namespace SavUtils {
		extern SavType Sav; // Active SavType.
		extern std::string SavName;

		SavType DetectType(const std::string &File);
		SavType DetectType(const std::unique_ptr<uint8_t[]> &Data, const uint32_t Size);
		bool LoadSav(const std::string &File, const std::string &BasePath = "", const bool DoBackup = false);
		bool LoadSav(std::unique_ptr<uint8_t[]> &Data, const uint32_t Size, const std::string &BasePath = "", const bool DoBackup = false);
		bool CreateBackup(const std::string &BasePath);
		void Finish();
		bool ChangesMade();
	};


	/* SavUtils for GBA. */
	namespace GBASavUtils {
		extern std::unique_ptr<GBASav> Sav;

		/*
			Read from the SavBuffer.

			const uint32_t Offs: The Offset from where to read.
		*/
		template <typename T>
		T Read(const uint32_t Offs) {
			if (!GBASavUtils::Sav || !GBASavUtils::Sav->GetValid() || !GBASavUtils::Sav->GetData()) return 0;
			return DataHelper::Read<T>(GBASavUtils::Sav->GetData(), Offs);
		};

		/*
			Write to the SavBuffer.

			const uint32_t Offs: The Offset where to write to.
			T Data: The data which to write.
		*/
		template <typename T>
		void Write(const uint32_t Offs, T Data) {
			if (!GBASavUtils::Sav || !GBASavUtils::Sav->GetValid()) return;

			if (DataHelper::Write<T>(GBASavUtils::Sav->GetData(), Offs, Data)) {
				if (!GBASavUtils::Sav->GetChangesMade()) GBASavUtils::Sav->SetChangesMade(true);
			}
		};

		/* BIT stuff. */
		const bool ReadBit(const uint32_t Offs, const uint8_t BitIndex);
		void WriteBit(const uint32_t Offs, const uint8_t BitIndex, const bool IsSet);
		const uint8_t ReadBits(const uint32_t Offs, const bool First = true);
		void WriteBits(const uint32_t Offs, const bool First = true, const uint8_t Data = 0x0);

		const std::string ReadString(const uint32_t Offs, const uint32_t Length);
		void WriteString(const uint32_t Offs, const uint32_t Length, const std::string &Str);
	};

	/* SavUtils for NDS. */
	namespace NDSSavUtils {
		extern std::unique_ptr<NDSSav> Sav;

		/*
			Read from the SavBuffer.

			const uint32_t Offs: The Offset from where to read.
		*/
		template <typename T>
		T Read(const uint32_t Offs) {
			if (!NDSSavUtils::Sav || !NDSSavUtils::Sav->GetValid() || !NDSSavUtils::Sav->GetData()) return 0;
			return DataHelper::Read<T>(NDSSavUtils::Sav->GetData(), Offs);
		};

		/*
			Write to the SavBuffer.

			const uint32_t Offs: The Offset where to write to.
			T Data: The data which to write.
		*/
		template <typename T>
		void Write(const uint32_t Offs, T Data) {
			if (!NDSSavUtils::Sav || !NDSSavUtils::Sav->GetValid()) return;

			if (DataHelper::Write<T>(NDSSavUtils::Sav->GetData(), Offs, Data)) {
				if (!NDSSavUtils::Sav->GetChangesMade()) NDSSavUtils::Sav->SetChangesMade(true);
			}
		};

		/* BIT stuff. */
		const bool ReadBit(const uint32_t Offs, const uint8_t BitIndex);
		void WriteBit(const uint32_t Offs, const uint8_t BitIndex, const bool IsSet);
		const uint8_t ReadBits(const uint32_t Offs, const bool First = true);
		void WriteBits(const uint32_t Offs, const bool First = true, const uint8_t Data = 0x0);

		const std::string ReadString(const uint32_t Offs, const uint32_t Length);
		void WriteString(const uint32_t Offs, const uint32_t Length, const std::string &Str);

		NDSSavRegion GetRegion();
	};
};

#endif