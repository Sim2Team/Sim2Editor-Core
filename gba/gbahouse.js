/*
*   This file is part of Sim2Editor-JSCore
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


import { SavUtils_ReadBits, SavUtils_WriteBits } from "../shared/savutils.js";
import { S2Editor_GBAHouseItem } from "./gbahouseitem.js";


export class S2Editor_GBAHouse {
	constructor(Offs) { this.Offs = Offs; };

	/*
		Get and Set the Room Design.
		Only 0 - 3 SHOULD be used at all, the others aren't actual room designs and instead may cause issues.
	*/
	Roomdesign(V) {
		if (V) SavUtils_WriteBits(this.Offs + 0x2E, true, V);
		else return SavUtils_ReadBits(this.Offs + 0x2E, true);
	};

	/* Get the Items of your House / Room. */
	Items() { return new S2Editor_GBAHouseItem(this.Offs + 0xD6); };
};