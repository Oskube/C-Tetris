//  Miscellaneous file related functions

/**
    \brief Calculates CRC32 checksum for given byte array
    \param input Array of bytes
    \param len Lenght of array
    \return A 32-bit checksum
*/
extern unsigned CalcCRC32(char* input, unsigned len);


extern unsigned EncodeBigendian(unsigned in);
extern unsigned DecodeBigendian(unsigned in);
