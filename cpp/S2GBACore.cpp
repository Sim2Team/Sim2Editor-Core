/*
*   This file is part of S2GBACore
*   Copyright (C) 2020-2022 Sim2Team
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

#include "S2GBACore.hpp" // main include.
#include <unistd.h> // access().


/*
	--------------------------------------------------
	The Sims 2 Game Boy Advance Save File Editing Core
	--------------------------------------------------

	File: B46E.sav
	Contributors: SuperSaiyajinStackZ, Sim2Team
	Version: 0.5
	Purpose: Easy editing of a The Sims 2 Game Boy Advance Savefile.
	Category: Save File Editing Core
	Last Updated: 14 January 2022
	--------------------------------------------------

	Research used from here: https://sim2team.github.io/wiki/research/sims2gba.

	-----------------------
	Explanation of the Core
	-----------------------

	- Use S2GBACore::SaveHandler::LoadSav(const std::string &) to load a SAVFile from from a file.
	- Use S2GBACore::SaveHandler::LoadSav(const std::unique_ptr<uint8_t[]> &) to load a SAVFile from an already existing Buffer.
	- Use S2GBACore::Sav::Finish() to update all the Checksums.
	- Use S2GBACore::SaveHandler::WriteBack(const std::string &) to write your changes back to the File.
	- Use S2GBACore::Sav to access the Save Pointer and with that.. all the sub classes if needed. DO NOT ACCESS THOSE OUTSIDE, BECAUSE THEY RELY ON S2GBACore::Sav's POINTER!!!

	Another Note about THIS Core:
		THIS IS NOT THREAD-SAFE!!!, because I (SuperSaiyajinStackZ) don't care about it, since I don't really work with Threads anyways.

	To compile, you need to compile this with C++17 or above.

	-------------------------------
	Notes about the GBA Save itself
	-------------------------------

	- Only 0x5000 of 0x10000 / 0x20000 are used at all. 0x5000+ is unused / 0xFF padding so far I could see.
	- The Savefile has 5 Checksums in place. (See the Checksum namespace for reference on how to calculate it).

	The Checksum locations are the following:
		- 0xE - 0xF (Range: 0x0 - 0x18) -- That is the Settings Checksum. This should always be valid or it formats the complete Savefile!
		- 0x1FFE - 0x1FFF (Range: 0x1000 - 0x1FFF) -- That is the first Save Slot Checksum.
		- 0x2FFE - 0x2FFF (Range: 0x2000 - 0x2FFF) -- That is the second Save Slot Checksum.
		- 0x3FFE - 0x3FFF (Range: 0x3000 - 0x3FFF) -- That is the third Save Slot Checksum.
		- 0x4FFE - 0x4FFF (Range: 0x4000 - 0x4FFF) -- That is the fourth Save Slot Checksum.


	--------------------------
	Notes about the Save Slots
	--------------------------

	- Each Slot has a size of 0x1000.
	- There exist 4 Slots -- so the Slot Size together is 0x4000.
	- The House Items affect Offsets from the Save Slots. It can be found at offset 0xD6 of the Save Slot.
	-- Per House Item, the things after 0xD7 of the Save Slot move up for 0x6, this needs to be kept in mind.
	--- It is unsure yet, for how much the things move. I still have to research this part, but the Checksum stays at the same place.


	----------------------------
	Detecting the Savefile notes
	----------------------------

	To check that the Savefile is a The Sims 2 GBA Save, check for the following things:
	1. Make sure the Savefile has a size of 64 / 128 KB. (0x10000 / 0x20000)
	2. Make sure the first 7 byte are those: (0x53, 0x54, 0x57, 0x4E, 0x30, 0x32, 0x34).
		- So far, all of my Savefile contains the following bytes from Offset 0x0 - 0x7, so probably a good check to make sure.
*/

namespace S2GBACore {
	const std::vector<std::string> EncodingTable = {
		/* Special. */
		"©", "œ", "¡", "¿", "À", "Á", "Â", "Ã", "Ä", "Å", "Æ", "Ç", "È", "É", "Ê", "Ë",
		"Ì", "Í", "Î", "Ï", "Ñ", "Ò", "Ó", "Ô", "Õ", "Ö", "Ø", "Ù", "Ú", "Ü", "ß", "à",
		"á", "â", "ã", "ä", "å", "æ", "ç", "è", "é", "ê", "ë", "ì", "í", "î", "ï", "ñ",
		"ò", "ó", "ô", "õ", "ö", "ø", "ù", "ú", "û", "ü", "º", "ª", "…", "™", "", "®", ""
	};

	std::unique_ptr<SAV> Sav = nullptr;

	/*
		////////////////////////////////////////////////
		The Sims 2 GBA Checksum namespace implementation.
		Main Contributor: SuperSaiyajinStackZ.
		Last Updated: 21 October 2021.
		////////////////////////////////////////////////
	*/

	/*
		Calculate the Checksum from the Savefile.

		const uint8_t *Buffer: The Save Buffer.
		const uint16_t StartOffs: The Start offset. (NOTE: You'll have to do '/ 2', because it's 2 byte based).
		const uint16_t EndOffs: The End offset. Same NOTE as above applies here as well.
		const bool SettingsChks: If it's the Settings Checksum Method or not.
	*/
	uint16_t Checksum::Calc(const uint8_t *Buffer, const uint16_t StartOffs, const uint16_t EndOffs, const bool SettingsChks) {
		if (!Buffer) return 0;

		uint8_t Byte1 = 0, Byte2 = 0;

		for (uint16_t Idx = StartOffs; Idx < EndOffs; Idx++) {
			if (SettingsChks) {
				if (Idx == (0xE / 2)) continue; // Settings needs to have this.
			}

			if (Buffer[(Idx * 2)] + Byte1 > 255) Byte2++;
			Byte1 += Buffer[(Idx * 2)];
			Byte2 += Buffer[(Idx * 2) + 1];
		}

		Byte2++;
		return (256 * (uint8_t)-Byte2) + (uint8_t)-Byte1;
	};


	/*
		///////////////////////////////////////////////////
		The Sims 2 GBA SaveHandler namespace implementation.
		Main Contributor: SuperSaiyajinStackZ.
		Last Updated: 21 October 2021.
		///////////////////////////////////////////////////
	*/

	/*
		Main Save Loading method by passing a path to a Savefile.

		const std::string &File: The Savefile path.

		Returns true, if the save is valid.
	*/
	bool SaveHandler::LoadSav(const std::string &File) {
		S2GBACore::Sav = std::make_unique<S2GBACore::SAV>(File);

		return S2GBACore::Sav->GetValid();
	};

	/*
		Another Method to load the Savefile by passing the direct Save Buffer over.

		std::unique_ptr<uint8_t[]> &Data: The Save Buffer.
		const uint32_t Size: The Savesize.

		Returns true, if the save is valid.
	*/
	bool SaveHandler::LoadSav(std::unique_ptr<uint8_t[]> &Data, const uint32_t Size) {
		/* 64 and 128 KB are valid sizes for it. */
		if (Size == 0x10000 || Size == 0x20000) {
			S2GBACore::Sav = std::make_unique<S2GBACore::SAV>(Data, Size);

			return S2GBACore::Sav->GetValid();
		}

		return false;
	};

	/*
		Writes the changes back to the Savefile, if changes are made and the save is valid.

		const std::string &File: The Savefile path.

		Returns true, if the save got successfully written back.
	*/
	bool SaveHandler::WriteBack(const std::string &File) {
		bool Res = false;
		if (access(File.c_str(), F_OK) != 0) return Res;

		if (S2GBACore::Sav && S2GBACore::Sav->GetData() && S2GBACore::Sav->GetValid() && S2GBACore::Sav->GetChangesMade()) {
			FILE *Out = fopen(File.c_str(), "rb+");

			if (Out) {
				S2GBACore::Sav->Finish(); // Update Checksum, if necessary.
				fwrite(S2GBACore::Sav->GetData(), 1, S2GBACore::Sav->GetSize(), Out);
				fclose(Out);

				Res = true;
			}
		}

		return Res;
	};


	/*
		///////////////////////////////////////////////////
		The Sims 2 GBA SimUtils namespace implementation.
		Main Contributor: SuperSaiyajinStackZ.
		Last Updated: 21 October 2021.
		///////////////////////////////////////////////////
	*/

	/*
		Returns the current time as a 24 Hour or 12 Hour string.

		const uint16_t Time: The current time as an uint16_t.
		const bool AMPM: If using AM / PM or 24 Hours.

		This Results in: '13:44' or '01:44 PM'.
	*/
	const std::string SimUtils::TimeString(const uint16_t Time, const bool AMPM) {
		char TimeBuffer[(AMPM ? 11 : 8)];
		const uint8_t Minute = (uint8_t)(Time >> 8), Hour = (uint8_t)Time;

		if (AMPM) snprintf(TimeBuffer, sizeof(TimeBuffer), "%02d:%02d %s", (Hour > 11 ? Hour - 12 : Hour), Minute, (Hour > 11 ? "PM" : "AM"));
		else snprintf(TimeBuffer, sizeof(TimeBuffer), "%02d:%02d", Hour, Minute);

		return TimeBuffer;
	};

	/*
		Returns the current Simoleon amount as a string.

		const uint32_t Simoleons: The current Simoleons.
		const bool SimoleonSignAfter: If placing the Simoleons sign after the number, or before. Defaults to true (after).

		This results in 123.456$.
	*/
	const std::string SimUtils::SimoleonsString(const uint32_t Simoleons, const bool SimoleonSignAfter) {
		std::string SString = std::to_string(Simoleons);

		/* Here we'll add the periods. */
		switch(SString.size()) {
			case 4:
			case 5:
			case 6:
				SString.insert(SString.end() - 3, '.');
				break;

			case 7: // Technically, 7 Digits are possible too for the Save, but that should never happen.
			case 8:
			case 9:
				SString.insert(SString.end() - 6, '.');
				SString.insert(SString.end() - 3, '.');
				break;

			default:
				break;
		}

		/* Simoleons Sign. */
		if (SimoleonSignAfter) SString += "§";
		else SString = "§" + SString;

		return SString;
	};

	/*
		Returns the current Ratings as a string.

		const uint16_t Ratings: The current Ratings.

		This results in 1.345.
	*/
	const std::string SimUtils::RatingString(const uint16_t Ratings) {
		std::string RString = std::to_string(Ratings);

		/* That's how it's handled in The Sims 2 GBA. If there are more THAN 3 digits, a '.' is being added. */
		if (RString.size() > 3) RString.insert(RString.end() - 3, '.');
		return RString;
	};


	/*
		////////////////////////////////////////////////
		The Sims 2 GBA Strings namespace implementation.
		Main Contributor: SuperSaiyajinStackZ.
		Last Updated: 21 October 2021.
		////////////////////////////////////////////////
	*/
	const std::vector<std::string> Strings::CastNames = {
		"Emperor Xizzle", "Burple", "Ara Fusilli", "Auda Sherif",
		"Ava Cadavra", "Bigfoot", "Frankie Fusilli", "Dusty Hogg",
		"Giuseppi Mezzoalto", "Honest Jackson", "Jebediah Jerky", "Jimmy the Neck",
		"Kayleigh Wintercrest", "Luthor L. Bigbucks", "Mamma Hogg", "Misty Waters",
		"Lord Mole", "Mummy", "Optimum Alfred", "Penelope Redd",
		"Pepper Pete", "Kent Hackett", "Sancho Paco Panza", "Tank Grunt",
		"Tristan Legend", "Yeti"
	};

	const std::vector<std::string> Strings::EpisodeNames = {
		"It All Began", "Buried By the Mob", "What Digs Beneath", "Aliens Arrived",
		"Blackout!", "A Brand New Scent", "The New Cola", "There Was This Mummy",
		"Triassic Trouble", "The Doomed Earth", "It All Came to an End", "A Very Special Reunion",
		"Unofficial episode"
	};

	const std::vector<std::string> Strings::ItemNames = {
		"??? (Crash)", "Asteroid", "Balloons", "Crystal", "Cat Clock",
		"Chug Chug Cola Poster", "Bigfoot Print", "Friendly Fish Tank", "Hearts", "Intimidating Flame",
		"Intimidating Suit of Armor (Green)", "Intimidating Suit of Armor (Blue)", "Intimidating Suit of Armor (Orange)",
		"Intimidating Suit of Armor (Pink)", "Intimidating Suit of Armor (Red)",
		"Glowing Green Lamp", "Lava Lamp", "Black and White Lamp", "Mystery Hanging", "Romantic Fountain",
		"Romantic Giant Teddy Bear (Green)", "Romantic Giant Teddy Bear (Blue)", "Romantic Giant Teddy Bear (Orange)",
		"Romantic Giant Teddy Bear (Pink)", "Romantic Giant Teddy Bear (Red)", "Sarcophagus", "Real Shark Jaws",
		"Stuffed Penguin", "UFO model", "Yeti Doll", "???", "???", "Stereo (Green)", "Stereo (Blue)", "Stereo (Orange)",
		"Stereo (Pink)", "Stereo (Red)", "???", "Television", "Shabby Bed (Green)", "Shabby Bed (Blue)",
		"Shabby Bed (Orange)", "Shabby Bed (Pink)", "Shabby Bed (Red)", "Luxury Bed (Green)", "Luxury Bed (Blue)",
		"Luxury Bed (Orange)", "Luxury Bed (Pink)", "Luxury Bed (Red)", "Shabby Chair (Green)", "Shabby Chair (Blue)",
		"Shabby Chair (Orange)", "Shabby Chair (Pink)", "Shabby Chair (Red)", "Luxury Chair (Green)", "Luxury Chair (Blue)",
		"Luxury Chair (Orange)", "Luxury Chair (Pink)", "Luxury Chair (Red)", "Rib Cage Chair", "Shabby Couch (Green)",
		"Shabby Couch (Blue)", "Shabby Couch (Orange)", "Shabby Couch (Pink)", "Shabby Couch (Red)", "Luxury Couch (Green)",
		"Luxury Couch (Blue)", "Luxury Couch (Orange)", "Luxury Couch (Pink)", "Luxury Couch (Red)", "Kitchen Counter (Green)",
		"Kitchen Counter (Blue)", "Kitchen Counter (Orange)", "Kitchen Counter (Pink)", "Kitchen Counter (Red)", "??? (Crash)",
		"Treasure Chest",
		"Mailbox", // Contains the same functionality, as the Mailbox outside your House.
		"Luxury Refrigerator (Green)", "Luxury Refrigerator (Blue)", "Luxury Refrigerator (Orange)", "Luxury Refrigerator (Pink)",
		"Luxury Refrigerator (Red)", "Mini Refrigerator (Green)", "Mini Refrigerator (Blue)", "Mini Refrigerator (Orange)",
		"Mini Refrigerator (Pink)", "Mini Refrigerator (Red)", "Shabby Shower (Green)", "Shabby Shower (Blue)", "Shabby Shower (Orange)",
		"Shabby Shower (Pink)", "Shabby Shower (Red)", "Luxury Shower (Green)", "Luxury Shower (Blue)", "Luxury Shower (Orange)",
		"Luxury Shower (Pink)", "Luxury Shower (Red)", "Bathroom Sink", "Kitchen Sink (Green)", "Kitchen Sink (Blue)",
		"Kitchen Sink (Orange)", "Kitchen Sink (Pink)", "Kitchen Sink (Red)", "Standard Stove (Green)", "Standard Stove (Blue)",
		"Standard Stove (Orange)", "Standard Stove (Pink)", "Standard Stove (Red)", "Basic Toilet", "Alien Disguise Device",
		"???", "???", "???", "???", "???", "???", "Golden Chair", "???", "???", "Cake", "Gold Medal",
		"Egyptian Funerary Urn", "Hematite Misty Waters", "Chaz Dastard Insignia", "???", "Milk Crate",
		"Robot's Arm and Torso", "Robot's Left Arm", "Robot's Left Leg", "Robot's Leg and Torso", "Robot's Head",
		"Dangerous Parfume", "???", "???", "Pizza Box", "Jar of Plutonium", "???", "Robot Head", "Scrap Iron",
		"Telescope", "???", "Treasure Chest", "Video Camera", "Arrest Warrant", "Misty's BlueBerry", "Bug Spray",
		"Artificial Bouquet", "Bottle of Water", "Briefcase", "Cactus Fruit", "Cactus Spine", "Camera", "Bike Parts",
		"Dam Drain Plug", "Desert Beetle", "Dinosaur Leg Bone", "Dinosaur Rib Bone", "Dinosaur Skull Bone",
		"Dinosaur Spine Bone", "Dinosaur Tail Bone", "Heavy Work Gloves", "Green Cloth", "Bottle of Green Goo",
		"Robotic Hand", "Invitation", "Jar of Color", "Jump Ramp", "Bottle of Expired Makeup", "Bag of Manure",
		"Sheet of Parchment", "Unfinished Map", "Map of Canyonero Grande", "Megalodon Jawbone", "Microfiche",
		"Chaz Dastard DVDs", "Chocolates", "Box of Cockroaches", "Comic Books", "Dead Fish", "Funny Shirt",
		"Gold Ring", "Heart-shaped Pillow", "Mix CD", "Moldy Pie", "Pizza", "Red Roses", "Rotten Eggs", "Teddy Bear",
		"Wilted Flowers", "Movie Script", "Note", "Letter", "Paddleball", "???", "Penguin Invoice", "Petition",
		"Photo Album", "Pinwheel", "Note", "Pillows", "Ray Gun", "Pile of Receipts", "Rubber Waders", "Sancho's Note",
		"Packet of Seeds", "Shark", "Moisturizing Skin Cream", "Smoke Bomb", "Radiation Squeegee", "SPF 27000 Sunblock",
		"Thorium", "Vacuum Tube", "Cheeze Pizza", "Chicken Soup", "Hamburger", "Hotdog", "Turkey Leg", "Iced Tea",
		"Cherry Soda", "Root Beer", "Skill Book: Confidence", "Skill Book: Mechanical", "Skill Book: Strength",
		"Skill Book: Personality", "Skill Book: Hotness", "Skill Book: Intellect", "??? (Crash)", "??? (Crash)",
		"??? (Crash)", "Empty", "??? (Crash)", "??? (Crash)", "??? (Crash)", "??? (Crash)", "??? (Crash)", "??? (Crash)",
		"??? (Crash)", "??? (Crash)", "??? (Crash)", "??? (Crash)", "??? (Crash)", "??? (Crash)", "??? (Crash)", "??? (Crash)",
		"??? (Crash)", "??? (Crash)", "??? (Crash)", "??? (Crash)", "??? (Crash)", "??? (Crash)", "??? (Crash)", "??? (Crash)",
		"??? (Crash)", "??? (Crash)", "??? (Crash)"
	};

	const std::vector<std::string> Strings::MinigameNames = {
		"Bigfoot Love Chickens", "Car Commercial", "Keelhaulin' Cards", "Cattle Cleanup", "King Chug Chug", "Canyon Jumping", "Chop Shop"
	};

	const std::vector<std::string> Strings::SkillPointNames = {
		"Confidence", "Mechanical", "Strength", "Personality", "Hotness", "Intellect"
	};

	const std::vector<std::string> Strings::SocialMoveNames = {
		"Chit-Chat", "Entertain", "Hug", "Brag", "Apologize", "Sweet Talk", "Flirt", "Blow Kiss",
		"Kiss", "Show Off Body", "Annoy", "Insult", "Threaten", "Rude Gesture", "Karate Moves"
	};


	/*
		////////////////////////////////////////////////////
		The Sims 2 GBA Cast Save Editing class implementation.
		Main Contributor: SuperSaiyajinStackZ.
		Last Updated: 21 October 2021.
		////////////////////////////////////////////////////
	*/

	/* Get and Set Friendly Conversation level. */
	uint8_t Cast::Friendly() const { return S2GBACore::Sav->Read<uint8_t>(this->Offs); };
	void Cast::Friendly(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offs, std::min<uint8_t>(3, V)); };

	/* Get and Set Romance Conversation level. */
	uint8_t Cast::Romance() const { return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x1); };
	void Cast::Romance(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x1, std::min<uint8_t>(3, V)); };

	/* Get and Set Intimidate Conversation level. */
	uint8_t Cast::Intimidate() const { return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x2); };
	void Cast::Intimidate(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x2, std::min<uint8_t>(3, V)); };

	/* Get and Set the Feeling. */
	CastFeeling Cast::Feeling() const { return (CastFeeling)S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x3); };
	void Cast::Feeling(const CastFeeling V) { S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x3, (uint8_t)V); };

	/* Get and Set the Feeling's effect hours. */
	uint8_t Cast::FeelingEffectHours() const { return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x6); };
	void Cast::FeelingEffectHours(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x6, V); };

	/* Get and Set the registered on phone state. */
	bool Cast::RegisteredOnPhone() const { return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x7); };
	void Cast::RegisteredOnPhone(const bool V) { S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x7, V); };

	/* Get and Set Secret Unlock state. */
	bool Cast::Secret() const { return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x8); };
	void Cast::Secret(const bool V) { S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x8, V); };


	/*
		///////////////////////////////////////////////////////
		The Sims 2 GBA Episode Save Editing class implementation.
		Main Contributor: SuperSaiyajinStackZ.
		Last Updated: 14 January 2022.
		///////////////////////////////////////////////////////
	*/

	/* Get and Set Episode Ratings. */
	uint8_t Episode::Rating(const uint8_t Category) const {
		return S2GBACore::Sav->Read<uint8_t>(this->Offs + std::min<uint8_t>(3, Category));
	};
	void Episode::Rating(const uint8_t Category, const uint8_t V) {
		S2GBACore::Sav->Write<uint8_t>(this->Offs + std::min<uint8_t>(3, Category), std::min<uint8_t>(25, V));
	};

	/* Get and Set the Unlocked State. */
	bool Episode::Unlocked() const { return S2GBACore::Sav->ReadBit(this->Offs + 0x4, 0); };
	void Episode::Unlocked(const bool V) { S2GBACore::Sav->WriteBit(this->Offs + 0x4, 0, V); };

	/* Get and Set the Played State. */
	bool Episode::Played() const { return S2GBACore::Sav->ReadBit(this->Offs + 0x4, 1); };
	void Episode::Played(const bool V) { S2GBACore::Sav->WriteBit(this->Offs + 0x4, 1, V); };


	/*
		/////////////////////////////////////////////////////
		The Sims 2 GBA House Save Editing class implementation.
		Main Contributor: SuperSaiyajinStackZ.
		Last Updated: 21 October 2021.
		/////////////////////////////////////////////////////
	*/

	/*
		Get and Set the Room Design.
		Only 0 - 3 SHOULD be used at all, the others aren't actual room designs and instead may cause issues.
	*/
	uint8_t House::Roomdesign() const { return S2GBACore::Sav->ReadBits(this->Offs + 0x2E, true); };
	void House::Roomdesign(const uint8_t V) { S2GBACore::Sav->WriteBits(this->Offs + 0x2E, true, V); };

	/* Get the Items of your House / Room. */
	std::unique_ptr<HouseItem> House::Items() const { return std::make_unique<HouseItem>(this->Offs + 0xD6); };


	/*
		//////////////////////////////////////////////////////////
		The Sims 2 GBA House Item Save Editing class implementation.
		Main Contributor: SuperSaiyajinStackZ.
		Last Updated: 21 October 2021.
		//////////////////////////////////////////////////////////
	*/

	/* Get and Set the Item Count. */
	uint8_t HouseItem::Count() const { return S2GBACore::Sav->Read<uint8_t>(this->Offs); };
	void HouseItem::Count(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offs, V); };

	/* Get and Set the Item ID. */
	uint8_t HouseItem::ID(const uint8_t Index) const {
		if (this->Count() == 0) return 0xE6;

		return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x1 + (std::min<uint8_t>(this->Count() - 1, Index) * 0x6));
	};
	void HouseItem::ID(const uint8_t Index, const uint8_t V) {
		if (this->Count() == 0) return;

		S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x1 + (std::min<uint8_t>(this->Count() - 1, Index) * 0x6), V);
	};

	/* Get and Set the Item Flag. */
	uint8_t HouseItem::Flag(const uint8_t Index) const {
		if (this->Count() == 0) return 0x0;

		return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x2 + (std::min<uint8_t>(this->Count() - 1, Index) * 0x6));
	};
	void HouseItem::Flag(const uint8_t Index, const uint8_t V) {
		if (this->Count() == 0) return;

		S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x2 + (std::min<uint8_t>(this->Count() - 1, Index) * 0x6), V);
	};

	/* Get and Set the Use Count(?). */
	uint8_t HouseItem::UseCount(const uint8_t Index) const {
		if (this->Count() == 0) return 0x0;

		return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x3 + (std::min<uint8_t>(this->Count() - 1, Index) * 0x6));
	};
	void HouseItem::UseCount(const uint8_t Index, const uint8_t V) {
		if (this->Count() == 0) return;

		S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x3 + (std::min<uint8_t>(this->Count() - 1, Index) * 0x6), V);
	};

	/* Get and Set the X Position of the Item. */
	uint8_t HouseItem::XPos(const uint8_t Index) const {
		if (this->Count() == 0) return 0x0;

		return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x4 + (std::min<uint8_t>(this->Count() - 1, Index) * 0x6));
	};
	void HouseItem::XPos(const uint8_t Index, const uint8_t V) {
		if (this->Count() == 0) return;

		S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x4 + (std::min<uint8_t>(this->Count() - 1, Index) * 0x6), V);
	};

	/* Get and Set the Y Position of the Item. */
	uint8_t HouseItem::YPos(const uint8_t Index) const {
		if (this->Count() == 0) return 0x0;

		return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x5 + (std::min<uint8_t>(this->Count() - 1, Index) * 0x6));
	};
	void HouseItem::YPos(const uint8_t Index, const uint8_t V) {
		if (this->Count() == 0) return;

		S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x5 + (std::min<uint8_t>(this->Count() - 1, Index) * 0x6), V);
	};

	/* Get and Set the Item Direction. */
	HouseItemDirection HouseItem::Direction(const uint8_t Index) const {
		if (this->Count() == 0) return HouseItemDirection::Invalid;

		const uint8_t D = S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x6 + (std::min<uint8_t>(this->Count() - 1, Index)) * 0x6);

		switch(D) {
			case 0x1:
				return HouseItemDirection::Right;

			case 0x3:
				return HouseItemDirection::Down;

			case 0x5:
				return HouseItemDirection::Left;

			case 0x7:
				return HouseItemDirection::Up;
		}

		return HouseItemDirection::Invalid;
	};
	void HouseItem::Direction(const uint8_t Index, const HouseItemDirection V) {
		if (this->Count() == 0) return;

		switch(V) {
			case HouseItemDirection::Right:
				S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x6 + (std::min<uint8_t>(this->Count() - 1, Index)) * 0x6, 0x1);
				break;

			case HouseItemDirection::Down:
				S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x6 + (std::min<uint8_t>(this->Count() - 1, Index)) * 0x6, 0x3);
				break;

			case HouseItemDirection::Left:
				S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x6 + (std::min<uint8_t>(this->Count() - 1, Index)) * 0x6, 0x5);
				break;

			case HouseItemDirection::Up:
				S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x6 + (std::min<uint8_t>(this->Count() - 1, Index)) * 0x6, 0x7);
				break;

			case HouseItemDirection::Invalid:
				break;
		}
	};

	/*
		Add an Item to the House.
		This needs to be handled like this, because things move 0x6 bytes up when an Item is being added.

		NOTE:
			The game seems to handle it the other way than this;
			by doing an insert or something at the 0xD7'th byte, but this way works too.

	*/
	bool HouseItem::AddItem(const uint8_t ID, const uint8_t Flag, const uint8_t UseCount, const uint8_t XPos, const uint8_t YPos, const HouseItemDirection Direction) {
		if (this->Count() == 0xC) return false; // Not allowed to add more than 0xC / 12 Items.

		const uint8_t CT = this->Count();
		this->Count(CT + 0x1);

		std::unique_ptr<uint8_t[]> TMP = std::make_unique<uint8_t[]>(0xF26 - (this->Count() * 6));
		memcpy( // Copy first to a TMP pointer.
			TMP.get(),
			S2GBACore::Sav->GetData() + (this->Offs + 0x1) + (CT * 0x6),
			0xF26 - (this->Count() * 6)
		);

		memcpy( // Then copy to the actual location from the TMP pointer.
			S2GBACore::Sav->GetData() + (this->Offs + 0x1) + (this->Count() * 0x6),
			TMP.get(),
			0xF26 - (this->Count() * 6)
		);

		/* Set Item Data. */
		this->ID(CT, ID);
		this->Flag(CT, Flag);
		this->UseCount(CT, UseCount);
		this->XPos(CT, XPos);
		this->YPos(CT, YPos);
		this->Direction(CT, Direction);

		return true;
	};

	/*
		Remove an Item from the House.
		This needs to be handled like this, because things move 0x6 bytes down when an Item is being removed.
	*/
	bool HouseItem::RemoveItem(const uint8_t Index) {
		if ((this->Count() == 0) || (this->Count() - 1 < Index)) return false; // Nanana, Index and or Count is not good.

		this->Count(this->Count() - 0x1);

		std::unique_ptr<uint8_t[]> TMP = std::make_unique<uint8_t[]>(0xF26 - (this->Count() * 6));
		memcpy( // Copy first to a TMP pointer.
			TMP.get(),
			S2GBACore::Sav->GetData() + (this->Offs + 0x1) + ((Index + 0x1) * 0x6),
			0xF26 - (this->Count() * 6)
		);

		memcpy( // Then copy to the actual location from the TMP pointer.
			S2GBACore::Sav->GetData() + (this->Offs + 0x1) + (Index * 0x6),
			TMP.get(),
			0xF26 - (this->Count() * 6)
		);

		return true;
	};


	/*
		////////////////////////////////////////////////////////////
		The Sims 2 GBA Item Package Save Editing class implementation.
		Main Contributor: SuperSaiyajinStackZ.
		Last Updated: 21 October 2021.
		////////////////////////////////////////////////////////////
	*/

	/* Get and Set the Item Count. */
	uint8_t ItemPackage::Count() const { return S2GBACore::Sav->Read<uint8_t>(this->Offs); };
	void ItemPackage::Count(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offs, V); };

	/* Get and Set the Item's ID. */
	uint8_t ItemPackage::ID(const uint8_t Index) const {
		return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x1 + (std::min<uint8_t>(5, Index) * 0x3));
	};
	void ItemPackage::ID(const uint8_t Index, const uint8_t V) {
		S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x1 + (std::min<uint8_t>(5, Index) * 0x3), V);

		/* Update Item Count. */
		uint8_t Amount = 0;
		for (uint8_t Idx = 0; Idx < 6; Idx++) {
			if (this->ID(Idx) != 0xE6) Amount++; // If not 0xE6 (Empty Item), increase count.
		}

		if (this->Count() != Amount) this->Count(Amount);
	};

	/* Get and Set the Item's flag. */
	uint8_t ItemPackage::Flag(const uint8_t Idx) const { return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x2 + (std::min<uint8_t>(5, Idx) * 0x3)); };
	void ItemPackage::Flag(const uint8_t Idx, const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x2 + (std::min<uint8_t>(5, Idx) * 0x3), V); };

	/* Get and Set the Item's Use Count. */
	uint8_t ItemPackage::UseCount(const uint8_t Idx) const { return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x3 + (std::min<uint8_t>(5, Idx) * 0x3)); };
	void ItemPackage::UseCount(const uint8_t Idx, const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x3 + (std::min<uint8_t>(5, Idx) * 0x3), V); };


	/*
		////////////////////////////////////////////////////////
		The Sims 2 GBA Minigame Save Editing class implementation.
		Main Contributor: SuperSaiyajinStackZ.
		Last Updated: 21 October 2021.
		////////////////////////////////////////////////////////
	*/

	/* Get and Set if you played that game already today. */
	bool Minigame::Played() const { return S2GBACore::Sav->ReadBit(this->Offs, this->Game); };
	void Minigame::Played(const bool V) { S2GBACore::Sav->WriteBit(this->Offs, this->Game, V); };

	/* Get and Set the Minigame Level. */
	uint8_t Minigame::Level() const { return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x24 + this->Game); };
	void Minigame::Level(const uint8_t V, const bool MetaData) {
		S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x24 + this->Game, std::min<uint8_t>(5, V));

		/* Optionally: Set to Metadata / Settings as well. */
		if (MetaData) S2GBACore::Sav->WriteBits(0x10 + (this->Game / 2), ((this->Game % 2) == 0), std::min<uint8_t>(5, V));
	};


	/*
		///////////////////////////////////////////////////
		The Sims 2 GBA SAV Save Editing class implementation.
		Main Contributor: SuperSaiyajinStackZ.
		Last Updated: 14 January 2021.
		///////////////////////////////////////////////////
	*/

	/*
		Main Save Constructor by passing over the path to the Savefile.

		const std::string &File: The Savefile path.
	*/
	SAV::SAV(const std::string &File) {
		if (access(File.c_str(), F_OK) != 0) { // File doesn't exist.
			this->SavValid = false;
			this->SavSize = 0;
			return;
		}

		FILE *In = fopen(File.c_str(), "r");

		if (In) {
			fseek(In, 0, SEEK_END);
			this->SavSize = ftell(In);
			fseek(In, 0, SEEK_SET);

			/* 64 and 128 KB are valid sizes for it. */
			if (this->SavSize == 0x10000 || this->SavSize == 0x20000) {
				this->SavData = std::make_unique<uint8_t[]>(this->SavSize);
				fread(this->SavData.get(), 1, this->SavSize, In);
				this->SavValid = this->ValidationCheck();

			} else {
				this->SavValid = false;
			}

			fclose(In);
		}
	};

	/*
		Some Save Validation checks.

		Returns true if the Save is valid, false if not.
	*/
	bool SAV::ValidationCheck() {
		if (!this->GetData()) return false;

		/* Now do the Validation check through the Save Header with the GBAIdents. */
		bool Res = true;
		for (uint8_t Idx = 0; Idx < 8; Idx++) {
			if (this->GetData()[Idx] != this->GBAIdent[Idx]) {
				Res = false;
				break;
			}
		}

		/* Language Checks as well, because why not. */
		if (this->GetData()[0xA] > 5) { // Language Index is 6 or larger, which is "blank" and can break the game.
			this->GetData()[0xA] = 0; // English.
			this->ChangesMade = true;
		}

		return Res;
	};


	/*
		Return a bit from the Save Buffer.

		const uint32_t Offs: The Offset to read from.
		const uint8_t BitIndex: The Bit index ( 0 - 7 ).
	*/
	bool SAV::ReadBit(const uint32_t Offs, const uint8_t BitIndex) const {
		if (!this->GetValid() || !this->GetData() || BitIndex > 7) return false;

		return (this->GetData()[Offs] >> BitIndex & 1) != 0;
	};

	/*
		Set a bit to the Save Buffer.

		const uint32_t Offs: The Offset to write to.
		const uint8_t BitIndex: The Bit index ( 0 - 7 ).
		const bool IsSet: If it's set (1) or not (0).
	*/
	void SAV::WriteBit(const uint32_t Offs, const uint8_t BitIndex, const bool IsSet) {
		if (!this->GetValid() || !this->GetData() || BitIndex > 7) return;

		this->GetData()[Offs] &= ~(1 << BitIndex);
		this->GetData()[Offs] |= (IsSet ? 1 : 0) << BitIndex;

		if (!this->ChangesMade) this->ChangesMade = true;
	};

	/*
		Read Lower / Upper Bits from the Save Buffer.

		const uint32_t Offs: The offset where to read from.
		const bool First: If Reading from the first four bits, or second.
	*/
	uint8_t SAV::ReadBits(const uint32_t Offs, const bool First) const {
		if (!this->GetValid() || !this->GetData()) return 0x0;

		if (First) return (this->GetData()[Offs] & 0xF); // Bit 0 - 3.
		else return (this->GetData()[Offs] >> 4); // Bit 4 - 7.
	};

	/*
		Write Lower / Upper Bits to the Save Buffer.

		const uint32_t Offs: The offset where to write to.
		const bool First: If Writing on the first four bits, or second.
		const uint8_t Data: The Data to write.
	*/
	void SAV::WriteBits(const uint32_t Offs, const bool First, const uint8_t Data) {
		if (!this->GetValid() || !this->GetData() || Data > 0xF) return;

		if (First) this->GetData()[Offs] = (this->GetData()[Offs] & 0xF0) | (Data & 0xF); // Bit 0 - 3.
		else this->GetData()[Offs] = (this->GetData()[Offs] & 0x0F) | (Data << 4); // Bit 4 - 7.

		if (!this->ChangesMade) this->ChangesMade = true;
	};

	/*
		Read a string from the Save Buffer.

		const uint32_t Offs: The Offset from where to read from.
		const uint32_t Length: The Length to read.
	*/
	std::string SAV::ReadString(const uint32_t Offs, const uint32_t Length) const {
		if (!this->GetValid() || !this->GetData()) return "";

		std::string Str = "";

		for (size_t Idx = 0; Idx < Length; Idx++) {
			const uint8_t Byte = this->GetData()[Offs + Idx];

			/* Out of range, or the last check being a new line which may not be great on names. */
			if ((Byte >= 0x1 && Byte <= 0x9) || (Byte >= 0xB && Byte <= 0x1F) || (Byte >= 0xBC) || (Byte == 0xA)) break;

			/* 0x7B - 0xBB is custom encoding, the other one is ASCII. */
			else if (Byte >= 0x7B && Byte <= 0xBB) Str += S2GBACore::EncodingTable[Byte - 0x7B];
			else {
				Str += Byte;
				if (Byte == 0x0) break;
			}
		}

		return Str;
	};

	/*
		Write a string to the Save Buffer.

		const uint32_t Offs: The offset from where to write to.
		const uint32_t Length: The length to write.
		const std::string &Str: The string to write.
	*/
	void SAV::WriteString(const uint32_t Offs, const uint32_t Length, const std::string &Str) {
		if (!this->GetValid() || !this->GetData()) return;

		bool Found = false;
		size_t EncodingLen = 0, EncodingMatchCount = 0;
		std::vector<uint8_t> StringBytes = { };

		/* Get the StringBytes that we would write. */
		for (size_t StrIdx = 0; StrIdx < Str.size(); StrIdx++) {
			const uint8_t Byte = (uint8_t)Str[StrIdx];

			/* Values that are too low for ASCII and 0xA for a new line may not be the greatest, so it's blocked for now until there is a valid reason to use one lol. */
			if ((Byte >= 0x1 && Byte <= 0x9) || (Byte >= 0xB && Byte <= 0x1F) || (Byte == 0xA)) break;
			else {
				Found = false;

				/* Check for ASCII. */
				for (size_t ASCIIIdx = 0x20; ASCIIIdx < 0x7A; ASCIIIdx++) {
					if (ASCIIIdx == Byte) {
						Found = true;
						StringBytes.push_back(ASCIIIdx);
						break;
					}
				}

				if (!Found) {
					/* Check for the Special Encoding. */
					for (size_t EncodingIdx = 0x0; EncodingIdx < S2GBACore::EncodingTable.size(); EncodingIdx++) {
						EncodingLen = S2GBACore::EncodingTable[EncodingIdx].size(); // Get the length of the encoding character.
						EncodingMatchCount = 0;

						if (EncodingLen == 0) continue; // There are also characters that have a size of 0, so skip those to not cause an infinity loop.

						for (size_t EncodingLenIdx = 0; EncodingLenIdx < EncodingLen; EncodingLenIdx++) {
							if (Str.size() - 1 < StrIdx + EncodingLenIdx) break;
							if (Str[StrIdx + EncodingLenIdx] == S2GBACore::EncodingTable[EncodingIdx][EncodingLenIdx]) EncodingMatchCount++;
							else break;
						}

						/* If the encoding length matches with the equal characters, then we got the right character. */
						if (EncodingMatchCount == EncodingLen) {
							Found = true;
							StringBytes.push_back(EncodingIdx + 0x7B);
							StrIdx += EncodingLen - 1;
							break;
						}
					}

					/* If nothing matches here either, end it with a NULL Terminator. */
					if (!Found) {
						StringBytes.push_back(0);
						break;
					}
				}
			}
		}

		/* Now write the actual bytes to the SaveData. */
		for (size_t Idx = 0; Idx < Length; Idx++) {
			if (Idx < StringBytes.size()) this->GetData()[Offs + Idx] = StringBytes[Idx];
			else this->GetData()[Offs + Idx] = 0x0;
		}

		if (!this->ChangesMade) this->ChangesMade = true;
	};

	/*
		Return, wheter a Slot is valid / exist.

		const uint8_t Slot: The Slot to check ( 1 - 4).
	*/
	bool SAV::SlotExist(const uint8_t Slot) const {
		if (Slot < 1 || Slot > 4) return false;

		for (uint8_t Idx = 0; Idx < 10; Idx++) {
			if (this->GetData()[(Slot * 0x1000) + Idx] != 0) return true;
		}

		return false;
	};

	/*
		Return a Slot class.

		const uint8_t Slt: The Slot ( 1 - 4 ).
	*/
	std::unique_ptr<Slot> SAV::_Slot(const uint8_t Slt) const {
		if (!this->SlotExist(Slt)) return nullptr;

		return std::make_unique<Slot>(Slt);
	};

	/* Get a Settings class. */
	std::unique_ptr<Settings> SAV::_Settings() const { return std::make_unique<Settings>(); };

	/*
		The Save Finish call, which updates the checksum and such.
	*/
	void SAV::Finish() {
		if (!this->GetValid()) return;

		for (uint8_t Slt = 1; Slt < 5; Slt++) { // Update Slot Checksums, if invalid and exist.
			if (this->SlotExist(Slt)) this->_Slot(Slt)->FixChecksum();
		}

		/* Do the same with the Settings. */
		this->_Settings()->UpdateChecksum();
	};


	/*
		////////////////////////////////////////////////////////
		The Sims 2 GBA Settings Save Editing class implementation.
		Main Contributor: SuperSaiyajinStackZ.
		Last Updated: 21 October 2021.
		////////////////////////////////////////////////////////
	*/

	/* Get and Set the Sound Effect Volume. */
	uint8_t Settings::SFX() const { return S2GBACore::Sav->Read<uint8_t>(0x8); };
	void Settings::SFX(const uint8_t V) {
		if (V > 10) return; // 0 - 10 only valid.
		S2GBACore::Sav->Write<uint8_t>(0x8, this->SFXLevels[V]);
	};

	/* Get and Set the Music Volume. */
	uint8_t Settings::Music() const { return S2GBACore::Sav->Read<uint8_t>(0x9); };
	void Settings::Music(const uint8_t V) {
		if (V > 10) return; // 0 - 10 only valid.
		S2GBACore::Sav->Write<uint8_t>(0x9, this->MusicLevels[V]);
	};

	/* Get and Set the Language. */
	Langs Settings::Language() const {
		if (S2GBACore::Sav->Read<uint8_t>(0xA) > 5) return Langs::EN; // Technically, that would be a "blank" Language in game, but ehh that's not good.
		return (Langs)S2GBACore::Sav->Read<uint8_t>(0xA);
	};
	void Settings::Language(const Langs V) {
		S2GBACore::Sav->Write<uint8_t>(0xA, (uint8_t)V);
	};

	/* Update the Checksum of the GBA Settings. */
	void Settings::UpdateChecksum() {
		const uint16_t CurCHKS = S2GBACore::Sav->Read<uint16_t>(0xE);
		const uint16_t Calced = Checksum::Calc(S2GBACore::Sav->GetData(), 0x0, 0x18 / 2, true);

		if (Calced != CurCHKS) { // If the calced result is NOT the current checksum.
			S2GBACore::Sav->Write<uint16_t>(0xE, Calced);
		}
	};


	/*
		////////////////////////////////////////////////////
		The Sims 2 GBA Slot Save Editing class implementation.
		Main Contributor: SuperSaiyajinStackZ.
		Last Updated: 14 January 2022.
		////////////////////////////////////////////////////
	*/

	/*
		The House Item Amount seems to affect some stuff and move things around for 0x6 per Item.
		So, we get the Item Count of the House from the 0xD6'th Byte from the GBA Slot.

		const uint32_t DefaultOffs: The Default Offset, for things without an Item in your house.
	*/
	uint32_t Slot::Offset(const uint32_t DefaultOffs) const {
		return (this->Offs + DefaultOffs) + (S2GBACore::Sav->Read<uint8_t>(this->Offs + 0xD6) * 0x6);
	};

	/* Get and Set Hour. */
	uint8_t Slot::Hour() const { return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x2); };
	void Slot::Hour(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x2, V); };

	/* Get and Set Minute. */
	uint8_t Slot::Minute() const { return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x3); };
	void Slot::Minute(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x3, V); };

	/* Get and Set Seconds. */
	uint8_t Slot::Seconds() const { return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x4); };
	void Slot::Seconds(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x4, V); };

	/* Get and Set Simoleons. */
	uint32_t Slot::Simoleons() const { return S2GBACore::Sav->Read<uint32_t>(this->Offs + 0x5) >> 8; };
	void Slot::Simoleons(uint32_t V) { S2GBACore::Sav->Write<uint32_t>(this->Offs + 0x5, (std::min<uint32_t>(999999, V) << 8)); };

	/* Get and Set Ratings. */
	uint16_t Slot::Ratings() const { return S2GBACore::Sav->Read<uint16_t>(this->Offs + 0xA); };
	void Slot::Ratings(const uint16_t V) { S2GBACore::Sav->Write<uint16_t>(this->Offs + 0xA, std::min<uint16_t>(9999, V)); };

	/* Get and Set Name. */
	std::string Slot::Name() const { return S2GBACore::Sav->ReadString(this->Offs + 0xD, 16); };
	void Slot::Name(const std::string &V) { S2GBACore::Sav->WriteString(this->Offs + 0xD, 16, V); };


	/* Get and Set Hairstyle. */
	uint8_t Slot::Hairstyle() const { return S2GBACore::Sav->ReadBits(this->Offs + 0x1D, false) / 2; };
	void Slot::Hairstyle(const uint8_t V) {
		if (V > 7) return;

		S2GBACore::Sav->WriteBits(this->Offs + 0x1D, false, (V * 2) + (this->Shirtcolor3() > 15 ? 0x1 : 0x0));
	};

	/* Get and Set third Shirtcolor (Long Sleeves). */
	uint8_t Slot::Shirtcolor3() const { return ((S2GBACore::Sav->ReadBits(this->Offs + 0x1D, false) % 2 == 1) ? 16 : 0) + S2GBACore::Sav->ReadBits(this->Offs + 0x1D, true); };
	void Slot::Shirtcolor3(const uint8_t V) {
		S2GBACore::Sav->WriteBits(this->Offs + 0x1D, true, ((V > 15) ? V - 16 : V));
		S2GBACore::Sav->WriteBits(this->Offs + 0x1D, false, (this->Hairstyle() * 2) + (V > 15 ? 0x1 : 0x0)); // Refresh Hairstyle as well.
	};

	/* Get and Set Tan / Skin color. */
	uint8_t Slot::Tan() const { return S2GBACore::Sav->ReadBits(this->Offs + 0x1E, false) / 2; };
	void Slot::Tan(const uint8_t V) {
		if (V > 5) return;

		S2GBACore::Sav->WriteBits(this->Offs + 0x1E, false, (V * 2) + (this->Shirtcolor2() > 15 ? 0x1 : 0x0));
	};

	/* Get and Set second Shirtcolor (Short Sleeves). */
	uint8_t Slot::Shirtcolor2() const { return ((S2GBACore::Sav->ReadBits(this->Offs + 0x1E, false) % 2 == 1) ? 16 : 0) + S2GBACore::Sav->ReadBits(this->Offs + 0x1E, true); };
	void Slot::Shirtcolor2(const uint8_t V) {
		S2GBACore::Sav->WriteBits(this->Offs + 0x1E, true, ((V > 15) ? V - 16 : V));
		S2GBACore::Sav->WriteBits(this->Offs + 0x1E, false, (S2GBACore::Sav->ReadBits(this->Offs + 0x1E, false) * 2) + (V > 15 ? 0x1 : 0x0)); // Refresh Tan as well.
	};

	/* Get and Set Haircolor. */
	uint8_t Slot::Haircolor() const { return S2GBACore::Sav->ReadBits(this->Offs + 0x1F, false); };
	void Slot::Haircolor(const uint8_t V) { S2GBACore::Sav->WriteBits(this->Offs + 0x1F, false, V); };

	/* Get the Hatcolor. NOTE: Is also shoe color. */
	uint8_t Slot::Hatcolor() const { return S2GBACore::Sav->ReadBits(this->Offs + 0x1F, true); };
	void Slot::Hatcolor(const uint8_t V) { S2GBACore::Sav->WriteBits(this->Offs + 0x1F, true, V); };

	/* Get and Set Shirt Type. */
	uint8_t Slot::Shirt() const { return S2GBACore::Sav->ReadBits(this->Offs + 0x20, false) / 2; };
	void Slot::Shirt(const uint8_t V) {
		if (V > 5) return;

		S2GBACore::Sav->WriteBits(this->Offs + 0x20, false, (V * 2) + (this->Shirtcolor1() > 15 ? 0x1 : 0x0));
	};

	/* Get and Set first Shirtcolor (Body). */
	uint8_t Slot::Shirtcolor1() const { return ((S2GBACore::Sav->ReadBits(this->Offs + 0x20, false) % 2 == 1) ? 16 : 0) + S2GBACore::Sav->ReadBits(this->Offs + 0x20, true); };
	void Slot::Shirtcolor1(const uint8_t V) {
		S2GBACore::Sav->WriteBits(this->Offs + 0x20, true, ((V > 15) ? V - 16 : V));
		S2GBACore::Sav->WriteBits(this->Offs + 0x20, false, (S2GBACore::Sav->ReadBits(this->Offs + 0x20, false) * 2) + (V > 15 ? 0x1 : 0x0)); // Refresh Shirt as well.
	};

	/* Get and Set Pants. */
	uint8_t Slot::Pants() const { return S2GBACore::Sav->ReadBits(this->Offs + 0x21, false) / 2; };
	void Slot::Pants(const uint8_t V) {
		if (V > 1) return;

		S2GBACore::Sav->WriteBits(this->Offs + 0x21, false, (V * 2) + (this->Pantscolor() > 15 ? 0x1 : 0x0));
	};

	/* Get and Set Pantscolor. */
	uint8_t Slot::Pantscolor() const { return ((S2GBACore::Sav->ReadBits(this->Offs + 0x21, false) % 2 == 1) ? 16 : 0) + S2GBACore::Sav->ReadBits(this->Offs + 0x21, true); };
	void Slot::Pantscolor(const uint8_t V) {
		S2GBACore::Sav->WriteBits(this->Offs + 0x21, true, ((V > 15) ? V - 16 : V));
		S2GBACore::Sav->WriteBits(this->Offs + 0x21, false, (this->Pants() * 2) + (V > 15 ? 0x1 : 0x0)); // Refresh Pants as well.
	};

	/* Get and Set the Confidence Skill Points. */
	uint8_t Slot::Confidence() const { return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x22); };
	void Slot::Confidence(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x22, std::min<uint8_t>(5, V)); };

	/* Get and Set the Mechanical Skill Points. */
	uint8_t Slot::Mechanical() const { return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x23); };
	void Slot::Mechanical(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x23, std::min<uint8_t>(5, V)); };

	/* Get and Set the Strength Skill Points. */
	uint8_t Slot::Strength() const { return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x24); };
	void Slot::Strength(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x24, std::min<uint8_t>(5, V)); };

	/* Get and Set the Personality Skill Points. */
	uint8_t Slot::Personality() const { return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x25); };
	void Slot::Personality(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x25, std::min<uint8_t>(5, V)); };

	/* Get and Set the Hotness Skill Points. */
	uint8_t Slot::Hotness() const { return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x26); };
	void Slot::Hotness(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x26, std::min<uint8_t>(5, V)); };

	/* Get and Set the Intellect Skill Points. */
	uint8_t Slot::Intellect() const { return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x27); };
	void Slot::Intellect(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x27, std::min<uint8_t>(5, V)); };

	/* Get and Set the Sanity. */
	uint8_t Slot::Sanity() const { return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x32); };
	void Slot::Sanity(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x32, std::min<uint8_t>(100, V)); };

	/* Get and Set the Aspiration. */
	uint8_t Slot::Aspiration() const { return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x4B); };
	void Slot::Aspiration(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x4B, std::min<uint8_t>(2, V)); };

	/* Return some ItemPackages of 6 Items each package. */
	std::unique_ptr<ItemPackage> Slot::PawnShop() const { return std::make_unique<ItemPackage>(this->Offs + 0x4C); };
	std::unique_ptr<ItemPackage> Slot::Saloon() const { return std::make_unique<ItemPackage>(this->Offs + 0x5F); };
	std::unique_ptr<ItemPackage> Slot::Skills() const { return std::make_unique<ItemPackage>(this->Offs + 0x72); };
	std::unique_ptr<ItemPackage> Slot::Mailbox() const { return std::make_unique<ItemPackage>(this->Offs + 0x98); };
	std::unique_ptr<ItemPackage> Slot::Inventory() const { return std::make_unique<ItemPackage>(this->Offs + 0xAB); };

	/* Return House Items. */
	std::unique_ptr<House> Slot::_House() const { return std::make_unique<House>(this->Offs); };

	/* Get and Set Empty Chug-Chug Cola Cans Amount. */
	uint8_t Slot::Cans() const { return S2GBACore::Sav->Read<uint8_t>(this->Offset(0xF6)); };
	void Slot::Cans(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offset(0xF6), std::min<uint8_t>(250, V)); };

	/* Get and Set Cowbells Amount. */
	uint8_t Slot::Cowbells() const { return S2GBACore::Sav->Read<uint8_t>(this->Offset(0xF7)); };
	void Slot::Cowbells(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offset(0xF7), std::min<uint8_t>(250, V)); };

	/* Get and Set Alien Spaceship Parts Amount. */
	uint8_t Slot::Spaceship() const { return S2GBACore::Sav->Read<uint8_t>(this->Offset(0xF8)); };
	void Slot::Spaceship(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offset(0xF8), std::min<uint8_t>(250, V)); };

	/* Get and Set Nuclear Fuelrods Amount. */
	uint8_t Slot::Fuelrods() const { return S2GBACore::Sav->Read<uint8_t>(this->Offset(0xF9)); };
	void Slot::Fuelrods(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offset(0xF9), std::min<uint8_t>(250, V)); };

	/* Get and Set Empty Chug-Chug Cola Cans Sell price. */
	uint8_t Slot::CansPrice() const { return S2GBACore::Sav->Read<uint8_t>(this->Offset(0xFA)); };
	void Slot::CansPrice(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offset(0xFA), V); };

	/* Get and Set the Cowbells Sell price. */
	uint8_t Slot::CowbellsPrice() const { return S2GBACore::Sav->Read<uint8_t>(this->Offset(0xFB)); };
	void Slot::CowbellsPrice(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offset(0xFB), V); };

	/* Get and Set Alien Spaceship Parts Sell price. */
	uint8_t Slot::SpaceshipPrice() const { return S2GBACore::Sav->Read<uint8_t>(this->Offset(0xFC)); };
	void Slot::SpaceshipPrice(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offset(0xFC), V); };

	/* Get and Set Nuclear Fuelrods Sell price. */
	uint8_t Slot::FuelrodsPrice() const { return S2GBACore::Sav->Read<uint8_t>(this->Offset(0xFD)); };
	void Slot::FuelrodsPrice(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offset(0xFD), V); };

	/* Get the Current Episode you are in. */
	uint8_t Slot::CurrentEpisode() const {
		for (uint8_t Idx = 0; Idx < 12; Idx++) {
			if (S2GBACore::Sav->Read<uint8_t>(this->Offset(0x1A3)) == this->EPVals[Idx]) return Idx;
		}

		return 12; // 12 -> "Unofficial Episode". ;P
	};

	/*
		Set the Current Episode.

		const uint8_t V: The Episode.
		const bool ValidCheck: If checking for official Episodes (valid) or not. It is recommended to have this to TRUE.
	*/
	void Slot::CurrentEpisode(const uint8_t V, const bool ValidCheck) {
		if (!ValidCheck) { // In case we're not checking for validateness, Set it without checks.
			S2GBACore::Sav->Write<uint8_t>(this->Offset(0x1A3), V);
			S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x9, V); // It's better to set that to 0x9 as well for display.
			return;
		}

		for (uint8_t Idx = 0; Idx < 12; Idx++) {
			if (V == this->EPVals[Idx]) {
				S2GBACore::Sav->Write<uint8_t>(this->Offset(0x1A3), V);
				S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x9, V); // It's better to set that to 0x9 as well for display.
				break;
			}
		}
	};

	/* Get a Minigame class. */
	std::unique_ptr<Minigame> Slot::_Minigame(const uint8_t Game) {
		return std::make_unique<Minigame>(this->Offset(0x1AD), Game);
	};

	/* Get and Set the Mystery Plot unlock state. */
	bool Slot::MysteryPlot() const { return S2GBACore::Sav->ReadBit(this->Offset(0x1CF), 0x0); };
	void Slot::MysteryPlot(const bool V) { S2GBACore::Sav->WriteBit(this->Offset(0x1CF), 0x0, V); };

	/* Get and Set the Friendly Plot unlock state. */
	bool Slot::FriendlyPlot() const { return S2GBACore::Sav->ReadBit(this->Offset(0x1CF), 0x1); };
	void Slot::FriendlyPlot(const bool V) { S2GBACore::Sav->WriteBit(this->Offset(0x1CF), 0x1, V); };

	/* Get and Set the Romance Plot unlock state. */
	bool Slot::RomanticPlot() const { return S2GBACore::Sav->ReadBit(this->Offset(0x1CF), 0x2); };
	void Slot::RomanticPlot(const bool V) { S2GBACore::Sav->WriteBit(this->Offset(0x1CF), 0x2, V); };

	/* Get and Set the Intimidate Plot unlock state. */
	bool Slot::IntimidatingPlot() const { return S2GBACore::Sav->ReadBit(this->Offset(0x1CF), 0x3); };
	void Slot::IntimidatingPlot(const bool V) { S2GBACore::Sav->WriteBit(this->Offset(0x1CF), 0x3, V); };

	/* Get and Set the Motorbike aka "The Chopper" unlock state */
	bool Slot::TheChopperPlot() const { return S2GBACore::Sav->ReadBit(this->Offset(0x1CF), 0x4); };
	void Slot::TheChopperPlot(const bool V) { S2GBACore::Sav->WriteBit(this->Offset(0x1CF), 0x4, V); };

	/* Get and Set the Weirdness Plot unlock state. */
	bool Slot::WeirdnessPlot() const { return S2GBACore::Sav->ReadBit(this->Offset(0x1CF), 0x5); };
	void Slot::WeirdnessPlot(const bool V) { S2GBACore::Sav->WriteBit(this->Offset(0x1CF), 0x5, V); };

	/* Get and Set the Motorbike aka "The Chopper" color. */
	uint8_t Slot::TheChopperColor() const { return S2GBACore::Sav->ReadBits(this->Offset(0x1F2), true); };
	void Slot::TheChopperColor(const uint8_t V) { S2GBACore::Sav->WriteBits(this->Offset(0x1F2), true, std::min<uint8_t>(9, V)); };

	/* Get an Episode class. */
	std::unique_ptr<Episode> Slot::_Episode(const uint8_t EP) const {
		return std::make_unique<Episode>(this->Slt, EP, S2GBACore::Sav->Read<uint8_t>(this->Offs + 0xD6));
	};

	/* Get a Social Move class. */
	std::unique_ptr<SocialMove> Slot::_SocialMove(const uint8_t Move) const {
		return std::make_unique<SocialMove>(this->Offset(0x3EE) + (std::min<uint8_t>(14, Move)) * 0x8, Move);
	};

	/* Get a Cast class. */
	std::unique_ptr<Cast> Slot::_Cast(const uint8_t CST) const {
		return std::make_unique<Cast>(this->Offset(0x466) + (std::min<uint8_t>(25, CST)) * 0xA, CST);
	};

	/*
		Fix the Checksum of the current Slot, if invalid.

		Returns false if already valid, true if got fixed.
	*/
	bool Slot::FixChecksum() {
		const uint16_t CurCHKS = S2GBACore::Sav->Read<uint16_t>(this->Offs + 0xFFE);
		const uint16_t Calced = Checksum::Calc(S2GBACore::Sav->GetData(), this->Offs / 2, (this->Offs + 0xFFE) / 2, false);

		/* If the calced result is NOT the current checksum. */
		if (Calced != CurCHKS) {
			S2GBACore::Sav->Write<uint16_t>(this->Offs + 0xFFE, Calced);
			return true;
		}

		return false;
	};


	/*
		//////////////////////////////////////////////////////////
		The Sims 2 GBA Social Move Save Editing class implementation.
		Main Contributor: SuperSaiyajinStackZ.
		Last Updated: 21 October 2021.
		//////////////////////////////////////////////////////////
	*/

	/* Get and Set the Social Move Flag. */
	SocialMoveFlag SocialMove::Flag() const { return (SocialMoveFlag)S2GBACore::Sav->Read<uint8_t>(this->Offs); };
	void SocialMove::Flag(const SocialMoveFlag V) { S2GBACore::Sav->Write<uint8_t>(this->Offs, (uint8_t)V); };

	/* Get and Set the Social Move Level. */
	uint8_t SocialMove::Level() const { return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x4); };
	void SocialMove::Level(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x4, std::min<uint8_t>(3, V)); };

	/* Get and Set the Social Move Blocked Hours. */
	uint8_t SocialMove::BlockedHours() const { return S2GBACore::Sav->Read<uint8_t>(this->Offs + 0x6); };
	void SocialMove::BlockedHours(const uint8_t V) { S2GBACore::Sav->Write<uint8_t>(this->Offs + 0x6, V); };
};