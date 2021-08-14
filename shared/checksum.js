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


/*
	Calculates the Checksum and returns it as an uint16_t.

	Buffer: The Save Buffer.
	StartOffs: The Start offset. (NOTE: You'll have to do '/ 2', because it's 2 byte based).
	EndOffs: The End offset. Same NOTE as above applies here as well.
	Skipoffs: The Offsets which to skip. same NOTE as above applies as well).
*/
export function Checksum_Calc(Buffer, StartOffs, EndOffs, SkipOffs) {
	if (!Buffer) return -1;

	let Byte1 = 0, Byte2 = 0;

	for(let Index = StartOffs; Index < EndOffs; Index++) {
		if (SkipOffs != undefined && SkipOffs.includes(Index)) continue; // Skip, if found in the Skip Offsets.

		Byte1 = (Byte1 + Buffer.getUint8(Index * 2));

		if (Byte1 > 255) {
			Byte1 = Byte1 % 256;
			Byte2++;
		}

		Byte2 = (Byte2 + Buffer.getUint8((Index * 2) + 1)) % 256;
	}

	Byte2++;
	if (Byte2 > 255) Byte2 = 0;

	return (256 * (256 - Byte2)) + (256 - Byte1); // Return it as an uint16_t.
};



/*
	Calculate the GBA Slot's checksum.

	Buffer: The Savebuffer.
	Slot: The slot to calculate.
*/
export function Checksum_CalcGBASlot(Buffer, Slot) {
	return Checksum_Calc(Buffer, (Slot * 0x1000) / 2, ((Slot * 0x1000) + 0xFFE) / 2);
};


/*
	Calculate the GBA Settings checksum.

	Buffer: The Savebuffer.
*/
export function Checksum_CalcGBASettings(Buffer) {
	return Checksum_Calc(Buffer, 0x0, (0x18 / 2), [(0xE / 2)]);
};



/*
	Calculate the NDS Slot's Main checksum.

	Buffer: The Savebuffer.
	Slot: The slot to calculate.
*/
export function Checksum_CalcNDSSlotMain(Buffer, Slot) {
	return Checksum_Calc(
		Buffer, ((Slot * 0x1000) + 0x10) / 2, ((Slot * 0x1000) + 0x1000) / 2,
		[(((Slot * 0x1000) + 0x12) / 2), (((Slot * 0x1000) + 0x28) / 2)] // Skipped Offsets.
	);
};


/*
	Calculate the NDS Slot's Shared checksum.

	Buffer: The Savebuffer.
	Slot: The slot to calculate.
*/
export function Checksum_CalcNDSSlotShared(Buffer, Slot) {
	return Checksum_Calc(
		Buffer, ((Slot * 0x1000) + 0x14) / 2, ((Slot * 0x1000) + 0x1000) / 2
	);
};


/*
	Calculate the NDS Slot's Header checksum.

	Buffer: The Savebuffer.
	Slot: The slot to calculate.

	That one is a bit more "special", as it relies on byte 0x13's value for the checksum
	and can't be used the way the main calculation method works.
*/
export function Checksum_CalcNDSSlotHeader(Buffer, Slot) {
	if (!Buffer) return -1;

	let Byte1 = 0, Byte2 = 0;

	for(let Index = (Slot * 0x1000) / 2; Index < ((Slot * 0x1000) + 0x13) / 2; Index++) {
		if (Index == (((Slot * 0x1000) + 0xE) / 2)) continue; // Skip checksum.

		Byte1 = (Byte1 + Buffer.getUint8(Index * 2));

		if (Byte1 > 255) {
			Byte1 = Byte1 % 256;
			Byte2++;
		}

		Byte2 = (Byte2 + Buffer.getUint8((Index * 2) + 1)) % 256;
	}

	/* If 0x13 is 0, then it just got created and hence, add +1 to Byte2. */
	if (Buffer.getUint8((Slot * 0x1000) + 0x13) == 0x0) Byte2++;

	if (Byte2 > 255) Byte2 = 0;
	return (256 * (256 - Byte2)) + (256 - Byte1); // Return it as an uint16_t.
};