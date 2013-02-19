
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


int  zigzag [64];  


unsigned char q0[2][64];  


BYTE  LumQuantMatrix   [64];
BYTE  ChromQuantMatrix [64];



int   infh, outfh;
char *scan_buf;
char in_filename[13];
char out_filename[13];
unsigned bytes;

DWORD   dwCPIPalPos, dwCPIDataPos; 
WORD    wRLECounts;


int   CompLevel;        // (Pre-scaled)
int   CompFactor;     // (Scaled)


BOOL bIsBufEmpty;
BOOL bIsLastRead;
WORD wInByteCount; 
WORD CurrChunkSize;
int   BadCodeCount;


WORD wImageWidth;
WORD wImageHeight;


WORD  wInByteCount;
WORD  wOutByteCount;


LPSTR lpHashTable;
LPSTR lpLZWIOBuf;
LPSTR lpLZWDecodeBuf;
LPSTR lpXFRMBuf;
int hLZWFile;
int  hTestFile;

