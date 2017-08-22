#ifndef FNV_H_INCLUDED
#define FNV_H_INCLUDED

#include <cstdint>
#include <cstddef>

//Simple, fast hash non-cryptographic algorithm
uint32_t FNV32a(const void *data, size_t len);

#endif //FNV_H_INCLUDED