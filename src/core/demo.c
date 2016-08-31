#include <stdlib.h>
#include <stdio.h>

#include "demo.h"
#include "file_misc.h"

#define DEMO_METADATA sizeof(unsigned)*5
#define DEMO_SIG 0xDE0666
#define DEMO_VER 1

static demo_list* CreateListElement(void* val);
static void FreeList(demo_list* list);

demo* DemoCreateInstance(void) {
    demo* ret = (demo*)malloc(sizeof(demo));
    if (!ret) return NULL;

    ret->instrsFirst = NULL;
    ret->piecesFirst = NULL;
    ret->instrsCurrent = NULL;
    ret->piecesCurrent = NULL;

    ret->piecesCount = 0;
    ret->instrsCount = 0;

    return ret;
}

void DemoFree(demo* ptr) {
    if (ptr) {
        FreeList(ptr->instrsFirst);
        FreeList(ptr->piecesFirst);
        free(ptr);
    }
}

int DemoAddInstruction(demo* ptr, unsigned time, unsigned instruction) {
    if (!ptr) return -1;

    demo_instruction* ins = (demo_instruction*)malloc(sizeof(demo_instruction));
    if (!ins) return -2;
    ins->time = time;
    ins->instruction = instruction;

    demo_list* nw = CreateListElement((void*)ins);
    if (!nw) {
        free(ins);
        return -3;
    }

    if (!ptr->instrsCurrent) { // 1st instruction
        ptr->instrsFirst = nw;
        ptr->instrsCurrent = ptr->instrsFirst;
    } else {
        ptr->instrsCurrent->next = nw;
        ptr->instrsCurrent = nw;
    }
    ptr->instrsCount++;
    return 0;
}

int DemoAddPiece(demo* ptr, unsigned shape) {
    if (!ptr) return -1;

    unsigned* ptrShape = (unsigned*)malloc(sizeof(unsigned));
    *ptrShape = shape;
    demo_list* nw = CreateListElement((void*)ptrShape);
    if (!nw) {
        free(ptrShape);
        return -2;
    }

    if (!ptr->piecesCurrent) { // 1st piece
        ptr->piecesFirst = nw;
        ptr->piecesCurrent = ptr->piecesFirst;
    } else {
        ptr->piecesCurrent->next = nw;
        ptr->piecesCurrent = nw;
    }
    ptr->piecesCount++;
    return 0;
}

/*
    FILE:
        0-3             Signature           (unsigned)
        4-7             Version*            (unsigned)
        8-11            Piece count         (unsigned)
        12-15           Instruction count   (unsigned)
        16-x            Pieces              (unsigned)
        (x+1)-z         Instructions        (unsigned)*2
        z+1             CRC32               (unsigned)

        x = sizeof(unsigned)*piecesCount
        z = sizeof(2*unsigned)*instrsCount

        *Version means the version of the demosystem, not the complete game
*/

unsigned DemoSave(demo* ptr, const char* path) {
    unsigned ret = 0;
    if (!ptr) return ret;

    //  Allocate buffer
    size_t bufLen = sizeof(demo_instruction)*(ptr->instrsCount) + sizeof(unsigned)*(ptr->piecesCount) + DEMO_METADATA;
    unsigned* buf = (unsigned*)calloc(bufLen+1, 1);
    if (!buf) return ret;

    unsigned* pos = buf;
    // Write header
    *pos = DEMO_SIG;
    *(pos+1) = DEMO_VER;
    *(pos+2) = ptr->piecesCount; //  Write count of pieces
    *(pos+3) = ptr->instrsCount; //  Write count of instructions
    pos += 4;

    //  Write all pieces to the buffer
    demo_list* list = ptr->piecesFirst;
    while (list != NULL) {
        *pos = *(unsigned*)list->value;
        pos++;
        list = list->next;
    }

    list = ptr->instrsFirst;
    while(list != NULL) {
        demo_instruction* c = (demo_instruction*)list->value;
        *pos     = c->time;
        *(pos+1) = c->instruction;
        pos += 2;
        list = list->next;
    }

    //  Crc32
    unsigned crc32 = CalcCRC32((char*)buf, bufLen-4);
    *pos = crc32;

    //  Write buffer into a file
    FILE* fp = fopen(path, "w");
    if (fp) {
        ret = fwrite((void*)buf, 1, bufLen, fp);
        fclose(fp);
    }
    free(buf);  //  Free buffer
    return ret;
}

demo* DemoRead(const char* path) {
    FILE* fp = fopen(path, "r");
    if (!fp) return NULL;

    //  Get filesize
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    rewind(fp);

    //  Allocate buffer
    unsigned* buffer = (unsigned*)calloc(sizeof(char)*len, 1);
    if (!buffer) {
        fclose(fp);
        return NULL;
    }
    //  Read file to buffer
    fread(buffer, sizeof(char), len, fp);
    fclose(fp);

    unsigned* pos = buffer;
    // Check signature and version
    if(*pos != DEMO_SIG || *(pos+1) != DEMO_VER) {
       free(buffer);
       return NULL;
    }
    //  Check CRC32 checksum
    unsigned crc32 = *(unsigned*)((char*)pos+len-4);
    if (crc32 != CalcCRC32((char*)buffer, len-4)) {
        free(buffer);
        return NULL;
    }

    unsigned pieces = *(pos+2);
    unsigned instrs = *(pos+3);
    pos += 4;

    //  Create demo instance
    demo* ret = DemoCreateInstance();

    //  Extract all pieces
    for (unsigned* end=pos+pieces; pos != end; pos++) {
        DemoAddPiece(ret, *pos);
    }

    //  Extract all instructions
    for (unsigned* end=pos+instrs*2; pos != end; pos+=2) {
        unsigned time = *pos;
        unsigned instruction = *(pos+1);
        DemoAddInstruction(ret, time, instruction);
    }

    // Free buffer and return demo instance
    free(buffer);
    return ret;
}

/**
    STATIC FUNCTIONS
**/
demo_list* CreateListElement(void* val) {
    demo_list* ret = (demo_list*)malloc(sizeof(demo_list));
    if (ret) {
        ret->value = val;
        ret->next = NULL;
    }
    return ret;
}

void FreeList(demo_list* list) {
    while (list != NULL) {
        demo_list* rm = list;
        list = list->next;
        free(rm->value); // Free value
        free(rm);        // Free list element
    }
}
