
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <malloc.h> 
#include "cpifmt.h"
#include "msproto.h"

#define TRUE 1
#define FALSE 0
#define COLOR 0
#define GRAY  1
#define  DEFAULT_COMP_QUALITY 80  /* default quality level */
#define  Q_FACTOR_MIN 1
#define  Q_FACTOR_MAX 50

typedef unsigned char far * LPBYTE;


int  zigzag [64] = 
{
    0,  1,  5,  6, 14, 15, 27, 28,
    2,  4,  7, 13, 16, 26, 29, 42,
    3,  8, 12, 17, 25, 30, 41, 43,
    9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};



unsigned char q0[2][64] = 
    {
        {
            16,
            12, 12,
            13, 12, 13,
            17, 14, 14, 17,
            25, 18, 16, 18, 25,
            38, 27, 21, 21, 27, 38,
            55, 40, 33, 30, 33, 40, 55,
            74, 57, 44, 38, 38, 44, 57, 74,
            76, 61, 53, 48, 53, 61, 76,
            78, 68, 63, 63, 68, 78,
            83, 78, 76, 78, 83,
            95, 87, 87, 95,
            105, 103, 105,
            117, 117,
            127
        },
        {
            16,
            18, 18,
            24, 20, 24,
            43, 26, 26, 43,
            85, 53, 37, 53, 85,
            127, 90, 70, 70, 90, 127,
            127, 127, 110, 96, 110, 127, 127,
            127, 127, 127, 127, 127, 127, 127, 127,
            127, 127, 127, 127, 127, 127, 127,
            127, 127, 127, 127, 127, 127,
            127, 127, 127, 127, 127,
            127, 127, 127, 127,
            127, 127, 127,
            127, 127,
            127
    }};



BYTE  LumQuantMatrix   [64];
BYTE  ChromQuantMatrix [64];




BOOL bIsBufEmpty;
BOOL bIsLastRead;
WORD wInByteCount; 
WORD CurrChunkSize;
int   BadCodeCount;



WORD  wInByteCount;
WORD  wOutByteCount;

LPSTR lpLZWIOBuf;
LPSTR lpLZWDecodeBuf;
LPSTR lpXFRMBuf;
int hLZWFile;

typedef struct tagRLE
{
    BYTE Count;
    WORD Value;
}   RLE; 
