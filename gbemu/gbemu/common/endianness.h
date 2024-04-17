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
// As this is running on Intel, AMD and Apple Silicon chips
// Will add compatibility for Big Endian machine later

inline uint16 READ_UINT16(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[1] << 8) | b[0];
}

inline uint32 READ_UINT32(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | (b[0]);
}

inline uint64 READ_UINT64(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return ((uint64)b[7] << 56) | ((uint64)b[6] << 48) | ((uint64)b[5] << 40) | ((uint64)b[4] << 32) | ((uint64)b[3] << 24) | ((uint64)b[2] << 16) | ((uint64)b[1] << 8) | ((uint64)b[0]);
}

inline void WRITE_UINT16(void *ptr, uint16 value) {
	uint8 *b = (uint8 *)ptr;
	b[0] = (uint8)(value >> 0);
	b[1] = (uint8)(value >> 8);
}

inline void WRITE_UINT32(void *ptr, uint32 value) {
	uint8 *b = (uint8 *)ptr;
	b[0] = (uint8)(value >>  0);
	b[1] = (uint8)(value >>  8);
	b[2] = (uint8)(value >> 16);
	b[3] = (uint8)(value >> 24);
}

inline void WRITE_UINT64(void *ptr, uint64 value) {
	uint8 *b = (uint8 *)ptr;
	b[0] = (uint8)(value >>  0);
	b[1] = (uint8)(value >>  8);
	b[2] = (uint8)(value >> 16);
	b[3] = (uint8)(value >> 24);
	b[4] = (uint8)(value >> 32);
	b[5] = (uint8)(value >> 40);
	b[6] = (uint8)(value >> 48);
	b[7] = (uint8)(value >> 56);
}

// Used in Common::Stream
#define READ_LE_UINT16(a) READ_UINT16(a)
#define READ_LE_UINT32(a) READ_UINT32(a)
#define READ_LE_UINT64(a) READ_UINT64(a)
#define WRITE_LE_UINT16(a, v) WRITE_UINT16(a, v)
#define WRITE_LE_UINT32(a, v) WRITE_UINT32(a, v)
#define WRITE_LE_UINT64(a, v) WRITE_UINT64(a, v)
#define FROM_LE_16(a) ((uint16)(a))
#define FROM_LE_32(a) ((uint32)(a))
#define FROM_LE_64(a) ((uint64)(a))
#define FROM_BE_16(a) SWAP_BYTES_16(a)
#define FROM_BE_32(a) SWAP_BYTES_32(a)
#define FROM_BE_64(a) SWAP_BYTES_64(a)
#define TO_LE_16(a) ((uint16)(a))
#define TO_LE_32(a) ((uint32)(a))
#define TO_LE_64(a) ((uint64)(a))
#define TO_BE_16(a) SWAP_BYTES_16(a)
#define TO_BE_32(a) SWAP_BYTES_32(a)
#define TO_BE_64(a) SWAP_BYTES_64(a)

// Might come handy later
#define LITTLE_ENDIAN false
#define BIG_ENDIAN true