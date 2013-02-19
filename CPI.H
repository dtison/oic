#define LZW_CPI  0
#define LZW_GIF  1
#define LZW_TIF  2


void FAR lmemset (LPSTR, WORD,  WORD);  
void FAR lmemcpy (LPSTR, LPSTR, WORD);
void FAR lmemcmp (LPSTR, LPSTR, WORD);
void FAR lmemrcpy (LPSTR, LPSTR, WORD);  // Reverse copy


int  FAR CompressLZW    (WORD, LPSTR, LPSTR, LPSTR, WORD, WORD FAR *, WORD);
int  FAR DecompressLZW (int, LPSTR, LPSTR, int, WORD, WORD);


LONG FAR PASCAL ReadFile (int, LPSTR, LONG);
LONG FAR PASCAL WriteFile (int, LPSTR, LONG);
LONG FAR PASCAL SeekFile (int, LONG, int);


void FAR InvertScanlines (LPSTR, LPSTR, WORD, WORD);
void FAR PadScanlines (LPSTR, LPSTR, WORD, WORD, WORD);
void FAR ClipScanlines (LPSTR, LPSTR, WORD, WORD, WORD);


void FAR cm2bilvl (LPSTR, LPSTR, WORD, WORD, WORD);
void FAR PlanarToLinear (LPSTR, LPSTR, WORD, WORD, WORD, WORD);
void FAR LinesToTriplets (LPSTR, LPSTR, WORD, WORD);

void FAR PASCAL CreateLogPalFromRGBQuad (LPLOGPALETTE, RGBQUAD FAR *, WORD);

void FAR RGBToBGR (LPSTR, LPSTR, WORD, WORD, WORD);

int  FAR PASCAL chdir (LPSTR);  // (Not invented here)
int  FAR WinMkDir (LPSTR);
int  FAR WinRmDir (LPSTR);
int  FAR WinRename (LPSTR);



#ifdef NEEDFILEIO 
LONG FAR PASCAL WinReadFile (int, LPSTR, LONG);
LONG FAR PASCAL WinWriteFile (int, LPSTR, LONG);
#endif


#ifdef NEVER   // Old stuff no longer supported
int FAR PASCAL BytesTo3Plane (LPSTR FAR *, LPSTR FAR *, LPSTR);
BOOL IsOurVGA (void);
BOOL IsAtLeastVGA (void);
void FAR lstrcpy (LPSTR, LPSTR);
void FAR lstrcat (LPSTR, LPSTR);
#endif


int  FAR PASCAL AppendFile (int, LPSTR, DWORD, DWORD);

DWORD FAR PASCAL GetDiskSpace (int);
