/*
 * known_plaintext_decrypt.c — part of my Systems & Network Security lab. Author: Vansh Bhardwaj.
 *
 * A cryptanalysis exercise: recover a file that was encrypted with a weak,
 * home-grown XOR stream cipher, using a known-plaintext attack. Because a PDF
 * always begins with the bytes "%PDF-1.x", XORing that known header against the
 * first ciphertext bytes recovers the seed key; the keystream is then advanced
 * by a modular multiply. The point of the lab: never roll your own crypto — a
 * predictable header plus a weak keystream is enough to break it. No secrets or
 * exploit payloads here; it only decrypts a file you already hold.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Vansh Bhardwaj. See the LICENSE file in this repository.
 *
 *   Build:  cc known_plaintext_decrypt.c -o known_plaintext_decrypt
 *   Usage:  ./known_plaintext_decrypt <ciphertext_file>
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

static void print_hex(const unsigned char *b, size_t n) {
    for (size_t i = 0; i < n; i++) { printf("%02x", b[i]); if (i + 1 < n) putchar(' '); }
}
static uint64_t bytes_to_u64_le(const unsigned char *b) {
    uint64_t x = 0;
    for (int i = 0; i < 8; i++) x |= (uint64_t)b[i] << (8 * i);
    return x;
}
static void u64_to_bytes_le(uint64_t x, unsigned char *o) {
    for (int i = 0; i < 8; i++) o[i] = (unsigned char)((x >> (8 * i)) & 0xff);
}

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "Usage: %s <ciphertext_file>\n", argv[0]); return 1; }
    const char *path = argv[1];
    FILE *f = fopen(path, "rb");
    if (!f) { perror("fopen"); return 1; }
    fseek(f, 0, SEEK_END);
    long flen = ftell(f);
    rewind(f);
    unsigned char *data = malloc(flen);
    fread(data, 1, flen, f);
    fclose(f);

    printf("First 8 ciphertext bytes: ");
    print_hex(data, 8);
    printf("\n");

    /* Try each plausible PDF header version "%PDF-1.v" as known plaintext. */
    for (int v = 1; v <= 7; ++v) {
        unsigned char guess[8] = {'%', 'P', 'D', 'F', '-', '1', '.', (unsigned char)('0' + v)};
        unsigned char key_bytes[8];
        for (int i = 0; i < 8; i++) key_bytes[i] = data[i] ^ guess[i];
        uint64_t key = bytes_to_u64_le(key_bytes);
        uint64_t k = key;
        uint64_t mask64 = 0xFFFFFFFFFFFFFFFFULL;
        unsigned char *out = malloc(flen);
        size_t off = 0;
        while (off < (size_t)flen) {
            size_t chunk = ((size_t)flen - off) >= 8 ? 8 : ((size_t)flen - off);
            unsigned char ks[8];
            u64_to_bytes_le(k, ks);
            for (size_t j = 0; j < chunk; j++) out[off + j] = data[off + j] ^ ks[j];
            k = (uint64_t)(((unsigned __int128)k * (unsigned __int128)key) & mask64);
            off += chunk;
        }
        char outname[512];
        snprintf(outname, sizeof(outname), "%s.decrypted_1%d.pdf", path, v);
        FILE *of = fopen(outname, "wb");
        fwrite(out, 1, flen, of);
        fclose(of);
        printf("try 1.%d: key=0x%016" PRIx64 " wrote %s\n", v, key, outname);
        free(out);
    }
    free(data);
    return 0;
}
