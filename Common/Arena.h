#pragma once

#include "Common.h"

struct Arena
{
	void* mAllocation = nullptr;
	void* mTop = nullptr;
	void* mTip = nullptr;
	bool bInitialized = false;

	void Init(void* data, u64 size)
	{
    		mAllocation = data;

    		mTip = (u8*)mAllocation + size;
    		mTop = (u8*)mAllocation;
         bInitialized = true;
	}

	static Arena Create(void* data, u64 size)
	{
	   Arena arena;
		arena.Init(data, size);
	   return arena;
	}

	u64 Size() const
	{
         return (u8*)mTip - (u8*)mAllocation;
	}

	template<typename T>
	T* Allocate()
	{
	   return (T*)Allocate(sizeof(T));
	}

	void* Allocate(u64 size)
	{
		CHECK(mAllocation != nullptr && mTop != nullptr && mTip != nullptr)
		CHECK(size > 0)

		void* result = Align16(mTop);
		void* newTop = (u8*)result + size;
		CHECK(newTop <= mTip)

		mTop = newTop;

		return result;
	}

	bool Contains(void* ptr, u64 size = 0) const
	{
		return ptr >= mAllocation && (u8*)(ptr) + size <= mTop;
	}

	void Clear() { mTop = mAllocation; }
};

#define AllocateStaticArena(NAME, SIZE) Arena NAME; u8 _##NAME##Data[(SIZE)]; NAME.Init(_##NAME##Data, (SIZE))
