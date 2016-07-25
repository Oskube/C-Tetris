#include <stdio.h>
#include <stdlib.h>

#include "hiscore.h"
#include "file_misc.h" /* CalcCRC32(), DecodeBigendian(), EncodeBigendian() */

#define HEADER_LEN (sizeof(unsigned)*3)
#define HEADER_SIG 0x2666
#define HEADER_VER 1

/**
    FILE:
    0-3 bytes  -> magic signature
    4-7 bytes  -> version
    8-11 bytes -> length of data segment, no use currently
    12-n bytes -> data
    n+1-n+4 bytes  -> CRC32 of the file

    Hi scores are ordered from best to last
*/

static unsigned ShiftScores(hiscore_list_entry* ptrTable, unsigned len, unsigned int pos);

int ReadHiScores(const char* file, hiscore_list_entry* ptrTable, unsigned len) {
    //  Try opening file
    FILE* ptrFile = fopen(file, "r");
    if (!ptrFile) return -2;

    //  Get size of file
    fseek(ptrFile, 0, SEEK_END);
    unsigned fLen = ftell(ptrFile);
    fseek(ptrFile, 0, SEEK_SET); // rewind

    //  Allocate memory for the buffer, read file and close it
    char* buffer = (char*)malloc(sizeof(char)*fLen);
    if (!buffer) {
        fclose(ptrFile);
        return -1;
    }
    if (fread((void*)buffer, sizeof(char), (size_t)fLen, ptrFile) != fLen) {
        return -5;
    }

    fclose(ptrFile);
    //  Check CRC32 of the file
    unsigned crc32 = CalcCRC32(buffer, fLen-4);
    if (crc32 != DecodeBigendian(*(unsigned*)(buffer+fLen-4))) {
        return -4;
    }

    //  Check signature and version
    char* p = buffer;
    char* end = buffer+fLen-4; // data end, 1st byte of crc32
    if (DecodeBigendian(*(unsigned*)(p)) != HEADER_SIG ||
        DecodeBigendian(*(unsigned*)(p+4)) != HEADER_VER
    ) {
        free(buffer);
        return -3;
    }
    // unsigned dLen = DecodeBigendian(*(unsigned*)(p+8));
    p += HEADER_LEN;

    for (unsigned cur=0; p != end; cur++) {
        ptrTable[cur].score = DecodeBigendian(*(unsigned*)(p));
        ptrTable[cur].rows  = DecodeBigendian(*(unsigned*)(p+4));
        ptrTable[cur].lvl   = DecodeBigendian(*(unsigned*)(p+8));
        ptrTable[cur].time  = DecodeBigendian(*(unsigned*)(p+12));
        ptrTable[cur].date  = DecodeBigendian(*(unsigned*)(p+16));
        p += 20;

        //  Copy name
        for (unsigned i=0; i<16; p++, i++) {
            ptrTable[cur].name[i] = *p;
        }
        ptrTable[cur].name[15] = '\0'; // Lets ensure string is terminated with '\0'
    }

    free(buffer);
    return 0;
}

int SaveHiScores(const char* file, hiscore_list_entry* ptrTable, unsigned len) {
    //  Calculate lenght for buffer, header+list+crc32
    size_t bufLen = HEADER_LEN + sizeof(hiscore_list_entry)*len + sizeof(unsigned);
    char* buffer = (char*)calloc(bufLen, bufLen);
    if (!buffer) return -1;

    //  Write file header to buffer
    char* p = buffer;
    *((unsigned int*)p)   = EncodeBigendian(HEADER_SIG);
    *((unsigned int*)p+1) = EncodeBigendian(HEADER_VER);
    *((unsigned int*)p+2) = EncodeBigendian((unsigned)sizeof(hiscore_list_entry)*len);
    p += HEADER_LEN;

    //  Write scores to buffer
    for (unsigned cur = 0; cur < len; cur++) {
        // score, rows, level, time, date
        *((unsigned int*)p)    = EncodeBigendian(ptrTable[cur].score);
        *((unsigned int*)p+1)  = EncodeBigendian(ptrTable[cur].rows);
        *((unsigned int*)p+2)  = EncodeBigendian(ptrTable[cur].lvl);
        *((unsigned int*)p+3)  = EncodeBigendian(ptrTable[cur].time);
        *((unsigned int*)p+4)  = EncodeBigendian(ptrTable[cur].date);
        p += 20;

        // name, write 16 bytes even if the name is shorter
        for (unsigned i=0; i<16; p++, i++)
            *p = ptrTable[cur].name[i];
    }

    //  Calculate CRC32 for the buffer and append it to end
    unsigned crc32 = CalcCRC32(buffer, bufLen-4);
    *((unsigned*)(buffer+bufLen-4)) = EncodeBigendian(crc32);
    // printf("0x%08x\n", crc32);

    //   Open file for writing
    FILE* ptrFile = fopen(file, "w");
    if (!ptrFile) {
        free(buffer);
        return -2;
    }
    int ret = fwrite(buffer, sizeof(char), bufLen, ptrFile);

    fclose(ptrFile);
    free(buffer);
    return ret;
}

int AddScoreToList(hiscore_list_entry* ptrTable, unsigned len, hiscore_list_entry *ptrEntry) {
    if (!ptrEntry) return -2;
    int rank = GetRanking(ptrTable, len, ptrEntry->score);
    if (rank >= 0 && rank < len) {
        //  Shift and copy values
        ShiftScores(ptrTable, len, rank);

        ptrTable[rank].score = ptrEntry->score;
        ptrTable[rank].rows  = ptrEntry->rows;
        ptrTable[rank].lvl   = ptrEntry->lvl;
        ptrTable[rank].time  = ptrEntry->time;
        ptrTable[rank].date  = ptrEntry->date;
        for (unsigned i=0; i<16; i++)
            ptrTable[rank].name[i] = ptrEntry->name[i];

        ptrTable[rank].name[15] = '\0';
    }

    return rank;
}

int GetRanking(hiscore_list_entry* ptrTable, unsigned len, unsigned score) {
    if (!ptrTable) return -1;
    int rank = 0;

    for (; rank < len && score < ptrTable[rank].score; rank++);

    return rank;
}

/*
    Static functions
*/

/**
    \brief Shifts entries from pos by 1 towards end
    \param ptrTable Pointer to array of scores to be saved
    \param len Lenght of given array
    \param pos Position of the first shifted
    \return 0 on success
*/
unsigned ShiftScores(hiscore_list_entry* ptrTable, unsigned len, unsigned int pos) {
    if (!ptrTable) return -1;
    if (pos >= len) return -2;

    unsigned cur = len-2;
    while (cur >= pos && cur < len) {
        ptrTable[cur+1].score = ptrTable[cur].score;
        ptrTable[cur+1].rows = ptrTable[cur].rows;
        ptrTable[cur+1].lvl = ptrTable[cur].lvl;
        ptrTable[cur+1].time = ptrTable[cur].time;
        ptrTable[cur+1].date = ptrTable[cur].date;
        for (unsigned i=0; i<16; i++)
            ptrTable[cur+1].name[i] = ptrTable[cur].name[i];

        ptrTable[cur].name[15] = '\0';
        cur--;
    }
    return 0;
}
