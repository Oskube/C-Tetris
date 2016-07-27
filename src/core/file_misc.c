#include "file_misc.h"

static unsigned polynomial = 0x04C11DB7; // Polynomial used in calculating
static int init = 0; // Has look-up table been initialized
static unsigned crc32[256]; //  Look-up table

static void CalcCRC32Table(); // Calculates look-up table

unsigned CalcCRC32(char* input, unsigned len) {
    if (!init) {
        CalcCRC32Table();
        init = 1;
    }

    char* p = input;
    char* end = input+len;
    unsigned crc = 0;
    while (p != end) {
        crc ^= *p << 24;
        crc = (crc << 8)^(crc32[crc >> 24]);
        p++;
    }

    return crc;
}

void CalcCRC32Table() {
    for (unsigned i=0; i<256; i++) {
        unsigned crc = i << 24;
        for(unsigned char j=0x80;j>0;j>>=1) {
            if (crc & 0x80000000) {
                crc <<= 1;
                crc ^= polynomial;
            } else {
                crc <<= 1;
            }
        }
        crc32[i] = crc;
    }
}

unsigned EncodeBigendian(unsigned in) {
    return DecodeBigendian(in);
}
unsigned DecodeBigendian(unsigned in) {
    unsigned char* ptr = (unsigned char*)&in;
    return ptr[3] | (ptr[2] << 8) | (ptr[1] << 16) | (ptr[0] << 24);
}
