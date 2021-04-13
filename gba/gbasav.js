/*
*   This file is part of Sim2Editor-JSCore
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

import { SAVUtils_Read, SAVUtils_Write } from '../shared/savutils.js';
import { S2Editor_GBASlot } from './gbaslot.js';
import { S2Editor_GBASettings } from './gbasettings.js';

export class S2Editor_GBASAV {
	constructor() {
		this.ChangesMade = false;
		this.SAVValid = true; // TODO: Maybe make a use of this? No idea.

		/* Language Index is 6 or larger, which is "blank" and can break the game. */
		if (SAVUtils_Read("uint8_t", 0xA) > 5) SAVUtils_Write("uint8_t", 0xA, 0); // English.
	};

	/*
		Return, wheter a Slot is valid / exist.

		Slot: The Slot to check.
	*/
	SlotExist(Slot) {
		if (Slot < 1 || Slot > 4 || !this.GetValid()) return false;

		for (let Idx = 0; Idx < 10; Idx++) {
			if (SAVUtils_Read("uint8_t", (Slot * 0x1000) + Idx) != 0) return true;
		}

		return false;
	};

	/*
		Return a GBASlot class.

		Slot: The GBASAV Slot ( 1 - 4 ).
	*/
	Slot(Slot) {
		if (!this.SlotExist(Slot)) return undefined;

		return new S2Editor_GBASlot(Slot);
	};

	/* Get a Settings class. */
	Settings() { return new S2Editor_GBASettings(); };

	/*
		Finish call before writting to file.

		Fix the Checksum of all existing Slots and the Settings (TODO), if invalid.
	*/
	Finish() {
		if (!this.GetValid()) return;

		for (let Slot = 1; Slot < 5; Slot++) {
			if (this.SlotExist(Slot)) this.Slot(Slot).FixChecksum();
		}

		/* Do the same with the Settings. */
		this.Settings().UpdateChecksum();
	};

	/* Return if the SAV is valid. */
	GetValid() { return this.SAVValid; };

	/* Get and Set if changes made. */
	GetChangesMade() { return this.ChangesMade; };
	SetChangesMade(V) { this.ChangesMade = V; };
};