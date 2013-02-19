

#define TRUE 1
#define FALSE 0
#define COLOR 0
#define GRAY  1
#define  DEFAULT_COMP_QUALITY 80  /* default quality level */
#define  Q_FACTOR_MIN 1
#define  Q_FACTOR_MAX 50

#define  OICMETER_STRING_SIZE 80

typedef BYTE FAR *LPBYTE;


typedef struct tagRLE
{
    BYTE Count;
    WORD Value;
}   RLE; 

typedef struct tagOICMETER
{
    HWND    hWnd;               // Window of caller
    HANDLE  hInst;              // Instance of caller
    FARPROC lpfnAbandon;        // Abandon callback for progress dialog
    LPSTR   lpCaption;          // Caption to put on prometer
    LPSTR   lpMessage1;         // Message to put in prometer
}   OICMETER;

typedef OICMETER FAR *LPOICMETER;

int CPIOICFixupFile (int, LPSTR, LPCPIIMAGESPEC);

/*  Squash side prototypes   */


int   FAR PASCAL CompressOIC (int, int, LPSTR, LPSTR, WORD, WORD, WORD, WORD FAR *, LPSTR, LPOICMETER);

void  TripletsToStrips (LPSTR *, LPSTR *, WORD, WORD, WORD);
void  AverageStrip (LPSTR *, LPSTR *, WORD, WORD, WORD);


void  PadRepScanLines (LPSTR, LPSTR, WORD, WORD, WORD);

void  FwdCosineTransform (LPSTR *, LPSTR *, WORD, WORD, WORD);
void  FwdCosineTransform_TEST (LPSTR *, LPSTR *, WORD, WORD, WORD);


void  StripsToBlocks (LPSTR *, LPSTR *, WORD, WORD, WORD);

void  QuantizeDCTs (LPSTR *, LPSTR *, WORD, WORD, WORD);
void  QuantizeBlock (int FAR *, int FAR *, BOOL);

void  QuantizeLumBlock (int FAR *, int FAR *, BOOL);
void  QuantizeChromBlock (int FAR *, int FAR *, BOOL);


/*  GOFAST squash side assembler functions  */
/*
void fdct   (int FAR *, LPSTR, WORD);
*/
void AvgChrom   (LPSTR, LPSTR, WORD);
void StrToBlkL_A (LPSTR, LPSTR, WORD, WORD);
void StrToBlkCr_A (LPSTR, LPSTR, WORD, WORD);


void  RGBToYCRCB (LPSTR, LPSTR, WORD);
void  EncodeRLE (LPSTR, WORD FAR *, WORD, WORD *);
void  TripletsToPlanes (LPSTR, LPSTR, WORD, WORD);
void  TripletsToStrips (LPSTR *, LPSTR *, WORD, WORD, WORD);
void  fdct (int FAR *, unsigned char FAR *,  WORD);


/*  Unsquash side prototypes   */

int   FAR PASCAL DecompressOIC (int, int, LPSTR, LPSTR, WORD, WORD, WORD, WORD FAR *, LPSTR, LPSTR, LPOICMETER);

void  StripsToTriplets (LPSTR *, LPSTR *, WORD, WORD, WORD);
void  DeAverageStrip (LPSTR *, LPSTR *, WORD, WORD, WORD);
void  InverseCosineTransform (LPSTR *, LPSTR *, WORD, WORD, WORD, LPSTR);
void  idct (BYTE FAR *, int FAR *, WORD, LPSTR);

void  BlocksToStrips (LPSTR *, LPSTR *, WORD, WORD, WORD);
void  DeQuantizeDCTs (LPSTR *, LPSTR *, WORD, WORD, WORD);

void  PlanesToTriplets (LPSTR, LPSTR, WORD, WORD);
void  YCRCBToRGB (LPSTR *, LPSTR *, WORD);
void  DecodeRLE2 (WORD FAR *, LPSTR, WORD);
void  DeQuantizeBlock   (int FAR *, int FAR *, BOOL);

int cpisquash ();


/*  GOFAST unsquash side assembler functions  */

void DecodeRLE2_A (LPSTR, LPSTR, WORD); 
void PlanesToTriplets_A (LPSTR, LPSTR, WORD, WORD);
void DeAvgCr_A (LPSTR, LPSTR, WORD);
void BlkToStrL_A (LPSTR, LPSTR, WORD, WORD);
void BlkToStrCr_A (LPSTR, LPSTR, WORD, WORD);







/*  Others  */

int   put_line();
void  get_img_info();
int   ReadCPiHdr();


void   _cdecl exit(int);
long   _cdecl atol(const char *);


int WriteCPiFileSpec ();
int WriteCPiImgSpec ();


int  ScaleFactor (int);
void CalcMatrices (int);


int   CPiFixupFile (int, LPSTR);




/* Data declarations  */


extern int  zigzag [64];

extern unsigned char q0[2][64]; // 2 64


extern BYTE  LumQuantMatrix   [64];
extern BYTE  ChromQuantMatrix [64];


extern BOOL bIsBufEmpty;
extern BOOL bIsLastRead;
extern WORD wInByteCount; 
extern WORD CurrChunkSize;
extern int   BadCodeCount;


extern WORD wImageWidth;
extern WORD wImageHeight;


extern WORD  wInByteCount;
extern WORD  wOutByteCount;



extern LPSTR lpLZWIOBuf;
extern LPSTR lpLZWDecodeBuf;
extern LPSTR lpXFRMBuf;
extern int hLZWFile;
extern int  hTestFile;




extern WORD    wRLECounts;

