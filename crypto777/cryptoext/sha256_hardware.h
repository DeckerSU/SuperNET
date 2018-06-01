#ifndef _VEX_SHA256_H
#define _VEX_SHA256_H

#include <stdint.h>
#include <stddef.h>
#include <endian.h>

#define SHA256_DIGEST_SIZE 32
#define SHA256_WORDS 8
#define SHA_TYPE 3          // 1 = AVX, 2 = SSE4, 3 = RORX, 4 = RORX8

typedef struct {
	uint64_t totalLength;
  uint64_t blocks;
	uint32_t hash[SHA256_WORDS];
	uint32_t bufferLength;
	union {
		uint32_t words[16];
		uint8_t bytes[64];
	} buffer;
} SHA256_Context;

#ifdef __cplusplus
extern "C" {
#endif

/* Public API */
void SHA256_hash(const void* data, size_t len, uint8_t digest[SHA256_DIGEST_SIZE]);

/* Utility functions */
void SHA256_init(SHA256_Context *sc);
void SHA256_update(SHA256_Context* sc, const void* data, size_t len);
void SHA256_final(SHA256_Context* sc, uint8_t hash[SHA256_DIGEST_SIZE]);

/* Intel optimized update functions */
extern void sha256_sse4(void* input_data, uint32_t digest[8], uint64_t num_blks);
extern void sha256_avx(void* input_data, uint32_t digest[8], uint64_t num_blks);
extern void sha256_rorx(void* input_data, uint32_t digest[8], uint64_t num_blks);
extern void sha256_rorx_x8ms(void* input_data, uint32_t digest[8], uint64_t num_blks);

#ifdef __cplusplus
}
#endif

#endif
