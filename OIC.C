#include <windows.h>
#include <cpi.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <malloc.h> 
#include <memory.h>
#include <cpifmt.h>
#include <oic.h>
#include <prometer.h>


#ifdef TEST
BOOL bTestMode; 
int  hTestFile;
#endif

#define GOFAST

/*  OIC   

    Returns TRUE if sucessful
            FALSE if an error occurs or if user-abandoned */
       

extern BOOL bAbandon;

int FAR PASCAL CompressOIC (hInFile, hOutFile, lpBuffer1, lpBuffer2, wBytesPerRow, wScanWidth, wScanHeight, lpRLECounts, lpHashTable, lpOICMeter)
int         hInFile;
int         hOutFile;
LPSTR       lpBuffer1;
LPSTR       lpBuffer2;
WORD        wBytesPerRow;
WORD        wScanWidth;
WORD        wScanHeight;
WORD    FAR *lpRLECounts;
LPSTR       lpHashTable;
LPOICMETER  lpOICMeter;      // Structure with stuff for Prometer in it.
{

    WORD      wBlocksPerRow;
    WORD      wBlocksPerAveragedRow;
    WORD      wBlocksPerColumn;
    WORD      wBytesPerStrip;
    WORD      wBlockedScanWidth;
    WORD      wBlockedScanHeight;
    WORD      wBytesPerBlockedStrip;
    WORD      wBytesPerAveragedStrip;

    WORD      wCompressedBytes;
    WORD      wFlags;

    LPSTR     lpSource;
    LPSTR     lpDest;
    LPSTR     lpTmp;

    WORD      i;
    LONG      lNumBytes;   //  (For ReadFile & WriteFile functions)

    BOOL      bIsWindows = (GetVersion () > 0);


    wBlocksPerRow         = ((wScanWidth + 7) / 8);         // First time
    wBlocksPerAveragedRow = (wBlocksPerRow + 1) >> 1;
    wBlocksPerRow         = (wBlocksPerAveragedRow << 1);   // Rounded up 

    wBlocksPerColumn      = ((wScanHeight + 7) / 8);

    wBlockedScanWidth     = (wBlocksPerRow << 3);
    wBlockedScanHeight    = (wBlocksPerColumn << 3);

    wBytesPerStrip          = (wBytesPerRow << 3);   // (Input value)
    wBytesPerBlockedStrip   = wBlocksPerRow * 64 * 3;
    wBytesPerAveragedStrip  = (wBlockedScanWidth << 4);  // (8 * Width * 2 types)

    lpSource  = lpBuffer1;
    lpDest    = lpBuffer2;


    /*  Prometer setup  */

    EnableWindow (lpOICMeter -> hWnd, FALSE);
    ProOpen (lpOICMeter -> hWnd, NULL, MakeProcInstance (lpOICMeter -> lpfnAbandon, lpOICMeter -> hInst), lpOICMeter -> lpCaption);
    ProSetBarRange (wBlocksPerColumn);
    ProSetText (ID_STATUS1, lpOICMeter -> lpMessage1);


    wFlags = 8;        // For decoder (8 bit code size)


//  LZW ONLY

    if (WriteFile (hOutFile, (LPSTR) &wFlags, 1L) != 1L)
        return (FALSE);

    for (i = 0; (i < wBlocksPerColumn  && ! bAbandon); i++)
    {

        /*  Grab a strip of CPI raw data      */

        if (i == (wBlocksPerColumn - 1))
        {
            LPSTR lpTmp1;
            LPSTR lpTmp2;

            WORD  wUnusedLines;

            _fmemset (lpSource, 0, wBytesPerStrip);

            wUnusedLines = (wBlocksPerColumn << 3) - wScanHeight;

            if (wUnusedLines > 0)
            {
                lpTmp2 = lpSource + ((8 - wUnusedLines) * wBytesPerRow); // Point to end of image
                lpTmp1 = lpTmp2 - wBytesPerRow;                     // Point to beginning of last actual scanline

                _fmemset (lpTmp2, 0, (wUnusedLines * wBytesPerRow));
            }

            lNumBytes = (LONG) ((8 - wUnusedLines) * wBytesPerRow);

            if (ReadFile (hInFile, lpSource, lNumBytes) != lNumBytes)
                return (FALSE);
                


        }
        else
        {

            lNumBytes = (LONG) wBytesPerStrip;
            if (ReadFile (hInFile, lpSource, lNumBytes) != lNumBytes)
                return (FALSE);

        }


        /*  Transform and pad into "blocks"  (maybe?)  */

        TripletsToStrips (&lpDest, &lpSource, wBlocksPerRow, wBlockedScanWidth, wScanWidth);
  
        lpTmp     = lpSource;
        lpSource  = lpDest;
        lpDest    = lpTmp;


        /*  Convert R line, G line & B line to Luminance, Cr, Cb values  (not yet scaled -128 to 127)  */

        RGBToYCRCB (lpDest, lpSource, wBlockedScanWidth);

        /*  Average half Cr and CB components, preserving Luminance values  */


        lpTmp     = lpSource;
        lpSource  = lpDest;
        lpDest    = lpTmp;
   
        AverageStrip (&lpDest, &lpSource, wBlockedScanWidth, wBlocksPerRow, wBlocksPerAveragedRow);


        /*  Data now in strips, put into contiguous blocks of 64 bytes each  */
  
        lpTmp     = lpSource;
        lpSource  = lpDest;
        lpDest    = lpTmp;
   
        StripsToBlocks (&lpDest, &lpSource, wBlockedScanWidth, wBlocksPerRow, wBlocksPerAveragedRow);

        /*  Perform discrete cosine transform on bytes producing ints -1023..1023  */


        lpTmp     = lpSource;
        lpSource  = lpDest;
        lpDest    = lpTmp;
   

        #ifdef TEST
        if (bTestMode)
            FwdCosineTransform_TEST (&lpDest, &lpSource, wBlockedScanWidth, wBlocksPerRow, wBlocksPerAveragedRow);
        else
        #endif
            FwdCosineTransform (&lpDest, &lpSource, wBlockedScanWidth, wBlocksPerRow, wBlocksPerAveragedRow);


        /*  Perform the final step.  Quantize DCT's to user-defined "squash rate"  */


        lpTmp     = lpSource;
        lpSource  = lpDest;
        lpDest    = lpTmp;
   
        QuantizeDCTs (&lpDest, &lpSource, wBlockedScanWidth, wBlocksPerRow, wBlocksPerAveragedRow);

/*  RLE STUFF   */

  
        {
 
            lpTmp     = lpSource;
            lpSource  = lpDest;
            lpDest    = lpTmp;

            EncodeRLE     (lpDest, (WORD FAR *) lpSource, (wBytesPerAveragedStrip << 1), &wCompressedBytes);

            *lpRLECounts++ = wCompressedBytes;

 
        }    

        {
            // *  Now just ask Mr. LZW to compress the strip and viola' to the file we go  *

            lpTmp     = lpSource;
            lpSource  = lpDest;
            lpDest    = lpTmp;

          //lzw:


            if (i == 0)
                wFlags = 0;
            else
                if (i == (wBlocksPerColumn - 1))
                    wFlags = 2;
                else
                    wFlags = 1;
   
///         CompressStrip (13, lpDest, lpSource, lpHashTable, (wBytesPerAveragedStrip << 1), &wCompressedBytes, wFlags);
          //CompressStrip (8, lpDest, lpSource, lpHashTable, wBytesPerStrip, &wCompressedBytes, wFlags);


            // For RLE use:

            CompressLZW   (13, lpDest, lpSource, lpHashTable, wCompressedBytes, (WORD FAR *) &wCompressedBytes, wFlags);

            if (WriteFile (hOutFile, lpDest, (LONG) wCompressedBytes) != (LONG) wCompressedBytes)
                return (FALSE);
        }      

        {
            char Message [80];


            if (! bIsWindows)
            {
                sprintf (Message, "Processed %d strips of %d\n",(i + 1),wBlocksPerColumn);
                fputs (Message, stdout);
            }
            else
                ProDeltaPos (1);


        }

    }

    // Future:  Make ProDeltaPos return FALSE if user abandons operation.  (This may be return value of
    //          callback fn or may be set always by DLL


    EnableWindow (lpOICMeter -> hWnd, TRUE);
    ProClose ();

    if (bAbandon)
        return (FALSE);
    else
        return (TRUE);

} 


void TripletsToStrips (plpDest, plpSource, wBlocksPerRow, wBlockedScanWidth, wScanWidth)
LPSTR *plpDest;
LPSTR *plpSource;
WORD  wBlocksPerRow;
WORD  wBlockedScanWidth;
WORD  wScanWidth;
{


    LPSTR lpTmp;
    LPSTR lpSource;
    LPSTR lpDest;


    lpSource = *plpSource;
    lpDest   = *plpDest;

    /*  Blocks are always 8 wide by 8 high.  Strips are padded to an even
        integer multiple of blocks  */


    TripletsToPlanes (lpDest, lpSource, wScanWidth, 8);

    /*  PadScanLines with replicate last values on right  */
    /*  Call with a third as many bytes / row with 3 times # of rows... */

    lpTmp     = lpSource;
    lpSource  = lpDest;
    lpDest    = lpTmp;

    PadRepScanLines (lpDest, lpSource, wScanWidth, wBlockedScanWidth, (8 * 3));


    *plpDest    = lpDest;
    *plpSource  = lpSource;


}



#ifndef GOFAST

//  Optimized  04-24-90   D. Ison

void TripletsToPlanes (lpDest, lpSource, wScanWidth, wNumRows)
LPSTR lpDest;
LPSTR lpSource;
WORD  wScanWidth;
WORD  wNumRows;
{


    int i;
    int j;
    LPSTR  lpRedDest;
    LPSTR  lpGreenDest;
    LPSTR  lpBlueDest;

    LPSTR  lpRedSource;
    LPSTR  lpGreenSource;
    LPSTR  lpBlueSource;
    WORD   wNumPixels;

    wNumPixels = wScanWidth * wNumRows;

    lpRedDest   = lpDest;
    lpGreenDest = lpRedDest + wNumPixels;
    lpBlueDest  = lpGreenDest + wNumPixels;

    lpRedSource   = lpSource;
    lpGreenSource = lpRedSource + 1;
    lpBlueSource  = lpGreenSource + 1;

    for (i = 0; i < wNumRows; i++)
    {

        for (j = 0; j < wScanWidth; j++)
        { 
            *lpRedDest++    = *lpRedSource;
            *lpGreenDest++  = *lpGreenSource;
            *lpBlueDest++   = *lpBlueSource;

            lpRedSource   += 3;
            lpGreenSource += 3;
            lpBlueSource  += 3;
        }
    }

}
  
#endif


/*  PadScanLines with replicate last values on right  */

void PadRepScanLines (lpDest, lpSource, wUnpaddedBytes, wPaddedBytes, wNumLines)
LPSTR 	lpDest;
LPSTR 	lpSource; 
WORD  	wUnpaddedBytes;
WORD  	wPaddedBytes;
WORD  	wNumLines;
{
    LPSTR lpInPtr;
    LPSTR lpOutPtr;
    BYTE  PadVal;

    WORD i;
    WORD wPadAmount;

    lpInPtr   = lpSource;
    lpOutPtr  = lpDest;
    wPadAmount = wPaddedBytes - wUnpaddedBytes;

    for (i = 0; i < wNumLines; i++)
    {
        _fmemcpy (lpOutPtr, lpInPtr, wUnpaddedBytes);
        lpOutPtr += wUnpaddedBytes;
        lpInPtr  += wUnpaddedBytes;

        if (wPadAmount > 0)
        {
            PadVal = *(lpInPtr - 1);                 // Last value on line
            _fmemset (lpOutPtr, PadVal, wPadAmount);
            lpOutPtr += wPadAmount;
        }

    }
}    
 




#ifndef GOFAST

/*  Convert R line, G line & B line to Luminance, Cr, Cb values  (not yet scaled -128 to 127)  */
//

void RGBToYCRCB (lpDest, lpSource, wBlockedScanWidth)
LPSTR lpDest;
LPSTR lpSource;
WORD  wBlockedScanWidth;
{

    #define ilong int  


    ilong   Luminance; 
    ilong   ChromR;
    ilong   ChromB;
    ilong   i; 

    ilong   Red, Green, Blue; 


    LPSTR lpRedPtr;
    LPSTR lpGreenPtr;
    LPSTR lpBluePtr;

    LPSTR lpLumPtr;
    LPSTR lpCrPtr;
    LPSTR lpCbPtr;

    LPSTR lpTmp;
    WORD  wNumPixels;
    WORD  wNumRows;

    ilong   Normh;



    wNumRows    = 8;
    wNumPixels  = wBlockedScanWidth * wNumRows;

    Normh       = 64;

    lpRedPtr    = lpSource;
    lpGreenPtr  = lpRedPtr    + wNumPixels;
    lpBluePtr   = lpGreenPtr  + wNumPixels;

    lpLumPtr    = lpDest;
    lpCrPtr     = lpLumPtr + wNumPixels;
    lpCbPtr     = lpCrPtr  + wNumPixels;

    for (i = 0; i < wNumPixels; i++)
    {

        Red    = (BYTE) *lpRedPtr++; 
        Green  = (BYTE) *lpGreenPtr++;
        Blue   = (BYTE) *lpBluePtr++;


        Red >>= 2;
        Green >>= 2;
        Blue >>= 2;



            //  Get Luminance (gray) level


        Luminance  = (Red * 153); 
        Luminance += (Green * 300);
        Luminance += (Blue * 58);  


        if (Luminance < 0)
            Luminance -= Normh;
        else
            Luminance += Normh;


        Luminance >>= 7;


            if (Luminance < 0)
                Luminance = 0;
            else
                if (Luminance > 255)
                    Luminance = 255;




        //  Get ChromR value


        ChromR  = (Red * 256); 
        ChromR += (Green * -214);
        ChromR += (Blue * -41);


        if (ChromR < 0)
            ChromR -= Normh;
        else
            ChromR += Normh;


        ChromR >>= 7;
        ChromR += 128;   // Artificially normalize Chrom components


        //  Get ChromB value

        ChromB  = (Red * -87); 
        ChromB += (Green * -169);
        ChromB += (Blue * 256);

        if (ChromB < 0)
            ChromB -= Normh;
        else
            ChromB += Normh;


        ChromB >>= 7;
        ChromB += 128;


        if (ChromB < 0)
            ChromB = 0;

        if (ChromB > 255)
            ChromB = 255;




        *lpLumPtr   = Luminance;
        *lpCrPtr    = ChromR;
        *lpCbPtr    = ChromB;



        lpLumPtr++;
        lpCrPtr++;
        lpCbPtr++;



    }



}

#endif

 
void AverageStrip (plpDest, plpSource, wBlockedScanWidth, wBlocksPerRow, wBlocksPerAveragedRow)
LPSTR *plpDest;
LPSTR *plpSource;
WORD  wBlockedScanWidth;
WORD  wBlocksPerRow;
WORD  wBlocksPerAveragedRow;
{

    LPSTR lpSource;
    LPSTR lpDest;
    WORD  wNumRows;
    WORD  wNumPixels;

    LPSTR lpSrcLumPtr;
    LPSTR lpDestLumPtr;

    LPSTR lpSrcCrPtr1;
    LPSTR lpSrcCbPtr1;

    LPSTR lpSrcCrPtr2;
    LPSTR lpSrcCbPtr2;

    LPSTR lpDestCrPtr;
    LPSTR lpDestCbPtr;


    WORD  wBytesPerAveragedRow;

    #ifndef GOFAST
    WORD   Val1, Val2;
    BYTE  Val3;
    int   i;
    int   j;
    #endif


    lpSource = *plpSource;
    lpDest   = *plpDest;


    wNumRows   = 8;
    wNumPixels = wBlockedScanWidth * wNumRows;
    wBytesPerAveragedRow    = 8 * wBlocksPerAveragedRow;

    lpSrcLumPtr    = lpSource;
    lpDestLumPtr   = lpDest;


    lpDestCrPtr = lpDestLumPtr + wNumPixels;
    lpDestCbPtr = lpDestCrPtr  + (wNumPixels >> 1);  // 4 rows down


    /*  Point to "current" block  */

    lpSrcCrPtr1    = lpSrcLumPtr + wNumPixels;
    lpSrcCbPtr1    = lpSrcCrPtr1 + wNumPixels;

 
    /*  Point to adjacent block  */

    lpSrcCrPtr2    = lpSrcCrPtr1 + 1;
    lpSrcCbPtr2    = lpSrcCbPtr1 + 1;


    /*  First just transfer Luminance plane verbatim  */

    _fmemcpy (lpDestLumPtr, lpSrcLumPtr, wNumPixels);



    /*  Next write four lines of each line with every 2nd val avged at 8 intervals */ 

    #ifdef GOFAST
    AvgChrom   (lpDestCrPtr, lpSrcCrPtr1, wNumPixels);
    #else
    for (i = 0; i < (wNumPixels >> 1); i++)
    {

        Val1 = (BYTE) *lpSrcCrPtr1;
        Val2 = (BYTE) *lpSrcCrPtr2;

        Val3 = (BYTE) ((Val1 + Val2) >> 1);

        *lpDestCrPtr++ = Val3;

        lpSrcCrPtr1 += 2; 
        lpSrcCrPtr2 += 2; 


        Val1 = (BYTE) *lpSrcCbPtr1;
        Val2 = (BYTE) *lpSrcCbPtr2;

        Val3 = (BYTE) ((Val1 + Val2) >> 1);

        lpSrcCbPtr1 += 2; 
        lpSrcCbPtr2 += 2; 

        *lpDestCbPtr++ = Val3;

    }  
    #endif   


    *plpDest    = lpDest;
    *plpSource  = lpSource;



}



void FwdCosineTransform (plpDest, plpSource, wBlockedScanWidth, wBlocksPerRow, wBlocksPerAveragedRow)
LPSTR *plpDest;
LPSTR *plpSource;
WORD  wBlockedScanWidth;
WORD  wBlocksPerRow;
WORD  wBlocksPerAveragedRow;
{

    LPSTR lpSource;
    LPSTR lpDest;

    WORD  wBytesPerLumRow;
    WORD  wBytesPerChromRow;
    WORD  wNumRows;
    WORD  wNumPixels;

    WORD  i;

    LPSTR lpSrcLumPtr;
    LPSTR lpSrcCrPtr;
    LPSTR lpSrcCbPtr;

    int   FAR *lpDestLumPtr;
    int   FAR *lpDestCrPtr;
    int   FAR *lpDestCbPtr;


    lpSource = *plpSource;
    lpDest   = *plpDest;

    wNumRows   = 8;
    wNumPixels = wBlockedScanWidth * wNumRows;

    wBytesPerLumRow         = (wBlocksPerRow * 8);
    wBytesPerChromRow       = (wBlocksPerAveragedRow * 8);

    lpSrcLumPtr = lpSource;
    lpSrcCrPtr  = lpSrcLumPtr + wNumPixels;
    lpSrcCbPtr  = lpSrcCrPtr  + (wNumPixels >> 1);   // Only 4 rows down

    lpDestLumPtr = (int FAR *) lpDest;
    lpDestCrPtr  = lpDestLumPtr + wNumPixels;
    lpDestCbPtr  = lpDestCrPtr  + (wNumPixels >> 1);   // Only 4 rows down



    /*  Transform Luminance Blocks  */


    for (i = 0; i < wBlocksPerRow; i++)
    {

        fdct   (lpDestLumPtr, lpSrcLumPtr, 8);
  
        lpSrcLumPtr  += 64;
        lpDestLumPtr += 64;

    }



  
    /*  Transform ChromR  Blocks  */

    for (i = 0; i < wBlocksPerAveragedRow; i++)
    {
        fdct   (lpDestCrPtr, lpSrcCrPtr, 8);

        lpSrcCrPtr   += 64;
        lpDestCrPtr  += 64;

    }

  
    /*  Transform ChromB  Blocks  */

    for (i = 0; i < wBlocksPerAveragedRow; i++)
    {
        fdct   (lpDestCbPtr, lpSrcCbPtr, 8);

        lpSrcCbPtr   += 64;
        lpDestCbPtr  += 64;

    }



    *plpDest    = lpDest;
    *plpSource  = lpSource;


}





#ifdef TEST 

/*  Transform with fdct with debugging tests  */


void  FwdCosineTransform_TEST (plpDest, plpSource, wBlockedScanWidth, wBlocksPerRow, wBlocksPerAveragedRow)
LPSTR *plpDest;
LPSTR *plpSource;
WORD  wBlockedScanWidth;
WORD  wBlocksPerRow;
WORD  wBlocksPerAveragedRow;
{

    LPSTR lpSource;
    LPSTR lpDest;

    WORD  wBytesPerLumRow;
    WORD  wBytesPerChromRow;
    WORD  wNumRows;
    WORD  wNumPixels;

    int   i;

    LPSTR lpSrcLumPtr;
    LPSTR lpSrcCrPtr;
    LPSTR lpSrcCbPtr;

    int   FAR *lpDestLumPtr;
    int   FAR *lpDestCrPtr;
    int   FAR *lpDestCbPtr;


    BYTE  BlockBuffer [64];
    int   DCTBuffer   [64];


    lpSource = *plpSource;
    lpDest   = *plpDest;

    wNumRows   = 8;
    wNumPixels = wBlockedScanWidth * wNumRows;

    wBytesPerLumRow         = (wBlocksPerRow * 8);
    wBytesPerChromRow       = (wBlocksPerAveragedRow * 8);

    lpSrcLumPtr = lpSource;
    lpSrcCrPtr  = lpSrcLumPtr + wNumPixels;
    lpSrcCbPtr  = lpSrcCrPtr  + (wNumPixels >> 1);   // Only 4 rows down

    lpDestLumPtr = (int FAR *) lpDest;
    lpDestCrPtr  = lpDestLumPtr + wNumPixels;
    lpDestCbPtr  = lpDestCrPtr  + (wNumPixels >> 1);   // Only 4 rows down



    /*  Transform Luminance Blocks  */


    for (i = 0; i < wBlocksPerRow; i++)
    {
        fdct (lpDestLumPtr, lpSrcLumPtr, 8);
  
        lpSrcLumPtr  += 64;
        lpDestLumPtr += 64;

    }



  
    /*  Transform ChromR  Blocks  */

    for (i = 0; i < wBlocksPerAveragedRow; i++)
    {
        fdct (lpDestCrPtr, lpSrcCrPtr, 8);

        lpSrcCrPtr   += 64;
        lpDestCrPtr  += 64;

    }

  
    /*  Transform ChromB  Blocks  */

    for (i = 0; i < wBlocksPerAveragedRow; i++)
    {
        fdct (lpDestCbPtr, lpSrcCbPtr, 8);

        lpSrcCbPtr   += 64;
        lpDestCbPtr  += 64;

    }



    *plpDest    = lpDest;
    *plpSource  = lpSource;


}


#endif



#ifdef TEST  


void fdct (xfrm, xin, wValuesPerRow) 
int FAR   *xfrm;
unsigned char FAR *xin;
WORD      wValuesPerRow;
{
    int   i, j, p;


    #ifdef PRINT
        #define   ilong long
    #else
        #define   ilong int
    #endif


    ilong      a;

//  long      k;
//  long      k4 [8];

    int       k;
    int       k4 [8];


    int       k00, k01, k02, k03, k04, k05, k06, k07, k10, k11, k12, 
              k13, k14, k15, k16, k17, k20, k21, k22, k23, k24, k25, 
              k26, k27, k34, k35, k36, k37;

//  Divided all values by 16

    ilong      c1p4  = 23;
    ilong      c1p8  = 30; 
    ilong      s1p8  = 12;
    ilong      c1p16 = 31;
    ilong      s1p16 =  6;
    ilong      c3p16 = 27;
    ilong      s3p16 = 18;
    ilong      norm  = 32;


    /* 1-dimensional transform on rows */

    p = 0;
    for (i = 0; i < 8; ++i) 
    {

        k00 = xin [p];
        k01 = xin [p + 1];
        k02 = xin [p + 2];
        k03 = xin [p + 3];
        k04 = xin [p + 4];
        k05 = xin [p + 5];
        k06 = xin [p + 6];
        k07 = xin [p + 7];

        k10 = (k00 + k07) << 2;
        k11 = (k01 + k06) << 2;
        k12 = (k02 + k05) << 2;
        k13 = (k03 + k04) << 2;
        k14 = (k03 - k04) << 2;
        k15 = (k02 - k05) << 2;
        k16 = (k01 - k06) << 2;
        k17 = (k00 - k07) << 2;

        k20 = k10 + k13;
        k21 = k11 + k12;
        k22 = k11 - k12;
        k23 = k10 - k13;
        k24 = k14;

        /*  k25  k26 ok  now  */


        a   = (ilong) c1p4 * (ilong) ((k16 - k15) >> 1);

        #ifdef PRINT
        if (a > 32767)
            _printf ("k25 (1) gets greater: %ld\n", a);

        if (a < -32767)
            _printf ("k25 (1) gets less than: %ld \n",a);
        #endif

        k25 = (int) (a >> 4);



        a   = (ilong) c1p4 * (ilong) ((k16 + k15) >> 1);

        #ifdef PRINT
        if (a > 32767)
            _printf ("k26 (1) gets greater: %ld\n", a);

        if (a < -32767)
            _printf ("k26 (1) gets less than: %ld \n",a);     
        #endif

        k26 = (int) (a >> 4);


        k27 = k17;



        /*  Fix for "xfrm [p]"      */

        a = (ilong) ((k20 + k21 - 4096) >> 3);
        a *= c1p4;

        #ifdef PRINT
        if (a > 32767)
            _printf ("xfrm [p] (1) gets greater: %ld\n", a);

        if (a < -32767)
            _printf ("xfrm [p] (1) gets less than: %ld \n",a);   
        #endif

        xfrm [p] = (int) (a >> 2);


        a = (ilong) ((k20 - k21) >> 2);
        a *= c1p4;


        #ifdef PRINT
        if (a > 32767)
            _printf ("xfrm [p + 4] (1) gets greater: %ld\n", a);

        if (a < -32767)
            _printf ("xfrm [p + 4] (1) gets less than: %ld \n",a); 

        #endif
        xfrm [p + 4] = (int) (a >> 3);




        /*  Fix for "xfrm [p + 2]"    */

        a  = (ilong) ((ilong) c1p8 * (ilong) (k23 >> 2));
        a += (ilong) ((ilong) s1p8 * (ilong) (k22 >> 2));

        #ifdef PRINT
        if (a > 32767)
            _printf ("xfrm [p + 2] (1) gets greater: %ld\n", a);

        if (a < -32767)
            _printf ("xfrm [p + 2] (1) gets less than: %ld \n",a);    
        #endif

        xfrm [p + 2] = (int) (a >> 3);  // (5 - shift amt above)



        /*  Fix for "xfrm [p + 6]"    */

        a  = (ilong) ((ilong) s1p8 * (ilong) (k23 >> 2));
        a -= (ilong) ((ilong) c1p8 * (ilong) (k22 >> 2));

        #ifdef PRINT
        if (a > 32767)
            _printf ("xfrm [p + 6] (1) gets greater: %ld\n", a);

        if (a < -32767)
            _printf ("xfrm [p + 6] (1) gets less than: %ld \n",a);    

        #endif
        xfrm [p + 6] = (int) (a >> 3);





        /*  k34 .. k37 ok  */

        k34 = k24 + k25;
        k35 = k24 - k25;
        k36 = k27 - k26;
        k37 = k27 + k26;





        /*  Fix for "xfrm [p + 1]"    */

        a  = (ilong) ((ilong) c1p16 * (ilong) (k37 >> 2));
        a += (ilong) ((ilong) s1p16 * (ilong) (k34 >> 2));

        #ifdef PRINT
        if (a > 32767)
            _printf ("xfrm [p + 1] (1) gets greater: %ld\n", a);

        if (a < -32767)
            _printf ("xfrm [p + 1] (1) gets less than: %ld \n",a); 
        #endif

        xfrm [p + 1] = (int) (a >> 3);
  

        /*  Fix for "xfrm [p + 5]"    */

        a  = (ilong) ((ilong) s3p16 * (ilong) (k36 >> 2));
        a += (ilong) ((ilong) c3p16 * (ilong) (k35 >> 2));

        #ifdef PRINT
        if (a > 32767)
            _printf ("xfrm [p + 5] (1) gets greater: %ld\n", a);

        if (a < -32767)
            _printf ("xfrm [p + 5] (1) gets less than: %ld \n",a);     
        #endif


        xfrm [p + 5] = (int) (a >> 3);



        /*  Fix for "xfrm [p + 3]"    */

        a  = (ilong) ((ilong) c3p16 * (ilong) (k36 >> 2));
        a -= (ilong) ((ilong) s3p16 * (ilong) (k35 >> 2));

        #ifdef PRINT
        if (a > 32767)
            _printf ("xfrm [p + 3] (1) gets greater: %ld\n", a);

        if (a < -32767)
            _printf ("xfrm [p + 3] (1) gets less than: %ld \n",a);     
        #endif


        xfrm [p + 3] = (int) (a >> 3);


        /*  Fix for "xfrm [p + 7]"    */

        a  = (ilong) ((ilong) s1p16 * (ilong) (k37 >> 2));
        a -= (ilong) ((ilong) c1p16 * (ilong) (k34 >> 2));

        #ifdef PRINT
        if (a > 32767)
            _printf ("xfrm [p + 7] (1) gets greater: %ld\n", a);

        if (a < -32767)
            _printf ("xfrm [p + 7] (1) gets less than: %ld \n",a);
        #endif                  


        xfrm [p + 7] = (int) (a >> 3);



        p += wValuesPerRow;
    }

    /* 1-dimensional transform on columns */

    for (j = 0; j < 8; ++j) 
    {

        k00 = xfrm [j];
        k01 = xfrm [j + (wValuesPerRow)];
        k02 = xfrm [j + (wValuesPerRow * 2)];
        k03 = xfrm [j + (wValuesPerRow * 3)];
        k04 = xfrm [j + (wValuesPerRow * 4)];
        k05 = xfrm [j + (wValuesPerRow * 5)];
        k06 = xfrm [j + (wValuesPerRow * 6)];
        k07 = xfrm [j + (wValuesPerRow * 7)];


        k10 = (k00 + k07) / 2;
        k11 = (k01 + k06) / 2;
        k12 = (k02 + k05) / 2;
        k13 = (k03 + k04) / 2;
        k14 = (k03 - k04) / 2;
        k15 = (k02 - k05) / 2;
        k16 = (k01 - k06) / 2;
        k17 = (k00 - k07) / 2;

        k20 = k10 + k13;
        k21 = k11 + k12;
        k22 = k11 - k12;
        k23 = k10 - k13;
        k24 = k14;



        a   = (ilong) c1p4 * (ilong) ((k16 - k15) >> 2);

        #ifdef PRINT
        if (a > 32767)
            _printf ("k25 (2) gets greater: %ld\n", a);

        if (a < -32767)
            _printf ("k25 (2) gets less than: %ld \n",a);
        #endif

        k25 = (int) (a >> 3);


        a   = (ilong) c1p4 * (ilong) ((k16 + k15) >> 2);

        #ifdef PRINT
        if (a > 32767)
            _printf ("k26 (2) gets greater: %ld\n", a);

        if (a < -32767)
            _printf ("k26 (2) gets less than: %ld \n",a);     
        #endif

        k26 = (int) (a >> 3);


        k27 = k17;



        /*  Fix for "k4 [0]"      */

        a = (ilong) ((k20 + k21) >> 3);
        a *= c1p4;


        #ifdef PRINT
        if (a > 32767)
            _printf ("k4 [0] gets greater: %ld\n", a);

        if (a < -32767) 
            _printf ("k4 [0] gets less than: %ld \n",a);    
        #endif


        k4 [0] = (int) (a >> 2);




        /*  Fix for "k4 [4]"      */

        a = (ilong) ((k20 - k21) >> 2);
        a *= c1p4;

        #ifdef PRINT
        if (a > 32767)
            _printf ("k4 [4] gets greater: %ld\n", a);

        if (a < -32767)
            _printf ("k4 [4] gets less than: %ld \n",a); 
        #endif


        k4 [4] = (int) (a >> 3);




        /*  Fix for "k4 [2]"      */

        a = (ilong) ((c1p8 * (k23 >> 2)) + (s1p8 * (k22 >> 2)));

        #ifdef PRINT
        if (a > 32767)
            _printf ("k4 [2] gets greater: %ld\n", a);

        if (a < -32767)
            _printf ("k4 [2] gets less than: %ld \n",a);     
        #endif


        k4 [2] = (int) (a >> 3);


        /*  Fix for "k4 [6]"      */

        a = (ilong) ((s1p8 * (k23 >> 2)) - (c1p8 * (k22 >> 2)));


        #ifdef PRINT
        if (a > 32767)
            _printf ("k4 [6] gets greater: %ld\n", a);

        if (a < -32767)
            _printf ("k4 [6] gets less than: %ld \n",a);     
        #endif

        k4 [6] = (int) (a >> 3);





        k34 = k24 + k25;
        k35 = k24 - k25;
        k36 = k27 - k26;
        k37 = k27 + k26;



        /*  Fix for "k4 [1]"      */

        a = (ilong) ((c1p16 * (k37 >> 3)) + (s1p16 * (k34 >> 3)));


        #ifdef PRINT
        if (a > 32767)
            _printf ("k4 [1] gets greater: %ld\n", a);

        if (a < -32767)
           _printf ("k4 [1] gets less than: %ld \n",a);     
        #endif

        k4 [1] = (int) (a >> 2); 



        /*  Fix for "k4 [5]"      */

        a = (ilong) ((s3p16 * (k36 >> 2)) + (c3p16 * (k35 >> 2)));

        #ifdef PRINT
        if (a > 32767)
            _printf ("k4 [5] gets greater: %ld\n", a);

        if (a < -32767)
            _printf ("k4 [5] gets less than: %ld \n",a);     
        #endif

        k4 [5] = (int) (a >> 3);




        /*  Fix for "k4 [3]"      */

        a = (ilong) ((c3p16 * (k36 >> 2)) - (s3p16 * (k35 >> 2)));


        #ifdef PRINT
        if (a > 32767)
            _printf ("k4 [3] gets greater: %ld\n", a);

        if (a < -32767)
            _printf ("k4 [3] gets less than: %ld \n",a);     

        #endif
        k4 [3] = (int) (a >> 3);



        /*  Fix for "k4 [7]"      */

        a = (ilong) ((s1p16 * (k37 >> 2)) - (c1p16 * (k34 >> 2)));

        #ifdef PRINT
        if (a > 32767)
            _printf ("k4 [7] gets greater: %ld\n", a);

        if (a < -32767)
            _printf ("k4 [7] gets less than: %ld \n",a);      
        #endif


//      k4 [7] = (int) (a >> 2);    Mistake ?

        k4 [7] = (int) (a >> 3);




        p = j;
        for (i = 0; i < 8; ++i) 
        {
            if (k4[i] < 0) 
            {
                k = (k4[i] - 4) / 8;
                if (k < -1023)
                    k = -1023;
            } 
            else 
            {
                k = (k4[i] + 4) / 8;
                if (k > 1023)
                    k = 1023;
            }
            xfrm[p] = (int) k;
            p += wValuesPerRow;
        }
    }


}

#endif



void StripsToBlocks (plpDest, plpSource, wValuesPerRow, wBlocksPerRow, wBlocksPerAveragedRow)
LPSTR *plpDest;
LPSTR *plpSource;
WORD  wValuesPerRow;
WORD  wBlocksPerRow;
WORD  wBlocksPerAveragedRow;
{

    WORD  wNumPixels;
    WORD  wNumRows;

    LPSTR lpSource;
    LPSTR lpDest;

    #ifndef GOFAST

    LPSTR lpSourcePtr;
    LPSTR lpDestPtr;
    int i;
    int j;
    int k;

    #endif

    wNumRows    = 8;
    wNumPixels  = wValuesPerRow * wNumRows;

    lpSource  = *plpSource;
    lpDest    = *plpDest;


    /*  Do Luminance strip  */


    #ifdef GOFAST
    StrToBlkL_A (lpDest, lpSource, wBlocksPerRow, wValuesPerRow);
    #else
    for (i = 0; i < wBlocksPerRow; i++)
    {

        lpSourcePtr = lpSource;
        lpDestPtr   = lpDest;

        for (j = 0; j < 8; j++)
        {   
            for (k = 0; k < 8; k++)
                *lpDestPtr++ = lpSourcePtr [k];

            lpSourcePtr += wValuesPerRow; 

        }

        lpSource  += 8;
        lpDest    += 64;
    } 
    #endif

    lpSource  = *plpSource;
    lpDest    = *plpDest;

    lpSource  += wNumPixels;
    lpDest    += wNumPixels;




    /*  Do ChromR strip  */

    #ifdef GOFAST
    StrToBlkL_A (lpDest, lpSource, wBlocksPerAveragedRow, (wValuesPerRow >> 1));
    #else
    for (i = 0; i < wBlocksPerAveragedRow; i++)
    {

        lpSourcePtr = lpSource;
        lpDestPtr   = lpDest;

        for (j = 0; j < 8; j++)
        {   
            for (k = 0; k < 8; k++)
                *lpDestPtr++ = lpSourcePtr [k];

            lpSourcePtr += (wValuesPerRow >> 1); 

        }

        lpSource  += 8;
        lpDest    += 64;
    }    
    #endif

    lpSource  = *plpSource;
    lpDest    = *plpDest;

    lpSource  += (wNumPixels + (wNumPixels >> 1));
    lpDest    += (wNumPixels + (wNumPixels >> 1));


    /*  Do ChromB strip  */

    #ifdef GOFAST
    StrToBlkL_A (lpDest, lpSource, wBlocksPerAveragedRow, (wValuesPerRow >> 1));
    #else
    for (i = 0; i < wBlocksPerAveragedRow; i++)
    {

        lpSourcePtr = lpSource;
        lpDestPtr   = lpDest;

        for (j = 0; j < 8; j++)
        {   
            for (k = 0; k < 8; k++)
                *lpDestPtr++ = lpSourcePtr [k];

            lpSourcePtr += (wValuesPerRow >> 1); 

        }

        lpSource  += 8;
        lpDest    += 64;

    }   
    #endif


}




  
void QuantizeDCTs (plpDest, plpSource, wBlockedScanWidth, wBlocksPerRow, wBlocksPerAveragedRow)
LPSTR *plpDest;
LPSTR *plpSource;
WORD  wBlockedScanWidth;
WORD  wBlocksPerRow;
WORD  wBlocksPerAveragedRow;
{

    LPSTR lpSource;
    LPSTR lpDest;

    WORD  wNumRows;
    WORD  wNumPixels;

    WORD  i;
 

    int   FAR *lpSrcLumPtr;
    int   FAR *lpSrcCrPtr;

    int   FAR *lpDestLumPtr;
    int   FAR *lpDestCrPtr;



    lpSource = *plpSource;
    lpDest   = *plpDest;

    wNumRows   = 8;
    wNumPixels = wBlockedScanWidth * wNumRows;

    _fmemset (lpDest, 0, (wNumPixels << 2));


    lpSrcLumPtr = (int FAR *) lpSource;
    lpSrcCrPtr  = lpSrcLumPtr + wNumPixels;

    lpDestLumPtr = (int FAR *) lpDest;
    lpDestCrPtr  = lpDestLumPtr + wNumPixels;




    /*  Quantize Luminance Strip  */


    for (i = 0; i < wBlocksPerRow; i++)
    {
        QuantizeBlock (lpDestLumPtr, lpSrcLumPtr, TRUE);
  
        lpSrcLumPtr  += 64;
        lpDestLumPtr += 64;

    }


    /*  Quantize Chrominance Strip  */

    for (i = 0; i < wBlocksPerRow; i++)
    {
        QuantizeBlock (lpDestCrPtr, lpSrcCrPtr, FALSE);

        lpSrcCrPtr  += 64;
        lpDestCrPtr += 64;


    }



    *plpDest    = lpDest;
    *plpSource  = lpSource;

}




void QuantizeBlock (lpDest, lpSource, bIsLuminance)
int FAR *lpDest;
int FAR *lpSource;
BOOL    bIsLuminance;
{
    int  i, k;
    int  ix, iq;
    int  tmp;


    for (i = 0; i < 64; i++) 
    {
        ix  = lpSource [i];
        k   = zigzag   [i];

        if (bIsLuminance)
            iq  = LumQuantMatrix [k];
        else
            iq  = ChromQuantMatrix [k];

        if (ix >= 0)
            lpDest [k] = (ix + iq / 2) / iq;   
        else
            lpDest [k] = (ix - iq / 2) / iq;



        tmp = lpDest [k];



    }
}



void QuantizeLumBlock (lpDest, lpSource, bIsLuminance)
int FAR *lpDest;
int FAR *lpSource;
BOOL    bIsLuminance;
{
    int  i, k;
    int  ix, iq;
    int  tmp;


/*  Do 64 inline quantizations  */

    ix = *lpSource++;
    iq = 16;

    if (ix >= 0)
        lpDest [0] = (ix + iq / 2) / iq;   
    else
        lpDest [0] = (ix - iq / 2) / iq;



    for (i = 0; i < 64; i++) 
    {
        ix  = lpSource [i];
        k   = zigzag   [i];

        if (bIsLuminance)
            iq  = LumQuantMatrix [k];
        else
            iq  = ChromQuantMatrix [k];

        if (ix >= 0)
            lpDest [k] = (ix + iq / 2) / iq;   
        else
            lpDest [k] = (ix - iq / 2) / iq;



        tmp = lpDest [k];



    }
}




#ifndef GOFAST

/*  Word oriented run length encoder  */ 

void EncodeRLE (lpDest, lpSource, wNumBytes, wCompressedBytes)
LPSTR lpDest;
WORD  FAR *lpSource;
WORD  wNumBytes;
WORD  *wCompressedBytes;
{


    WORD  CurrVal;
    WORD  NextVal;

    int   i;
    RLE   RLEPacket; 
    char  *PacketPtr;


    WORD  CurrRunCount;
    BOOL  bNeedNewPacket;
    WORD  wRLEByteCount;
    WORD  wNumVals;

    BOOL  bIsPending;
    BOOL  bIsGreater;
    WORD  wVals;

    wNumVals = wNumBytes >> 1;


    CurrVal         = *lpSource++;
    CurrRunCount    = 1;
    bNeedNewPacket  = FALSE; 
    wRLEByteCount   = 0;
    wVals           = 1; 


    do
    {


        NextVal = *lpSource++;
        wVals++;

        bIsPending = FALSE;
        bIsGreater = FALSE;

        if (NextVal == CurrVal)
        {
            CurrRunCount++;
            if (CurrRunCount == 255)
            {
                bNeedNewPacket = TRUE;
                bIsGreater = TRUE;
            }


        
        }
        else
            bNeedNewPacket = TRUE;


        if (bNeedNewPacket)
        {
            /*  Store old run packet  */

            RLEPacket.Count = CurrRunCount; 
            RLEPacket.Value = CurrVal;

            PacketPtr   = (char *) &RLEPacket;
 
            *lpDest++ = *PacketPtr++;
            *lpDest++ = *PacketPtr++;
            *lpDest++ = *PacketPtr;

            if (CurrRunCount < 255)
                CurrVal         = NextVal;
            else
            {
                CurrVal = *lpSource++;
                wVals++;
            } 


            CurrRunCount    = 1;
            bNeedNewPacket  = FALSE;
            wRLEByteCount   += 3;

            bIsPending = TRUE;

        }
    } while (wVals < wNumVals);



    if (CurrRunCount > 1)
    {
            /*  Store pending run packet  */

            RLEPacket.Count = CurrRunCount; 
            RLEPacket.Value = CurrVal;

            PacketPtr   = (char *) &RLEPacket;
 
            *lpDest++ = *PacketPtr++;
            *lpDest++ = *PacketPtr++;
            *lpDest++ = *PacketPtr;
            wRLEByteCount   += 3;

    }



    *wCompressedBytes = wRLEByteCount;

}


#endif

