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

import { SAVUtils_Read, SAVUtils_Write } from '../shared/savutils.js';

const EPOffs = [ 0x104, 0x10E, 0x122, 0x11D, 0x131, 0x127, 0x14A, 0x140, 0x118, 0x16D, 0x168 ]; // 11 Episodes.

export class S2Editor_GBAEpisode {
	constructor(Slot, Episode, Move = 0x0) {
		this.Episode = Episode;
		this.Offs = ((Slot * 0x1000) + this.SetOffset(Math.min(10, Move)));
	};

	/* Get Episode Index. */
	Index() { return this.Episode; };

	/* Get and Set Episode Ratings. */
	Rating(Category, V) {
		if (V) SAVUtils_Write("uint8_t", this.Offs + Math.min(3, Category), Math.min(25, V));
		else return SAVUtils_Read("uint8_t", this.Offs + Math.min(3, Category));
	};

	/* Get and Set the Unlocked State. */
	State(V) {
		if (V) SAVUtils_Write("uint8_t", this.Offs + 0x4, Math.min(1, V));
		else return SAVUtils_Read("uint8_t", this.Offs + 0x4);
	};

	/* Sets the base offset for the Episodes. */
	SetOffset(Move) { return EPOffs[this.Episode] + (Move * 0x6); };
};