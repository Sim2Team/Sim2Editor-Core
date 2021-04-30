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

import { SAVUtils_Read, SAVUtils_ReadBit, SAVUtils_Write, SAVUtils_WriteBit, SAVUtils_WriteBits } from '../shared/savutils.js';

export class S2Editor_GBAMinigame {
	constructor(Offs, Game) {
		this.Offs = Offs;
		this.Game = Game;
	};

	/* Get and Set if you played that game already today. */
	Played(V) {
		if (V) {
			SAVUtils_WriteBit(this.Offs, this.Game, V);

		} else {
			return SAVUtils_ReadBit(this.Offs, this.Game);
		}
	};

	/* Get and Set the Minigame Level. */
	Level(V, Meta) {
		if (V) {
			SAVUtils_Write("uint8_t", this.Offs + 0x24 + this.Game, Math.min(5, V));

			/* Optionally: Set to Metadata / Settings as well. */
			if (Meta) SAVUtils_WriteBits(0x10 + (this.Game / 2), ((this.Game % 2) == 0), Math.min(5, V));

		} else {
			SAVUtils_Read("uint8_t", this.Offs + 0x24 + this.Game);
		}
	};
};