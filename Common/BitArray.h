#pragma once

#include "Common.h"
#include "Arena.h"
#include <functional>

struct BitArray
{
	static constexpr u32 ElementBitSize = sizeof(u32) * 8;

	/* Data */
	u32* mData = nullptr;
	u32 mBitSize = 0;
	u32 mNumWords = 0;

	/* Interface */
	static u32 CalculateNumWords(u32 numBits) { return (numBits / ElementBitSize + 1); }
	static u32 CalculateSize(u32 numBits) { return CalculateNumWords(numBits) * sizeof(u32); }

	void Init(Arena& arena, u32 numBits)
	{
		mNumWords = CalculateNumWords(numBits);
		mData = (u32*)arena.Allocate(mNumWords);
		mBitSize = numBits;

		MemZero(mData, mNumWords * sizeof(u32));
	}

	void Init(void* data, u32 numBits)
	{
		mData = (u32*)data;
		mBitSize = numBits;
		mNumWords = CalculateNumWords(numBits);

		MemZero(mData, mNumWords * sizeof(u32));
	}

	bool Get(u32 bitIndex)
	{
		CHECK(bitIndex < mBitSize);
		return (mData[GetElementIndex(bitIndex)] & (1 << GetElementBitIndex(bitIndex))) != 0;
	}

	void Set(u32 bitIndex, bool value)
	{
		CHECK(bitIndex < mBitSize);

		const u32 elementIndex = GetElementIndex(bitIndex);
		const u32 elementBitIndex = GetElementBitIndex(bitIndex);

		if (value)
		{
			mData[elementIndex] |= (1 << elementBitIndex);
		}
		else
		{
			mData[elementIndex] &= ~(1 << elementBitIndex);
		}
	}

	template <typename FuncType>
	void IterateOverEnabledBits(FuncType func) const
	{
		for (u32 dataIndex = 0; dataIndex < mNumWords; ++dataIndex)
		{
			u32 dataValue = mData[dataIndex];
			while (dataValue != 0)
			{
				const u32 temp = dataValue & u32(-i32(dataValue));
				const u32 trailingZeros = CountTrailingZeros(dataValue);

				const u32 bitIndex = dataIndex * 32 + trailingZeros;

				if (bitIndex < mBitSize)
				{
 					std::invoke(func, bitIndex);
				}

				dataValue ^= temp;
			}
		}
	}

	i32 FindFirstZero() const
	{
 		for (u32 dataIndex = 0; dataIndex < mNumWords; ++dataIndex)
 		{
         if (const u32 trainlingOnes = CountTrailingOnes(mData[dataIndex]);
            trainlingOnes < 32)
         {
            return SafeTrunc<i32>(dataIndex * 32 + trainlingOnes);
         }
 		}

      return INDEX_NONE;
	}

	/* Internal helpers */
	u32 GetElementIndex(u32 bitIndex) const
	{
		CHECK(bitIndex < mBitSize)
		return bitIndex / ElementBitSize;
	}

	u32 GetElementBitIndex(u32 bitIndex) { return bitIndex % ElementBitSize; }
};
