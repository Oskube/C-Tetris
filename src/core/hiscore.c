#include <stdio.h>
#include <stdlib.h>

#include "hiscore.h"
#include "file_misc.h" /* CalcCRC32(), DecodeBigendian(), EncodeBigendian() */

#define HEADER_LEN 3
#define HEADER_SIG 0x2666
#define HEADER_VER 1

/**
    FILE:
    0-3 bytes  -> magic signature
    4-7 bytes  -> version
    8-11 bytes -> length of data segment, no use currently
    12-n bytes -> data
    n+1-n+4 bytes  -> CRC32 of the file

    Hi scores are ordered from best to worst
*/

static unsigned ShiftScores(hiscore_list_entry* ptrTable, unsigned len, unsigned int pos);
static int CompareEntry(hiscore_list_entry* a, hiscore_list_entry *b);

int ReadHiScores(const char* file, hiscore_list_entry* ptrTable, unsigned len) {
    //  Try opening file
    FILE* ptrFile = fopen(file, "r");
    if (!ptrFile) return -2;

    //  Get size of file
    fseek(ptrFile, 0, SEEK_END);
    unsigned fLen = ftell(ptrFile);
    fseek(ptrFile, 0, SEEK_SET); // rewind

    //  Allocate memory for the buffer, read file and close it
    unsigned* buffer = (unsigned*)malloc(sizeof(char)*fLen);
    if (!buffer) {
        fclose(ptrFile);
        return -1;
    }
    if (fread((void*)buffer, sizeof(char), (size_t)fLen, ptrFile) != fLen) {
        return -5;
    }

    fclose(ptrFile);
    //  Check CRC32 of the file
    unsigned crc32 = CalcCRC32((char*)buffer, fLen-4);
    if (crc32 != DecodeBigendian(*(unsigned*)((char*)buffer+fLen-4))) {
        return -4;
    }

    //  Check signature and version
    unsigned* p = buffer;
    unsigned* end = buffer+fLen-4; // data end, 1st byte of crc32
    if (DecodeBigendian(*p) != HEADER_SIG ||
        DecodeBigendian(*(p+1)) != HEADER_VER
    ) {
        free(buffer);
        return -3;
    }
    // unsigned dLen = DecodeBigendian(*(unsigned*)(p+8));
    p += HEADER_LEN;

    for (unsigned cur=0; p != end && cur < len; cur++) {
        ptrTable[cur].score = DecodeBigendian(*p);
        ptrTable[cur].rows  = DecodeBigendian(*(p+1));
        ptrTable[cur].lvl   = DecodeBigendian(*(p+2));
        ptrTable[cur].time  = DecodeBigendian(*(p+3));
        ptrTable[cur].date  = DecodeBigendian(*(p+4));
        p += 5;

        //  Copy name
        char* pp = (char*)p;
        for (unsigned i=0; i<16; pp++, i++) {
            ptrTable[cur].name[i] = *pp;
        }
        p = (unsigned*)pp;
        ptrTable[cur].name[15] = '\0'; // Lets ensure string is terminated with '\0'
    }

    free(buffer);
    return 0;
}

int SaveHiScores(const char* file, hiscore_list_entry* ptrTable, unsigned len) {
    //  Calculate lenght for buffer, header+list+crc32
    size_t bufLen = HEADER_LEN*sizeof(unsigned) + sizeof(hiscore_list_entry)*len + sizeof(unsigned);
    unsigned* buffer = (unsigned*)calloc(bufLen, sizeof(char));
    if (!buffer) return -1;

    //  Write file header to buffer
    unsigned* p = buffer;
    *p     = EncodeBigendian(HEADER_SIG);
    *(p+1) = EncodeBigendian(HEADER_VER);
    *(p+2) = EncodeBigendian((unsigned)sizeof(hiscore_list_entry)*len);
    p += HEADER_LEN;

    //  Write scores to buffer
    for (unsigned cur = 0; cur < len; cur++) {
        // score, rows, level, time, date
        *p      = EncodeBigendian(ptrTable[cur].score);
        *(p+1)  = EncodeBigendian(ptrTable[cur].rows);
        *(p+2)  = EncodeBigendian(ptrTable[cur].lvl);
        *(p+3)  = EncodeBigendian(ptrTable[cur].time);
        *(p+4)  = EncodeBigendian(ptrTable[cur].date);
        p += 5;

        // name, write 16 bytes even if the name is shorter
        unsigned i=0;
        char* pp = (char*)p;
        for (; i<16; pp++, i++) {
            *pp = ptrTable[cur].name[i];
            if (*pp == '\0') break;
        }
        for(; i<16; pp++, i++) *pp = 0;

        p = (unsigned*)pp;
    }

    //  Calculate CRC32 for the buffer and append it to end
    unsigned crc32 = CalcCRC32((char*)buffer, bufLen-4);
    *(unsigned*)((char*)buffer+bufLen-4) = EncodeBigendian(crc32);
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
    unsigned rank = GetRanking(ptrTable, len, ptrEntry);
    if (rank < len) {
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

unsigned GetRanking(hiscore_list_entry* ptrTable, unsigned len, hiscore_list_entry* ptrEntry) {
    if (!ptrTable) return len;
    unsigned rank = 0;

    while (rank < len) {
        if (CompareEntry(&ptrTable[rank], ptrEntry) == -1) break;
        rank++;
    }

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

/**
    \brief Compares 2 given entries
    \param a Pointer to entry
    \param b Pointer to entry
    \param 1 if a > b, 0 if same, -1 if b > a
*/
int CompareEntry(hiscore_list_entry* a, hiscore_list_entry *b) {
    if (!a || !b) return 0;
    if (a == b) return 0;

    int tmp = a->score - b->score;
    //  Compare score
    if (tmp > 0) return 1;  //  a is better
    else if (tmp < 0) return -1; // b is better

    // Same score --> compare time
    tmp = a->time - b->time;
    if (tmp < 0) return 1;
    else if (tmp > 0) return -1;

    // Same time --> compare date
    if (a->date < b->date) return 1;
    else if (a->date > b->date) return -1;

    return 0;
}
