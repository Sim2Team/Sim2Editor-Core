/*
*   This file is part of S2GBACore
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

#ifndef _S2GBA_CORE_HPP
#define _S2GBA_CORE_HPP


#include <cstring> // memcpy(...).
#include <math.h> // std::min<>(), std::max<>(...).
#include <memory> // std::unique_ptr<>.
#include <string> // std::string.
#include <vector> // std::vector<>.


namespace S2GBACore {
	/* Declare all used enum classes here. */
	enum class CastFeeling : uint8_t { Neutral = 0x0, Friendly = 0x1, Angry = 0x2, Romantic = 0x3 }; // Cast Feelings.
	enum class HouseItemDirection : uint8_t { Right = 0x1, Down = 0x3, Left = 0x5, Up = 0x7, Invalid = 0xFF }; // House Item Directions.
	enum class Langs : uint8_t { EN = 0x0, NL = 0x1, FR = 0x2, DE = 0x3, IT = 0x4, ES = 0x5 }; // Settings Languages.
	enum class SocialMoveFlag : uint8_t { Locked = 0x0, Unlocked = 0x1, Blocked = 0x2 }; // Social Move Flags.


	/* Declare all used classes here. */
	class Cast;
	class Episode;
	class House;
	class HouseItem;
	class ItemPackage;
	class Minigame;
	class SAV;
	class Settings;
	class Slot;
	class SocialMove;


	/* Some used pointers / objects from the Core. */
	extern std::unique_ptr<SAV> Sav;


	/*
		The Sims 2 GBA Checksum namespace implementation.
		Main Author: SuperSaiyajinStackZ.

		Just containing a function, which handles the Checksum calculation, that returns an uint16_t.
	*/
	namespace Checksum {
		uint16_t Calc(const uint8_t *Buffer, const uint16_t StartOffs, const uint16_t EndOffs, const bool SettingsChks);
	};


	/*
		The Sims 2 GBA SaveHandler namespace implementation.
		Main Author: SuperSaiyajinStackZ.

		This is used to load a save and some other utility functions.
	*/
	namespace SaveHandler {
		bool LoadSav(const std::string &File);
		bool LoadSav(std::unique_ptr<uint8_t[]> &Data, const uint32_t Size); // Overload function for direct pointer passing.
		bool WriteBack(const std::string &File);
	};


	/*
		The Sims 2 GBA SimUtils namespace implementation.
		Main Author: SuperSaiyajinStackZ.

		This is used for some formatting related things.
	*/
	namespace SimUtils {
		const std::string TimeString(const uint16_t Time, const bool AMPM = false);
		const std::string SimoleonsString(const uint32_t Simoleons, const bool SimoleonSignAfter = true);
		const std::string RatingString(const uint16_t Ratings);
	};


	/*
		The Sims 2 GBA Strings namespace implementation.
		Main Author: SuperSaiyajinStackZ.

		This is used for some string related things.
	*/
	namespace Strings {
		extern const std::vector<std::string> CastNames; // Casts.
		extern const std::vector<std::string> SocialMoveNames; // Social Moves.
		extern const std::vector<std::string> EpisodeNames; // Episodes.
		extern const std::vector<std::string> SkillPointNames; // Skill Points.
		extern const std::vector<std::string> ItemNames; // Item Names.
		extern const std::vector<std::string> MinigameNames; // Minigames.
	};

	/*
		The Sims 2 GBA Cast Save Editing class implementation.
		Main Author: SuperSaiyajinStackZ.
	*/
	class Cast {
	public:
		Cast(const uint32_t Offs, const uint8_t Cst)
			: Cst(Cst), Offs(Offs) { };

		uint8_t Index() const { return this->Cst; };

		/* Interaction Levels. */
		uint8_t Friendly() const;
		void Friendly(const uint8_t V);
		uint8_t Romance() const;
		void Romance(const uint8_t V);
		uint8_t Intimidate() const;
		void Intimidate(const uint8_t V);

		/* Feeling Stuff. */
		CastFeeling Feeling() const;
		void Feeling(const CastFeeling V);
		uint8_t FeelingEffectHours() const;
		void FeelingEffectHours(const uint8_t V);

		/* Unlockables. */
		bool RegisteredOnPhone() const;
		void RegisteredOnPhone(const bool V);
		bool Secret() const;
		void Secret(const bool V);
	private:
		uint8_t Cst = 0;
		uint32_t Offs = 0;
	};


	/*
		The Sims 2 GBA Episode Save Editing class implementation.
		Main Author: SuperSaiyajinStackZ.
	*/
	class Episode {
	public:
		Episode(const uint8_t Slt, const uint8_t EP, const uint8_t Move = 0x0)
			: EP(EP), Offs((Slt * 0x1000) + this->SetOffset(std::min<uint8_t>(Move, 10))) { };

		uint8_t Index() const { return this->EP; };

		uint8_t Rating(const uint8_t Category) const;
		void Rating(const uint8_t Category, const uint8_t V);

		bool State() const;
		void State(const bool V);
	private:
		uint8_t EP = 0;
		uint32_t Offs = 0;

		static constexpr uint32_t EPOffs[11] = { 0x104, 0x10E, 0x122, 0x11D, 0x131, 0x127, 0x14A, 0x140, 0x118, 0x16D, 0x168 }; // 11 Episodes.

		/* Sets the base offset for the Episodes. */
		uint32_t SetOffset(const uint8_t Move) const { return this->EPOffs[this->EP] + (Move * 0x6); };
	};


	/*
		The Sims 2 GBA House Save Editing class implementation.
		Main Author: SuperSaiyajinStackZ.
	*/
	class House {
	public:
		House(const uint32_t Offset)
			: Offs(Offset) { };

		uint8_t Roomdesign() const;
		void Roomdesign(const uint8_t V);

		std::unique_ptr<HouseItem> Items() const;
	private:
		uint32_t Offs = 0;
	};


	/*
		The Sims 2 GBA HouseItem Save Editing class implementation.
		Main Author: SuperSaiyajinStackZ.
	*/
	class HouseItem {
	public:
		HouseItem(const uint32_t Offset)
			: Offs(Offset) { };

		uint8_t Count() const;
		void Count(const uint8_t V);

		uint8_t ID(const uint8_t Index) const;
		void ID(const uint8_t Index, const uint8_t V);

		/* Flag and Use count. */
		uint8_t Flag(const uint8_t Index) const;
		void Flag(const uint8_t Index, const uint8_t V);
		uint8_t UseCount(const uint8_t Index) const;
		void UseCount(const uint8_t Index, const uint8_t V);

		/* Positions. */
		uint8_t XPos(const uint8_t Index) const;
		void XPos(const uint8_t Index, const uint8_t V);
		uint8_t YPos(const uint8_t Index) const;
		void YPos(const uint8_t Index, const uint8_t V);

		HouseItemDirection Direction(const uint8_t Index) const;
		void Direction(const uint8_t Index, const HouseItemDirection V);

		/* Add and Remove. */
		bool AddItem(const uint8_t ID, const uint8_t Flag, const uint8_t UseCount, const uint8_t XPos, const uint8_t YPos, const HouseItemDirection Direction);
		bool RemoveItem(const uint8_t Index);
	private:
		uint32_t Offs = 0;
	};


	/*
		The Sims 2 GBA Item Package Save Editing class implementation.
		Main Author: SuperSaiyajinStackZ.
	*/
	class ItemPackage {
	public:
		ItemPackage(const uint32_t Offset)
			: Offs(Offset) { };

		uint8_t Count() const;
		void Count(const uint8_t V);

		uint8_t ID(const uint8_t Index) const;
		void ID(const uint8_t Index, const uint8_t V);

		/* Flag and Use count. */
		uint8_t Flag(const uint8_t Idx) const;
		void Flag(const uint8_t Idx, const uint8_t V);
		uint8_t UseCount(const uint8_t Idx) const;
		void UseCount(const uint8_t Idx, const uint8_t V);
	private:
		uint32_t Offs = 0;
	};


	/*
		The Sims 2 GBA Minigame Save Editing class implementation.
		Main Author: SuperSaiyajinStackZ.
	*/
	class Minigame {
	public:
		Minigame(const uint32_t Offs, const uint8_t Game)
			: Game(std::min<uint8_t>(6, Game)), Offs(Offs) { };

		uint8_t Index() const { return this->Game; };

		bool Played() const;
		void Played(const bool V);

		uint8_t Level() const;
		void Level(const uint8_t V, const bool MetaData = false);
	private:
		uint8_t Game = 0;
		uint32_t Offs = 0;
	};


	/*
		The Sims 2 GBA SAV Save Editing class implementation.
		Main Author: SuperSaiyajinStackZ.

		NOTE: NEVER ACCESS THIS CLASS AND OTHER SUB CLASSES OUTSIDE THE S2GBACore::SaveHandler AND S2GBACore::Sav CALL!!!
	*/
	class SAV {
	public:
		SAV(const std::string &File); // Way 1 with a file.
		SAV(std::unique_ptr<uint8_t[]> &Data, const uint32_t Size)
			: SavData(std::move(Data)), SavSize(Size) { this->SavValid = this->ValidationCheck(); }; // Way 2 with a Buffer.

		bool ValidationCheck();
		uint8_t *GetData() const { return this->SavData.get(); };
		uint32_t GetSize() const { return this->SavSize; };
		bool GetChangesMade() const { return this->ChangesMade; };
		bool GetValid() const { return this->SavValid; };

		/* Reads stuff from the Save Buffer. */
		template<class T> T Read(const uint32_t Offs) const {
			if (!this->GetValid() || !this->GetData()) return 0;

			T Temp;
			memcpy(&Temp, this->SavData.get() + Offs, sizeof(T));
			return Temp;
		};

		/* Writes stuff to the Save Buffer. */
		template<class T> void Write(const uint32_t Offs, T Data) {
			if (!this->GetValid() || !this->GetData()) return; // Do nothing.

			/* NOTE: This only works with `>>` operator types. */
			for (size_t Idx = 0; Idx < sizeof(T); Idx++) {
				this->SavData.get()[Offs + Idx] = (uint8_t)Data;
				Data >>= 8; // Go to the next byte.
			};

			if (!this->ChangesMade) this->ChangesMade = true;
		};

		/* Some other Read and Writes. */
		bool ReadBit(const uint32_t Offs, const uint8_t BitIndex) const;
		void WriteBit(const uint32_t Offs, const uint8_t BitIndex, const bool IsSet);
		uint8_t ReadBits(const uint32_t Offs, const bool First) const;
		void WriteBits(const uint32_t Offs, const bool First, const uint8_t Data);
		std::string ReadString(const uint32_t Offs, const uint32_t Length) const;
		void WriteString(const uint32_t Offs, const uint32_t Length, const std::string &Str);

		bool SlotExist(const uint8_t Slot) const;
		std::unique_ptr<Slot> _Slot(const uint8_t Slt) const;
		std::unique_ptr<Settings> _Settings() const;
		void Finish();
	private:
		std::unique_ptr<uint8_t[]> SavData;
		uint32_t SavSize = 0;
		bool ChangesMade = false, SavValid = false;
		static constexpr uint8_t GBAIdent[7] = { 0x53, 0x54, 0x57, 0x4E, 0x30, 0x32, 0x34 };
	};


	/*
		The Sims 2 GBA Settings Save Editing class implementation.
		Main Author: SuperSaiyajinStackZ.
	*/
	class Settings {
	public:
		Settings() { };

		/* Volume Levels. */
		uint8_t SFX() const;
		void SFX(const uint8_t V);
		uint8_t Music() const;
		void Music(const uint8_t V);

		Langs Language() const;
		void Language(const Langs V);

		void UpdateChecksum();
	private:
		static constexpr uint8_t MusicLevels[11] = { 0x0, 0x19, 0x32, 0x4B, 0x64, 0x7D, 0x96, 0xAF, 0xC8, 0xE1, 0xFF };
		static constexpr uint8_t SFXLevels[11]   = { 0x0, 0x0C, 0x18, 0x24, 0x30, 0x3C, 0x48, 0x54, 0x60, 0x6C, 0x80 };
	};


	/*
		The Sims 2 GBA Slot Save Editing class implementation.
		Main Author: SuperSaiyajinStackZ.
	*/
	class Slot {
	public:
		Slot(const uint8_t Slt)
			: Slt(Slt), Offs(Slt * 0x1000) { };

		/* Main things. */
		uint8_t Hour() const;
		void Hour(const uint8_t V);
		uint8_t Minute() const;
		void Minute(const uint8_t V);
		uint8_t Seconds() const;
		void Seconds(const uint8_t V);
		uint32_t Simoleons() const;
		void Simoleons(const uint32_t V);
		uint16_t Ratings() const;
		void Ratings(const uint16_t V);
		std::string Name() const;
		void Name(const std::string &V);

		/* Appearance. */
		uint8_t Hairstyle() const;
		void Hairstyle(const uint8_t V);
		uint8_t Shirtcolor3() const;
		void Shirtcolor3(const uint8_t V);
		uint8_t Tan() const;
		void Tan(const uint8_t V);
		uint8_t Shirtcolor2() const;
		void Shirtcolor2(const uint8_t V);
		uint8_t Haircolor() const;
		void Haircolor(const uint8_t V);
		uint8_t Hatcolor() const;
		void Hatcolor(const uint8_t V);
		uint8_t Shirt() const;
		void Shirt(const uint8_t V);
		uint8_t Shirtcolor1() const;
		void Shirtcolor1(const uint8_t V);
		uint8_t Pants() const;
		void Pants(const uint8_t V);
		uint8_t Pantscolor() const;
		void Pantscolor(const uint8_t V);

		/* Skill Points. */
		uint8_t Confidence() const;
		void Confidence(const uint8_t V);
		uint8_t Mechanical() const;
		void Mechanical(const uint8_t V);
		uint8_t Strength() const;
		void Strength(const uint8_t V);
		uint8_t Personality() const;
		void Personality(const uint8_t V);
		uint8_t Hotness() const;
		void Hotness(const uint8_t V);
		uint8_t Intellect() const;
		void Intellect(const uint8_t V);
		uint8_t Sanity() const;
		void Sanity(const uint8_t V);
		uint8_t Aspiration() const;
		void Aspiration(const uint8_t V);

		/* Items. */
		std::unique_ptr<ItemPackage> PawnShop() const;
		std::unique_ptr<ItemPackage> Saloon() const;
		std::unique_ptr<ItemPackage> Skills() const;
		std::unique_ptr<ItemPackage> Mailbox() const;
		std::unique_ptr<ItemPackage> Inventory() const;

		/* House data. */
		std::unique_ptr<House> _House() const;

		/* Collectables Amount. */
		uint8_t Cans() const;
		void Cans(const uint8_t V);
		uint8_t Cowbells() const;
		void Cowbells(const uint8_t V);
		uint8_t Spaceship() const;
		void Spaceship(const uint8_t V);
		uint8_t Fuelrods() const;
		void Fuelrods(const uint8_t V);

		/* Collectables Price. */
		uint8_t CansPrice() const;
		void CansPrice(const uint8_t V);
		uint8_t CowbellsPrice() const;
		void CowbellsPrice(const uint8_t v);
		uint8_t SpaceshipPrice() const;
		void SpaceshipPrice(const uint8_t V);
		uint8_t FuelrodsPrice() const;
		void FuelrodsPrice(const uint8_t V);

		/* Episode stuff. */
		uint8_t CurrentEpisode() const;
		void CurrentEpisode(const uint8_t V, const bool ValidCheck = true);

		/* Minigames. */
		std::unique_ptr<Minigame> _Minigame(const uint8_t Game);

		/* Plot Points stuff. */
		bool MysteryPlot() const;
		void MysteryPlot(const bool V);
		bool FriendlyPlot() const;
		void FriendlyPlot(const bool V);
		bool RomanticPlot() const;
		void RomanticPlot(const bool V);
		bool IntimidatingPlot() const;
		void IntimidatingPlot(const bool V);
		bool TheChopperPlot() const;
		void TheChopperPlot(const bool V);
		bool WeirdnessPlot() const;
		void WeirdnessPlot(const bool V);

		uint8_t TheChopperColor() const;
		void TheChopperColor(const uint8_t V);

		/* Some class pointers. */
		std::unique_ptr<Episode> _Episode(const uint8_t EP) const;
		std::unique_ptr<SocialMove> _SocialMove(const uint8_t Move) const;
		std::unique_ptr<Cast> _Cast(const uint8_t CST) const;

		bool FixChecksum();
	private:
		uint8_t Slt = 0;
		uint32_t Offs = 0;

		uint32_t Offset(const uint32_t DefaultOffs) const;

		/* This contains all official Episode Values found at offset (Slot * 0x1000) + 0x1A9. */
		static constexpr uint8_t EPVals[12] = {
			0x0, 0x1, 0x3, 0x7, // Tutorial + Season 1.
			0x6, 0xA, 0x8, 0xF, // Season 2.
			0xD, 0x5, 0x16, 0x15 // Season 3.
		};
	};


	/*
		The Sims 2 GBA Social Move Save Editing class implementation.
		Main Author: SuperSaiyajinStackZ.
	*/
	class SocialMove {
	public:
		SocialMove(const uint32_t Offs, const uint8_t Move)
			: Move(Move), Offs(Offs) { };

		uint8_t Index() const { return this->Move; };

		SocialMoveFlag Flag() const;
		void Flag(const SocialMoveFlag V);

		uint8_t Level() const;
		void Level(const uint8_t V);

		uint8_t BlockedHours() const;
		void BlockedHours(const uint8_t V);
	private:
		uint8_t Move = 0;
		uint32_t Offs = 0;
	};
};

#endif