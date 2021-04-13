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

export const GBAIdent = [ 0x53, 0x54, 0x57, 0x4E, 0x30, 0x32, 0x34 ]; // GBA Header Identifier.
export const NDSIdent = [ 0x64, 0x61, 0x74, 0x0, 0x20, 0x0, 0x0, 0x0 ]; // NDSSlot Header Identifier.
export let SAV, SAVName, SAVBuffer, SAVData, SAVSize, SAVType; // SAV Variables.

/* Import SAV classes. */
import { S2Editor_GBASAV } from '../gba/gbasav.js';
import { S2Editor_NDSSAV } from '../nds/ndssav.js';

/* Character Whitelist. */
const SAVUtils_CharWhiteList = [
	/* UPPERCASE characters. */
	'A', 'B', 'C', 'D', 'E', 'F', 'G',
	'H', 'I', 'J', 'K', 'L', 'M',
	'N', 'O', 'P', 'Q', 'R', 'S',
	'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	/* UPPERCASE Numbers. */
	'0', '1', '2', '3', '4',
	'5', '6', '7', '8', '9',
	/* LOWERCASE characters. */
	'a', 'b', 'c', 'd', 'e', 'f', 'g',
	'h', 'i', 'j', 'k', 'l', 'm',
	'n', 'o', 'p', 'q', 'r', 's',
	't', 'u', 'v', 'w', 'x', 'y', 'z',
	/* LOWERCASE Special signs. */
	'!', '+', '#', '$', '%',
	'&', '_', '*', '(', ')'
];

/*
	Detect the SAVType of a SAVFile.

	Data: The DataView of the SAVData.
	Size: The size of the SAVFile.

	Returns -1 for Invalid, 0 for GBA and 1 for NDS.
*/
export function SAVUtils_DetectType(Data, Size) {
	if (!Data) {
		console.log("No Data provided.");
		return -1;
	}

	let Count = 0;

	/* Checking SAVType here. */
	switch(Size) {
		case 0x10000:
		case 0x20000: // 64, 128 KB is a GBA Size.
			for (let ID = 0; ID < 7; ID++) { if (Data.getUint8(ID) == GBAIdent[ID]) Count++; };
			if (Count == 7) return 0;
			else return -1;

		case 0x40000:
		case 0x80000: // 256, 512 KB is a NDS Size.
			for (let Slot = 0; Slot < 5; Slot++) { // Check for all 5 possible Slots.
				Count = 0; // Reset Count here.

				for (let ID = 0; ID < 8; ID++) { if (Data.getUint8((Slot * 0x1000) + ID) == NDSIdent[ID]) Count++; };

				if (Count == 8) return 1;
			}

			return -1; // There were no 8 Count matches on all 5 Slots, hence invalid.

		default:
			return -1;
	}
};

/*
	Load the SAV.

	SAVFile: SAVFile.
	LoadCallback: The Action that happens, after the SAVFile is valid and has been read (Like the Menu Handle).

	returns -1 for invalid, 0 for GBA, 1 for NDS.
*/
export function SAVUtils_LoadSAV(SAVFile, LoadCallback) {
	if (!SAVFile) {
		alert("No SAVFile selected.");
		return -1;
	}

	SAVName = SAVFile.name;
	SAVSize = SAVFile.size;

	let Reader = new FileReader();
	Reader.readAsArrayBuffer(SAVFile);

	Reader.onload = function() {
		SAVBuffer = new Uint8Array(this.result);
		SAVData = new DataView(SAVBuffer.buffer);
		SAVType = SAVUtils_DetectType(SAVData, SAVSize); // Detect SAVType.

		switch(SAVType) {
			case 0:
				SAV = new S2Editor_GBASAV(); // We are using a GBA SAV.
				break;

			case 1:
				SAV = new S2Editor_NDSSAV(); // We are using a NDS SAV.
				break;

			default:
				SAV = undefined; // Invalid SAV.
				break;
		}

		if (SAV != undefined) LoadCallback();
	};
};

/*
	Return, if changes have been made.

	Returns false or true, depending on changes made status.
*/
export function SAVUtils_ChangesMade() {
	switch(SAVType) {
		case 0: // GBA.
		case 1: // NDS.
			return SAV.GetChangesMade();

		default: // NONE.
			return false;
	}
};

/*
	Read something from the SAVData.

	Type: The type to read.
	Offs: Where to read from.

	I used this style, because it seems similar at some point to the C++ version.
*/
export function SAVUtils_Read(Type, Offs) {
	if (SAVType == -1) return 0; // -1 -> Invalid.

	switch(Type) {
		case "uint8_t":
		case "u8":
			return SAVData.getUint8(Offs);

		case "uint16_t":
		case "u16":
			return SAVData.getUint16(Offs, true);

		case "uint32_t":
		case "u32":
			return SAVData.getUint32(Offs, true);

		default:
			return 0;
	}
};

/*
	Write something to the SAVData.

	Type: The type to write.
	Offs: Where to write to.
	Data: What to write.

	I used this style, because it seems similar at some point to the C++ version.
*/
export function SAVUtils_Write(Type, Offs, Data) {
	if (SAVType == -1 || SAVData == undefined) return; // -1 -> Invalid.

	switch(Type) {
		case "uint8_t":
		case "u8":
			SAVData.setUint8(Offs, Math.min(0xFF, Data));
			if (!SAV.GetChangesMade()) SAV.SetChangesMade(true);
			break;

		case "uint16_t":
		case "u16":
			SAVData.setUint16(Offs, Math.min(0xFFFF, Data), true);
			if (!SAV.GetChangesMade()) SAV.SetChangesMade(true);
			break;

		case "uint32_t":
		case "u32":
			SAVData.setUint32(Offs, Math.min(0xFFFFFFFF, Data), true);
			if (!SAV.GetChangesMade()) SAV.SetChangesMade(true);
			break;
	}
};

/*
	Read Lower / Upper Bits.

	Offs: The offset where to read from.
	First: If Reading from the first four bits, or second.
*/
export function SAVUtils_ReadBits(Offs, First) {
	if (SAVData == undefined || SAVType == -1) return 0x0;

	if (First) return (SAVData.getUint8(Offs) & 0xF); // Bit 0 - 3.
	else return (SAVData.getUint8(Offs) >> 4); // Bit 4 - 7.
};

/*
	Write Lower / Upper Bits.

	Offs: The offset where to write to.
	First: If Writing on the first four bits, or second.
	Data: The Data to write.
*/
export function SAVUtils_WriteBits(Offs, First, Data) {
	if (Data > 0xF || SAVData == undefined || SAVType == -1) return;

	if (First) SAVUtils_Write("uint8_t", Offs, (SAVData.getUint8(Offs) & 0xF0) | (Data & 0xF)); // Bit 0 - 3.
	else SAVUtils_Write("uint8_t", Offs, (SAVData.getUint8(Offs) & 0x0F) | (Data << 4)); // Bit 4 - 7.
};


/*
	Read a String from the SAVData.

	Offs: Where to read from.
	Length: The length to read.
*/
export function SAVUtils_ReadString(Offs, Length) {
	if (SAVType == -1 || SAVData == undefined) return ""; // -1 -> Invalid.
	let STR = '';

	for (let i = 0; i < Length; i++) {
		if (SAVData.getUint8(Offs + i) == 0x0) break; // Do not continue to read.

		STR += String.fromCharCode(SAVData.getUint8(Offs + i));
	}

	return STR;
};

/*
	Write a String to the SAVData.

	Offs: Where to write to.
	Length: The length to write.
	STR: What to write.
*/
export function SAVUtils_WriteString(Offs, Length, STR) {
	if (SAVType == -1 || STR == undefined || SAVData == undefined) return;
	let Index = 0, Filler = false;

	while(Index < Length) {
		Index++;
		let CouldFind = false;

		if (!Filler) { // As long as it's not the filler, we're able to do this action, else we fill with ZEROs.
			for (let i = 0; i < 72; i++) {
				if (STR.charAt(Index - 1) == SAVUtils_CharWhiteList[i]) {
					SAVData.setUint8(Offs + (Index - 1), STR.charCodeAt(Index - 1));
					CouldFind = true;
					break;
				}
			}
		}

		if (!CouldFind) {
			Filler = true;
			SAVData.setUint8(Offs + (Index - 1), 0x0); // Place 0x0.
		}
	}

	if (!SAV.GetChangesMade()) SAV.SetChangesMade(true); // Changes have been made.
};

/*
	Call this when you are done.

	This updates the checksums etc and downloads the SAVFile.
	NOTE: Everything is being reset which is SAV Related, keep that in mind.
*/
export function SAVUtils_Finish() {
	if (SAVType == -1) return;

	SAV.Finish(); // Finish SAV Call.

	/* Setup and prepare Download Click. */
	let blob = new Blob([SAVBuffer], { type: "application/octet-stream" });
	let a = document.createElement('a');
	let url = window.URL.createObjectURL(blob);
	a.href = url;
	a.download = SAVName;

	a.click(); // Download the SAVFile.

	/* Reset SAV. */
	SAVType = -1;
	SAVName = '';
	SAV = undefined;
	SAVData = undefined;
	SAVBuffer = undefined;
	SAVSize = 0;
};