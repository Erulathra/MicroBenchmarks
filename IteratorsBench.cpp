#include "Common/Arena.h"
#include "Common/BitArray.h"
#include "Common/Common.h"
#include "Common/Stack.h"

#define dataSetSize (1 << 16)
#define numTests (1 << 16)
#define arenaSize (2 * kGibi)

i32 main()
{
	u64 seed = QueryPerfCounter();
	Arena arena = Arena::Create(malloc(arenaSize), arenaSize);

	BitArray bitArray = BitArray::Create(arena, 128);

	u64 forEachBitsetTics = 0;
	u64 iteratorBitsetTics = 0;
	u64 rangeBitsetTics = 0;

	u64 ortodoxTics = 0;
	u64 forEachArrayTics = 0;
	u64 forEachFuncArrayTics = 0;
	u64 iteratorArrayTics = 0;
	u64 rangeArrayTics = 0;

	/* Bit set test */
	for (u32 testId = 0; testId < numTests; ++testId)
	{
		if (testId % 1024 == 0)
		{
			printf("It: %u/%u\n", testId, numTests);
		}

		arena.Clear();

		BitArray bitArray = BitArray::Create(arena, dataSetSize);

		/* Randomize data */
		TStaticStack<u32, dataSetSize> randomNumbers;
		TStaticStack<u32, dataSetSize> baseIndexes;

		{
			for (u32 i = 0; i < dataSetSize; ++i)
			{
				seed = SplitMix64(seed);
				if (seed % 4 == 0)
				{
					bitArray.Set(i, true);
					baseIndexes.Push(i);
				}

				randomNumbers.Push(seed % 1024);
			}

			/* For Each template */
			TStaticStack<u32, dataSetSize> forEachResult;
			{
				const u64 start = QueryPerfCounter();
				bitArray.IterateOverEnabledBits(
					[&](u32 index)
					{
						forEachResult.Push(index);
					}
				);
				const u64 end = QueryPerfCounter();
				forEachBitsetTics += end - start;
			}

			/* Range based loop */
			TStaticStack<u32, dataSetSize> rangeResult;
			{
				const u64 start = QueryPerfCounter();
				for (auto index : EnabledBitsRange(bitArray))
				{
					rangeResult.Push(index);
				}
				const u64 end = QueryPerfCounter();
				rangeBitsetTics += end - start;
			}

			/* Iterator */
			TStaticStack<u32, dataSetSize> iteratorResult;
			{
				const u64 start = QueryPerfCounter();
				for (EnabledBitsIterator it(bitArray); it; ++it)
				{
					iteratorResult.Push(*it);
				}
				const u64 end = QueryPerfCounter();
				iteratorBitsetTics += end - start;
			}

			/* Results Validation */
			CHECK(
				baseIndexes.mSize == forEachResult.mSize && baseIndexes.mSize == iteratorResult.mSize
				&& baseIndexes.mSize == rangeResult.mSize
			)

			for (u32 i = 0; i < baseIndexes.mSize; ++i)
			{
				CHECK(
					baseIndexes[i] == forEachResult[i] && baseIndexes[i] == iteratorResult[i]
					&& baseIndexes[i] == rangeResult[i]
				)
			}
		}

		{
			/* Ortodox iteration */
			u32 ortodoxSum = 0;
			{
				const u64 start = QueryPerfCounter();
				for (u32 i = 0; i < randomNumbers.mSize; ++i)
				{
					ortodoxSum += randomNumbers[i];
				}
				const u64 end = QueryPerfCounter();
				ortodoxTics += end - start;
			}

			/* For Each template */
			u32 forEachSum = 0;
			{
				const u64 start = QueryPerfCounter();
				randomNumbers.ForEach(
					[&forEachSum](u32& data)
					{
						forEachSum += data;
					}
				);
				const u64 end = QueryPerfCounter();
				forEachArrayTics += end - start;
			}

			/* For Each func */
			u32 forEachFuncSum = 0;
			{
				const u64 start = QueryPerfCounter();
				randomNumbers.ForEachFunc(
					[&forEachFuncSum](u32& data)
					{
						forEachFuncSum += data;
					}
				);
				const u64 end = QueryPerfCounter();
				forEachFuncArrayTics += end - start;
			}

			/* Iterator */
			u32 iteratorSum = 0;
			{
				const u64 start = QueryPerfCounter();
				for (auto It = randomNumbers.begin(); It != randomNumbers.end(); ++It)
				{
					iteratorSum += *It;
				}
				const u64 end = QueryPerfCounter();
				iteratorArrayTics += end - start;
			}

			/* Range based loop */
			u32 rangeSum = 0;
			{
				const u64 start = QueryPerfCounter();
				for (const u32 data : randomNumbers)
				{
					rangeSum += data;
				}
				const u64 end = QueryPerfCounter();
				rangeArrayTics += end - start;
			}

			CHECK(ortodoxSum == forEachSum && forEachSum == forEachFuncSum && forEachFuncSum == iteratorSum && iteratorSum == rangeSum)
		}
	}

	const double avgTimeForEach = (double)forEachBitsetTics / (double)numTests;
	const double avgTimeIterator = (double)iteratorBitsetTics / (double)numTests;
	const double avgTimeRange = (double)rangeBitsetTics / (double)numTests;

	printf("\n\n");
	printf("[Bitset]\n");
	printf("ForEach: %f\n", avgTimeForEach);
	printf("Iterator: %f\n", avgTimeIterator);
	printf("Range: %f\n", avgTimeRange);

	const double avgTimeOrotodoxArray = (double)ortodoxTics / (double)numTests;
	const double avgTimeForEachArray = (double)forEachArrayTics / (double)numTests;
	const double avgTimeForEachFuncArray = (double)forEachFuncArrayTics / (double)numTests;
	const double avgTimeIteratorArray = (double)iteratorArrayTics / (double)numTests;
	const double avgTimeRangeArray = (double)rangeArrayTics / (double)numTests;

	printf("\n");
	printf("[Array]\n");
	printf("Classic fori loop: %f\n", avgTimeOrotodoxArray);
	printf("ForEach: %f\n", avgTimeForEachArray);
	printf("ForEach[std::function]: %f\n", avgTimeForEachFuncArray);
	printf("Iterator: %f\n", avgTimeIteratorArray);
	printf("Range: %f\n", avgTimeRangeArray);
}
