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

/* English Strings. */
import S2Editor_GBACasts_EN from './en/gba/casts.js';
import S2Editor_GBAEpisodes_EN from './en/gba/episodes.js';
import S2Editor_GBAItemList_EN from './en/gba/itemlist.js';
import S2Editor_GBAMinigames_EN from './en/gba/minigames.js';
import S2Editor_GBASkillPoints_EN from './en/gba/skillpoints.js';
import S2Editor_GBASocialMoves_EN from './en/gba/socialmoves.js';
import S2Editor_NDSSkillPoints_EN from './en/nds/skillpoints.js';

/* German Strings. */
import S2Editor_GBACasts_DE from './de/gba/casts.js';
import S2Editor_GBAEpisodes_DE from './de/gba/episodes.js';
import S2Editor_GBAMinigames_DE from './de/gba/minigames.js';
import S2Editor_GBASkillPoints_DE from './de/gba/skillpoints.js';
import S2Editor_GBASocialMoves_DE from './de/gba/socialmoves.js';
import S2Editor_NDSSkillPoints_DE from './de/nds/skillpoints.js';

/* Strings which are being initialized and used at the end. */
export let S2Editor_GBACasts, S2Editor_GBAEpisodes, S2Editor_GBAItemList, S2Editor_GBAMinigames,
			S2Editor_GBASkillPoints, S2Editor_GBASocialMoves, S2Editor_NDSSkillPoints;

/*
	Load the Strings for the use with the SAVEditor.

	SAVType: The SAVType which strings to load. (0 -> GBA, 1 -> NDS).
	Language: The language index to load (It uses the GBASettings Language index right now for both, GBA and NDS).

	After that, use the exported variables listed above this comment.
*/
export function S2Editor_LoadStrings(SAVType, Language) {
	/* 0 --> GBA. */
	if (SAVType == 0) {
		switch(Language) {
			default: // English.
			case 0:
				S2Editor_GBACasts = S2Editor_GBACasts_EN;
				S2Editor_GBAEpisodes = S2Editor_GBAEpisodes_EN;
				S2Editor_GBAItemList = S2Editor_GBAItemList_EN;
				S2Editor_GBAMinigames = S2Editor_GBAMinigames_EN;
				S2Editor_GBASkillPoints = S2Editor_GBASkillPoints_EN;
				S2Editor_GBASocialMoves = S2Editor_GBASocialMoves_EN;
				break;

			case 3: // German.
				S2Editor_GBACasts = S2Editor_GBACasts_DE;
				S2Editor_GBAEpisodes = S2Editor_GBAEpisodes_DE;
				S2Editor_GBAItemList = S2Editor_GBAItemList_EN; // Not available in German right now.
				S2Editor_GBAMinigames = S2Editor_GBAMinigames_DE;
				S2Editor_GBASkillPoints = S2Editor_GBASkillPoints_DE;
				S2Editor_GBASocialMoves = S2Editor_GBASocialMoves_DE;
				break;
		}

	/* 1 --> NDS. */
	} else if (SAVType == 1) {
		switch(Language) {
			default: // English.
			case 0:
				S2Editor_NDSSkillPoints = S2Editor_NDSSkillPoints_EN;
				break;

			case 3: // German.
				S2Editor_NDSSkillPoints = S2Editor_NDSSkillPoints_DE;
				break;
		}
	}
};

/*
	Return an GBA Item's name.

	ID: The Item ID from 0 up to 255.
*/
export function S2Editor_GetGBAItemName(ID) {
	if (ID < 256 && S2Editor_GBAItemList) return S2Editor_GBAItemList[ID];
	else return '?';
};