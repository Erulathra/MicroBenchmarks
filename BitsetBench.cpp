#include <cmath>

#include "Common/Arena.h"
#include "Common/BitArray.h"
#include "Common/Common.h"

// NOTE(SS): Some thing, designed to be larger than cache line.
struct Thing
{
	static constexpr u32 numFloats = 126;

	u32 id;
	fp32 data[numFloats];
	bool bFlag;
};

inline void DoSomeFancyWork(Thing& element)
{
	const float random = Hash64ToFloat(SplitMix64(element.id));
	for (u32 i = 0; i < Thing::numFloats - 1; ++i)
	{
		element.data[i] = sin(element.data[i + 1]) + sqrt(random);
	}
}

i32 main()
{
	constexpr u32 dataSetSize = 1 << 16;
	constexpr u32 numTests = 256;
	constexpr u64 arenaSize = 2 * kGibi;

	u64 seed = QueryPerfCounter();

	printf("Starting benchmark \n");

	Arena arena = Arena::Create(malloc(arenaSize), arenaSize);

	for (u32 enabledFlags = 1; enabledFlags < 64; ++enabledFlags)
	{
		printf("Starting test, enabled flags: %f [%i/%i] \n", 1.f / (float)enabledFlags, enabledFlags, 64);

		u64 sequentialTicsSum = 0;
		u64 sequentialBitsetTicsSum = 0;
		u64 sequentialIndexSortedArray = 0;
		u64 sequentialIndexShuffledArray = 0;

		u64 shuffledTicsSum = 0;
		u64 shuffledBitsetTicsSum = 0;
		u64 shuffledIndexSortedArray = 0;
		u64 shuffledIndexShuffledArray = 0;

		for (u32 testId = 0; testId < numTests; ++testId)
		{
			/* Allocate data */
			Thing* const sequentialData = (Thing*)arena.Allocate(dataSetSize * sizeof(Thing));
			Thing** shuffledData = (Thing**)arena.Allocate(dataSetSize * sizeof(Thing*));
			for (u32 i = 0; i < dataSetSize; ++i)
			{
				shuffledData[i] = (Thing*)arena.Allocate(sizeof(Thing));
			}

			/* Shuffle random data */
			for (i32 i = dataSetSize - 1; i > 0; --i)
			{
				seed = SplitMix64(seed);
				u32 randomIndex = seed % (i + 1);

				Swap(shuffledData[i], shuffledData[randomIndex]);
			}

			/* Allocate bitset and index array */
			BitArray flags;
			flags.Init(malloc(BitArray::CalculateSize(dataSetSize)), dataSetSize);

			u32* sortedIndexArray = (u32*)arena.Allocate(dataSetSize * sizeof(u32));
			u32* shuffledIndexArray = (u32*)arena.Allocate(dataSetSize * sizeof(u32));
			u32 indexArraySize = 0;

			/* Create fully random data set */
			for (u32 i = 0; i < dataSetSize; ++i)
			{
				sequentialData[i].id = i;
				shuffledData[i]->id = i;

				seed = SplitMix64(seed);

				const bool flagValue = (seed % enabledFlags) == 0;

				sequentialData[i].bFlag = flagValue;
				shuffledData[i]->bFlag = flagValue;
				flags.Set(i, flagValue);

				if (flagValue)
				{
					sortedIndexArray[indexArraySize] = i;
					indexArraySize++;
				}
			}

			/* Shuffle index array */
			Memcpy(shuffledIndexArray, sortedIndexArray, indexArraySize * sizeof(u32));
			for (i32 i = dataSetSize - 1; i > 0; --i)
			{
				seed = SplitMix64(seed);
				u32 randomIndex = seed % (i + 1);

				Swap(shuffledIndexArray[i], shuffledIndexArray[randomIndex]);
			}

			/* Sequential reading object flag */
			{
				const u64 timeStart = QueryPerfCounter();
				for (u32 i = 0; i < dataSetSize; ++i)
				{
					Thing& element = sequentialData[i];
					if (element.bFlag)
					{
						DoSomeFancyWork(element);
					}
				}
				const u64 timeEnd = QueryPerfCounter();
				sequentialTicsSum += timeEnd - timeStart;
			}

			/* Sequential bitset */
			{
				const u64 timeStart = QueryPerfCounter();
				flags.IterateOverEnabledBits(
					[&](u32 index)
					{
						DoSomeFancyWork(sequentialData[index]);
					}
				);
				const u64 timeEnd = QueryPerfCounter();
				sequentialBitsetTicsSum += timeEnd - timeStart;
			}

			/* Sequential index sorted array */
			{
				const u64 timeStart = QueryPerfCounter();
				for (u32 i = 0; i < indexArraySize; ++i)
				{
				   DoSomeFancyWork(sequentialData[sortedIndexArray[i]]);
				}
				const u64 timeEnd = QueryPerfCounter();
				sequentialIndexSortedArray += timeEnd - timeStart;
			}

			/* Sequential index shuffled array */
			{
				const u64 timeStart = QueryPerfCounter();
				for (u32 i = 0; i < indexArraySize; ++i)
				{
				   DoSomeFancyWork(sequentialData[shuffledIndexArray[i]]);
				}
				const u64 timeEnd = QueryPerfCounter();
				sequentialIndexShuffledArray += timeEnd - timeStart;
			}

			/* random reading flag */
			{
				u64 timeStart = QueryPerfCounter();
				for (u32 i = 0; i < dataSetSize; ++i)
				{
					Thing& element = *shuffledData[i];
					if (element.bFlag)
					{
						DoSomeFancyWork(element);
					}
				}
				u64 timeEnd = QueryPerfCounter();
				shuffledTicsSum += timeEnd - timeStart;
			}

			/* random bitset */
			{
				u64 timeStart = QueryPerfCounter();
				flags.IterateOverEnabledBits(
					[&](u32 index)
					{
						DoSomeFancyWork(*shuffledData[index]);
					}
				);
				u64 timeEnd = QueryPerfCounter();
				shuffledBitsetTicsSum += timeEnd - timeStart;
			}

			/* random index sorted array */
			{
				const u64 timeStart = QueryPerfCounter();
				for (u32 i = 0; i < indexArraySize; ++i)
				{
					DoSomeFancyWork(*shuffledData[sortedIndexArray[i]]);
				}
				const u64 timeEnd = QueryPerfCounter();
				shuffledIndexSortedArray += timeEnd - timeStart;
			}

			/* random index shuffled array */
			{
				const u64 timeStart = QueryPerfCounter();
				for (u32 i = 0; i < indexArraySize; ++i)
				{
					DoSomeFancyWork(*shuffledData[shuffledIndexArray[i]]);
				}
				const u64 timeEnd = QueryPerfCounter();
				shuffledIndexShuffledArray += timeEnd - timeStart;
			}

			arena.Clear();
		}

		const double sequential = (double)sequentialTicsSum / numTests;
		const double sequentialBits = (double)sequentialBitsetTicsSum / numTests;
		const double sequentialBitsRelative = (sequentialBits - sequential) / sequential;
		const double sequentialSortedIndex = (double)sequentialIndexSortedArray / numTests;
		const double sequentialSortedIndexRelative = (sequentialSortedIndex - sequential) / sequential;
		const double sequentialShuffledIndex = (double)sequentialIndexShuffledArray / numTests;
		const double sequentialShuffledIndexRelative = (sequentialShuffledIndex - sequential) / sequential;

		const double shuffled = (double)shuffledTicsSum / numTests;
		const double shuffledBits = (double)shuffledBitsetTicsSum / numTests;
		const double shuffledRelative = (shuffledBits - shuffled) / shuffled;
		const double shuffledSortedIndex = (double)shuffledIndexSortedArray / numTests;
		const double shuffledSortedIndexRelative = (shuffledSortedIndex - shuffled) / shuffled;
		const double shuffledShuffledIndex = (double)shuffledIndexShuffledArray / numTests;
		const double shuffledShuffledIndexRelative = (shuffledShuffledIndex - shuffled) / shuffled;

		printf("Sequential Data \n");
		printf("Iterating Object: %f \n", sequential);
		printf("Iterating Bitset: %f (%f%%) \n", sequentialBits, sequentialBitsRelative * 100.f);
		printf("Iterating Sorted Index Array: %f (%f%%) \n", sequentialSortedIndex, sequentialSortedIndexRelative * 100.f);
		printf("Iterating Shuffled Index Array: %f (%f%%) \n", sequentialShuffledIndex, sequentialShuffledIndexRelative * 100.f);
		printf("Shuffled Data \n");
		printf("Iterating Object: %f \n", shuffled);
		printf("Iterating Bitset: %f (%f%%) \n", shuffledBits, shuffledRelative * 100.f);
		printf("Iterating Sorted Index Array: %f (%f%%) \n", shuffledSortedIndex, shuffledSortedIndexRelative * 100.f);
		printf("Iterating Shuffled Index Array: %f (%f%%) \n", shuffledShuffledIndex, shuffledShuffledIndexRelative * 100.f);
	}

	return 0;
}
