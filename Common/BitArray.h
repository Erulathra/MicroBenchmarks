#pragma once

#include "Common.h"
#include "Arena.h"
#include <functional>

struct BitArray;


struct BitArray
{
	/* Data */
	u32* mData = nullptr;
	u32 mBitSize = 0;
	u32 mNumWords = 0;

	/* Interface */
	static u32 CalculateNumWords(u32 numBits) { return (numBits / 32 + 1); }
	static u32 CalculateSize(u32 numBits) { return CalculateNumWords(numBits) * sizeof(u32); }

	static BitArray Create(Arena& arena, u32 numBits)
	{
		BitArray result;
		result.Init(arena, numBits);

		return result;
	}

	static BitArray Create(void* data, u32 numBits)
	{
		BitArray result;
		result.Init(data, numBits);

		return result;
	}

	void Init(Arena& arena, u32 numBits)
	{
		mNumWords = CalculateNumWords(numBits);
		mData = (u32*)arena.Allocate(mNumWords);
		mBitSize = numBits;

		MemZero(mData, mNumWords * sizeof(u32));
	}

	void Init(void* data, u32 numBits)
	{
		CHECK(data)

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
	void IterateOverEnabledBits(const FuncType& func) const
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

	i32 FindFirstOne() const
	{
 		for (u32 dataIndex = 0; dataIndex < mNumWords; ++dataIndex)
 		{
         if (const u32 trainlingOnes = CountTrailingZeros(mData[dataIndex]);
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
		return bitIndex / 32;
	}

	u32 GetElementBitIndex(u32 bitIndex) { return bitIndex % 32; }
};

struct EnabledBitsIterator
{
	/* Data */
	BitArray* mBitArray = nullptr;
	u32 mWordIndex = 0;
	u32 mWord = 0;
	u32 mCurrentIndex = 0;

	/* Interaface */
	EnabledBitsIterator() = delete;
	explicit EnabledBitsIterator(BitArray& bitArray)
	{
		mBitArray = &bitArray;
		mWordIndex = 0;
		mWord = mBitArray->mData[0];

		// Jump to first set bit
		++(*this);
	}

	static EnabledBitsIterator CreateEnd(BitArray& bitArray)
	{
		EnabledBitsIterator result{bitArray};
		result.mBitArray = &bitArray;
		result.mWordIndex = bitArray.mNumWords;
		result.mWord = 0;

		return result;
	}

	u32 operator*() const
	{
		return mCurrentIndex;
	}
	explicit operator bool() const
	{
		return mWord != 0 || mWordIndex < mBitArray->mNumWords;
	}
	bool operator==(const EnabledBitsIterator& rhs) const
	{
		return mBitArray == rhs.mBitArray && mWordIndex == rhs.mWordIndex && mWord == rhs.mWord;
	}
	bool operator!=(const EnabledBitsIterator& rhs) const
	{
		return !(*this == rhs);
	}

	EnabledBitsIterator& operator++()
	{
		while (mWordIndex < mBitArray->mNumWords)
		{
			if (mWord != 0)
			{
				const u32 temp = mWord & u32(-i32(mWord));
				const u32 trailingZeros = CountTrailingZeros(mWord);

				mWord ^= temp;

				// We found one, stop here.
				mCurrentIndex = mWordIndex * 32 + trailingZeros;
				return *this;
			}

			mWordIndex++;
			mWord = mBitArray->mData[mWordIndex];
		}

		return *this;
	}
};

struct EnabledBitsRange
{
	BitArray* mBitArray = nullptr;

	EnabledBitsRange() = delete;
	explicit EnabledBitsRange(BitArray& bitArray)
	{
		mBitArray = &bitArray;
	}

	EnabledBitsIterator begin() const
	{
		return EnabledBitsIterator(*mBitArray);
	}

	EnabledBitsIterator end() const
	{
		return EnabledBitsIterator::CreateEnd(*mBitArray);
	}
};
