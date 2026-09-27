#pragma once

#include <bit>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <limits>

// ##################################################################################
//                                      TypeDefs
// ##################################################################################

using i8 = int8_t;
using u8 = uint8_t;
using i16 = int16_t;
using u16 = uint16_t;
using i32 = int32_t;
using u32 = uint32_t;
using i64 = int64_t;
using u64 = uint64_t;

using iPtr = uintptr_t;

using fp32 = float;
using fp64 = double;

using LiteralString = const char*;
using CString = const char*;

#define I16_MIN std::numeric_limits<i16>::min()
#define I16_MAX std::numeric_limits<i16>::max()
#define U16_MAX std::numeric_limits<u16>::max()

#define I32_MIN std::numeric_limits<i32>::min()
#define I32_MAX std::numeric_limits<i32>::max()
#define U32_MAX std::numeric_limits<u32>::max()

#define I64_MIN std::numeric_limits<i64>::min()
#define I64_MAX std::numeric_limits<i64>::max()
#define U64_MAX std::numeric_limits<u64>::max()

#define INDEX_NONE (-1)

// ##################################################################################
//                                        Macros
// ##################################################################################

#define CHECK(CONDITION)                                                                                               \
	if (!(CONDITION)) [[unlikely]]                                                                                     \
	{                                                                                                                  \
		fprintf(stderr, "Assertion `" #CONDITION "` failed.\n");                                                                  \
		abort();                                                                                                       \
	}

// ##################################################################################
//                                        Memory
// ##################################################################################

constexpr u64 kGibi = 1<<30;
constexpr u64 kMebi = 1<<20;
constexpr u64 kKibi = 1<<10;

template<typename T>
constexpr T Align4(T value)
{
   return (T)(((iPtr)value + 3) & ~3);
}

template<typename T>
constexpr T Align16(T value)
{
   return (T)(((iPtr)value + 15) & ~15);
}

inline void MemSet(void* dst, u8 bits, u64 numBytes) { memset(dst, bits, numBytes); }
inline void MemZero(void* dst, u64 numBytes) { MemSet(dst, 0, numBytes); }
inline void Memcpy(void* dst, const void* src, u64 numBytes) { memcpy(dst, src, numBytes); }

// ##################################################################################
//                                        Math
// ##################################################################################

template <typename DstT, typename SrcT>
DstT SafeTrunc(SrcT x)
{
   CHECK(x < std::numeric_limits<DstT>::max())
   CHECK(x >= std::numeric_limits<DstT>::min())

   return (DstT)(x);
}

u64 SplitMix64(u64 seed)
{
	u64 z = (seed += 0x9e3779b97f4a7c15);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
	z = (z ^ (z >> 27)) * 0x94d049bb133111eb;

	return z ^ (z >> 31);
}

float Hash64ToFloat(u64 hash)
{
   return (float)(hash / (double)(U64_MAX));
}

// ##################################################################################
//                                       Algo
// ##################################################################################

template <typename T>
void Swap(T& a, T& b)
{
   T temp = a;
   a = b;
   b = temp;
}

// ##################################################################################
//                                   Bit Operators
// ##################################################################################

inline u32 CountTrailingZeros(u32 value) { return std::countr_zero(value); }
inline u32 CountLeadingZeros(u32 value) { return std::countl_zero(value); }
inline u32 CountTrailingOnes(u32 value) { return std::countr_zero(~value); }
inline u32 CountLeadingOnes(u32 value) { return std::countl_zero(~value); }

// ##################################################################################
//                                   Measuring time
// ##################################################################################

#include "x86intrin.h"
#include "cpuid.h"
#include "chrono"

u64 QueryPerfCounter()
{
   std::chrono::high_resolution_clock::now();
   u32 dummy;
   return __rdtscp(&dummy);
}

u64 CalibratePerfFrequency()
{
   static u64 perfFrequency = 0;

   constexpr u32 numIterations = 5;

   u64 tics = 0;
   if (perfFrequency == 0)
   {
	   for (u32 i = 0; i < numIterations; ++i)
	   {
		   const u64 start = QueryPerfCounter();
		   timespec spec = {
			   .tv_sec = static_cast<i64>(0),
			   .tv_nsec = static_cast<i64>(50000000), // 1ms
		   };
		   nanosleep(&spec, nullptr);
		   const u64 end = QueryPerfCounter();

			tics += end - start;
	   }

		perfFrequency = tics * 20 / numIterations;
   }

   return perfFrequency;
}
