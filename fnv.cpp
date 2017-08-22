#include "fnv.h"

uint32_t FNV32a(const void *data, size_t len)
{
#if !defined(__OPTIMIZE__)
	const uint32_t FNV_32_PRIME = 16777619;
#endif
	uint32_t hval = 2166136261;
	const unsigned char *bp = reinterpret_cast<const unsigned char *>(data);
	const unsigned char *be = bp + len;
	while (bp < be) {
		hval ^= (uint32_t)*bp++;
#if !defined(__OPTIMIZE__)
		hval *= FNV_32_PRIME;
#else
		hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);
#endif
	}
	return hval;
}