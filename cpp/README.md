# Sim2Editor - CPlusPlus Compressed Cores

Here you'll find C++ variants of the Sim2Editor **Game Boy Advance** and the **Nintendo DS** Core. Both are separate for special version use.

## Notes about the Cores

* Both Cores require C++17 or above to compile.

* Both Cores are NOT THREAD-SAFE!!! So don't even try to use the same things on multiple threads or you may have problems.

* Make sure to read the Header of the `cpp` file of the Core for more information.

* You access Core specific things using `S2GBACore::Sav` for the Game Boy Advance version or `S2NDSCore::Sav` for the Nintendo DS version. There are also some namespaces such as `SimUtils` or `Strings`, so access them using the `S2GBACore::` or `S2NDSCore::` specifier.

* Both Cores support the following ways to load SavData:

- [x] Directly providing a path to a Savefile.
- [x] Directly accessing the SavData through a `std::unique_ptr<uint8_t[]>`. NOTE: The passed `std::unique_ptr` will be a nullptr after using the `SaveHandler LoadSav` function because it's getting moved to `Sav->SavData`.


## Examples

You'll find examples how to use Sim2Editor's CPlusPlus compressed Cores below.

### Example 1: Loading SavData from a File.

The first example shows you, how you load the SavData from a File by providing the Path to it.

```cpp
#include "S2GBACore.hpp"

int main(int ARGC, char *ARGV[]) {
	if (ARGC == 2) { // Ensure 2 arguments have been provided.
		const std::string FName = ARGV[1];

		if (S2GBACore::SaveHandler::LoadSav(FName)) {
			/* SAV is good. */
			std::unique_ptr<S2GBACore::Slot> Slot = S2GBACore::Sav->_Slot(1);

			/* If not nullptr, we loaded the first slot. */
			if (Slot) {
				printf("Name: %s\n", Slot->Name().c_str());
				printf("Simoleons: %d\n", Slot->Simoleons());
				printf("Ratings: %d\n", Slot->Ratings());
				printf("Time: %s\n", S2GBACore::SimUtils::TimeString(Slot->Time()).c_str());
			}

			/* When you're done, use the function below to update checksums etc and write back to file. */
			S2GBACore::SaveHandler::WriteBack(FName);
		}
	}

	return 0;
};
```


### Example 2: Loading SavData from a File as a buffer.

The second example shows the second way, how to load the SavData directly from a buffer - `std::unique_ptr<uint8_t[]>`.

```cpp
#include "S2GBACore.hpp"

int main(int ARGC, char *ARGV[]) {
	if (ARGC == 2) { // Ensure 2 arguments have been provided.
		const std::string FName = ARGV[1];

		FILE *In = fopen(FName.c_str(), "rb");

		if (In) {
			fseek(In, 0, SEEK_END);
			const uint32_t Size = ftell(In);
			fseek(In, 0, SEEK_SET);

			std::unique_ptr<uint8_t[]> Data = std::make_unique<uint8_t[]>(Size);
			fread(Data.get(), 0x1, Size, In);

			/* From now on, the unique_ptr of Data is a nullptr because it gets moved. */
			if (S2GBACore::SaveHandler::LoadSav(Data, Size)) {
				/* SAV is good. */
				std::unique_ptr<S2GBACore::Slot> Slot = S2GBACore::Sav->_Slot(1);

				/* If not nullptr, we loaded the first slot. */
				if (Slot) {
					printf("Name: %s\n", Slot->Name().c_str());
					printf("Simoleons: %d\n", Slot->Simoleons());
					printf("Ratings: %d\n", Slot->Ratings());
					printf("Time: %s\n", S2GBACore::SimUtils::TimeString(Slot->Time()).c_str());
				}

				/* When you're done, use the function below to update checksums etc. */
				S2GBACore::Sav->Finish();

				/*
					Now you can write the edited data back like this for example.
					
					YourData in this example would be a "uint8_t *YourData".

					Alternatively, use memcpy if you want.
				*/
				for (size_t Idx = 0; Idx < S2GBACore::Sav->GetSize(); Idx++) {
					YourData[Idx] = S2GBACore::Sav->GetData()[Idx];
				}
			}

			fclose(In);
		}
	}

	return 0;
};
```