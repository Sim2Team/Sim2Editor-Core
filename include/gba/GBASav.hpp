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

#ifndef _SIM2EDITOR_CPP_CORE_GBA_SAV_HPP
#define _SIM2EDITOR_CPP_CORE_GBA_SAV_HPP

#include "GBASettings.hpp"
#include "GBASlot.hpp"

namespace S2Editor {
	class GBASAV {
	public:
		GBASAV(const std::string &SAVFile);
		void ValidationCheck();

		bool SlotExist(const uint8_t Slot);

		/* Core Returns and Actions. */
		std::unique_ptr<GBASlot> Slot(const uint8_t Slot);
		std::unique_ptr<GBASettings> Settings() const;
		void Finish();

		/* Some Returns. */
		uint32_t GetSize() const { return this->SavSize; };
		uint8_t *GetData() const { return this->SavData.get(); };
		bool GetValid() const { return this->SavValid; };
		bool GetChangesMade() const { return this->ChangesMade; };
		void SetChangesMade(const bool V) { this->ChangesMade = V; };
	private:
		std::unique_ptr<uint8_t[]> SavData = nullptr;
		uint32_t SavSize = 0;
		bool SavValid = false, ChangesMade = false;
		static constexpr uint8_t GBAIdent[7] = { 0x53, 0x54, 0x57, 0x4E, 0x30, 0x32, 0x34 };
	};
};

#endif