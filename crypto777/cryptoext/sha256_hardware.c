#include <string.h>
#include "sha256_hardware.h"

typedef void (*func_ptr)(void *input_data, uint32_t digest[8], uint64_t num_blks);

/* Padding (FIPS180-2, Section 5.1.1) */
static uint8_t padding[64] = {
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* Initial hash value (FIPS180-2, Section 5.3.2) */
static const uint32_t iv256[SHA256_WORDS] = {
  0x6a09e667,
  0xbb67ae85,
  0x3c6ef372,
  0xa54ff53a,
  0x510e527f,
  0x9b05688c,
  0x1f83d9ab,
  0x5be0cd19
};

/* Choose an update function */

#if SHA_TYPE == 1
static func_ptr intel_func = sha256_avx;
#elif SHA_TYPE == 2
static func_ptr intel_func = sha256_sse4;
#elif SHA_TYPE == 3
static func_ptr intel_func = sha256_rorx;
#else // SHA_TYPE = 4
static func_ptr intel_func = sha256_rorx_x8ms;
#endif

void
SHA256_hash(const void* data, size_t len, uint8_t digest[SHA256_DIGEST_SIZE])
{
  SHA256_Context ctx;
  
  SHA256_init(&ctx);
  SHA256_update(&ctx, data, len);
  SHA256_final(&ctx, digest);
}

void
SHA256_init (SHA256_Context *sc)
{
  int i;
  for (i = 0; i < 8; i++) 
    sc->hash[i] = iv256[i];
  sc->totalLength = 0;
  sc->bufferLength = 0;
}

void
SHA256_update (SHA256_Context *sc, const void* data, size_t len)
{
	const uint8_t *data_ptr = data;
	uint32_t bufferBytesLeft;
	size_t bytesToCopy;
	int rem;

	if (sc->bufferLength) {
		do {
			bufferBytesLeft = 64L - sc->bufferLength;
			bytesToCopy = bufferBytesLeft;
			if (bytesToCopy > len)
				bytesToCopy = len;

			memcpy (&sc->buffer.bytes[sc->bufferLength], data_ptr, bytesToCopy);
			sc->totalLength += bytesToCopy * 8L;
			sc->bufferLength += bytesToCopy;
			data_ptr += bytesToCopy;
			len -= bytesToCopy;

			if (sc->bufferLength == 64L) {
				sc->blocks = 1;
			  intel_func(sc->buffer.words, sc->hash, sc->blocks);
				sc->bufferLength = 0L;
			} else {
				return;
			}
		} while (len > 0 && len <= 64L);
		if (!len) return;
	}

	sc->blocks = len >> 6;
	rem = len - (sc->blocks << 6);
	len = sc->blocks << 6;
	sc->totalLength += rem * 8L;

	if (len) {
		sc->totalLength += len * 8L;
		intel_func((uint32_t *)data_ptr, sc->hash, sc->blocks);
	}
	if (rem) {
		memcpy (&sc->buffer.bytes[0], data_ptr + len, rem);
		sc->bufferLength = rem;
	}
}

void 
SHA256_final(SHA256_Context *sc, uint8_t hash[SHA256_DIGEST_SIZE])
{
  uint32_t bytesToPad;
  uint64_t lengthPad;
  int i;

  bytesToPad = 120 - sc->bufferLength;
  if (bytesToPad > 64)
    bytesToPad -= 64;

  lengthPad = htobe64(sc->totalLength);
  SHA256_update(sc, padding, bytesToPad);
  SHA256_update(sc, &lengthPad, 8);

  if (hash) {
    for (i =0; i < SHA256_WORDS; i++) {
      hash[0] = (uint8_t) (sc->hash[i] >> 24);
      hash[1] = (uint8_t) (sc->hash[i] >> 16);
      hash[2] = (uint8_t) (sc->hash[i] >> 8);
      hash[3] = (uint8_t) sc->hash[i];
      hash += 4;
    }
  }
}
