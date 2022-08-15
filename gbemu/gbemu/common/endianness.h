#include "emusys.h"

// Below macros are used for converting
// Little Endian data to Big Endian and vice versa
// This uses MSVC's and GCC's stdlib header
// Using inline implmentations for other compilers
#if defined(__GNUC__)
	#define SWAP_BYTES_64(a) __builtin_bswap64(a)
	#define SWAP_BYTES_32(a) __builtin_bswap32(a)
	#define SWAP_BYTES_16(a) __builtin_bswap16(a)
#elif defined(_MSC_VER)
	#define SWAP_BYTES_64(a) _byteswap_uint64(a)
	#define SWAP_BYTES_32(a) _byteswap_ulong(a)
	#define SWAP_BYTES_16(a) _byteswap_ushort(a)
#else
	inline uint64 SWAP_BYTES_64(uint64 a) {
		uint32 low = (uint32)a, high = (uint32)(a >> 32);
		uint16 lowLow = (uint16)low, lowHigh = (uint16)(low >> 16), highLow = (uint16)high, highHigh = (uint16)(high >> 16);

		return ((uint64)(((uint32)(uint16)((lowLow >> 8) | (lowLow << 8)) << 16) | (uint16)((lowHigh >> 8) | (lowHigh << 8))) << 32) | (((uint32)(uint16)((highLow >> 8) | (highLow << 8)) << 16) | (uint16)((highHigh >> 8) | (highHigh << 8)));
	}
	inline uint32 SWAP_BYTES_32(uint32 a) {
		const uint16 low = (uint16)a, high = (uint16)(a >> 16);
		return ((uint32)(uint16)((low >> 8) | (low << 8)) << 16) | (uint16)((high >> 8) | (high << 8));
	}
	inline uint16 SWAP_BYTES_16(const uint16 a) {
		return (a >> 8) | (a << 8);
	}
#endif


// We assume the machine to be Little Endian by default
#define LITTLE_ENDIAN true
#define BIG_ENDIAN false