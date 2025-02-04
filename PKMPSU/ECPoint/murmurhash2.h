
//-----------------------------------------------------------------------------
// MurmurHash2 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

// Note - This code makes a few assumptions about how your machine behaves -

// 1. We can read a 4-byte value from any address without crashing
// 2. sizeof(int) == 4

// And it has a few limitations -

// 1. It will not work incrementally.
// 2. It will not produce the same results on little-endian and big-endian
//    machines.
// #ifndef MURMURHASH2_HPP_
// #define MURMURHASH2_HPP_
#pragma once
#include<cstdint>

//-----------------------------------------------------------------------------
// Platform-specific functions and macros

// Microsoft Visual Studio

#if defined(_MSC_VER)

#define BIG_CONSTANT(x) (x)

// Other compilers

#else	// defined(_MSC_VER)

#define BIG_CONSTANT(x) (x##LLU)

#endif // !defined(_MSC_VER)
//
//-----------------------------------------------------------------------------
// Block read - on little-endian machines this is a single load,
// while on big-endian or unknown machines the byte accesses should
// still get optimized into the most efficient instruction.
static inline uint32_t getblock ( const uint32_t * p )
{
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
  return *p;
#else
  const uint8_t *c = (const uint8_t *)p;
  return (uint32_t)c[0] |
	 (uint32_t)c[1] <<  8 |
	 (uint32_t)c[2] << 16 |
	 (uint32_t)c[3] << 24;
#endif
}

static inline uint64_t getblock ( const uint64_t * p )
{
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
  return *p;
#else
  const uint8_t *c = (const uint8_t *)p;
  return (uint64_t)c[0] |
	 (uint64_t)c[1] <<  8 |
	 (uint64_t)c[2] << 16 |
	 (uint64_t)c[3] << 24 |
	 (uint64_t)c[4] << 32 |
	 (uint64_t)c[5] << 40 |
	 (uint64_t)c[6] << 48 |
	 (uint64_t)c[7] << 56;
#endif
}

//-----------------------------------------------------------------------------

uint32_t MurmurHash2 ( const void * key, int len, uint32_t seed );
//-----------------------------------------------------------------------------
// MurmurHash2, 64-bit versions, by Austin Appleby

// The same caveats as 32-bit MurmurHash2 apply here - beware of alignment 
// and endian-ness issues if used across multiple platforms.

// 64-bit hash for 64-bit platforms

const uint64_t fixed_salt64 = 0xAAAAAAAA; 
uint64_t MurmurHash64A(const void * key, int len, uint64_t seed);


// 64-bit hash for 32-bit platforms

uint64_t MurmurHash64B (const void * key, int len, uint64_t seed);

//-----------------------------------------------------------------------------
// MurmurHash2A, by Austin Appleby

// This is a variant of MurmurHash2 modified to use the Merkle-Damgard 
// construction. Bulk speed should be identical to Murmur2, small-key speed 
// will be 10%-20% slower due to the added overhead at the end of the hash.

// This variant fixes a minor issue where null keys were more likely to
// collide with each other than expected, and also makes the function
// more amenable to incremental implementations.

#define mmix(h,k) { k *= m; k ^= k >> r; k *= m; h *= m; h ^= k; }

uint32_t MurmurHash2A ( const void * key, int len, uint32_t seed );

//-----------------------------------------------------------------------------
// CMurmurHash2A, by Austin Appleby

// This is a sample implementation of MurmurHash2A designed to work 
// incrementally.

// Usage - 

// CMurmurHash2A hasher
// hasher.Begin(seed);
// hasher.Add(data1,size1);
// hasher.Add(data2,size2);
// ...
// hasher.Add(dataN,sizeN);
// uint32_t hash = hasher.End()

class CMurmurHash2A
{
public:

  void Begin ( uint32_t seed = 0 );

  void Add ( const unsigned char * data, int len );

  uint32_t End ( void );

private:

  static const uint32_t m = 0x5bd1e995;
  static const int r = 24;

  void MixTail ( const unsigned char * & data, int & len );

  uint32_t m_hash;
  uint32_t m_tail;
  uint32_t m_count;
  uint32_t m_size;
};

//-----------------------------------------------------------------------------
// MurmurHashNeutral2, by Austin Appleby

// Same as MurmurHash2, but endian- and alignment-neutral.
// Half the speed though, alas.

uint32_t MurmurHashNeutral2 ( const void * key, int len, uint32_t seed );

//-----------------------------------------------------------------------------
// MurmurHashAligned2, by Austin Appleby

// Same algorithm as MurmurHash2, but only does aligned reads - should be safer
// on certain platforms. 

// Performance will be lower than MurmurHash2

#define MIX(h,k,m) { k *= m; k ^= k >> r; k *= m; h *= m; h ^= k; }


uint32_t MurmurHashAligned2 ( const void * key, int len, uint32_t seed );

//-----------------------------------------------------------------------------
// #endif
