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

#define GOFAST

/*  UN-OIC   

    Returns TRUE if sucessful
            FALSE if an error occurs  

       For now, it's not completely accurate because we don't return an error properly
       from the LZW decoder...    */    


extern BOOL bAbandon;

int FAR PASCAL DecompressOIC (hInFile, hOutFile, lpBuffer1, lpBuffer2, wBytesPerRow, wScanWidth, wScanHeight, lpRLECounts, lpXFRMBuf, lpLZWDecodeBuf, lpOICMeter)
int     hInFile;
int     hOutFile;
LPSTR   lpBuffer1;
LPSTR   lpBuffer2;
WORD    wBytesPerRow;
WORD    wScanWidth;
WORD    wScanHeight;
WORD    FAR *lpRLECounts;
LPSTR   lpXFRMBuf;
LPSTR   lpLZWDecodeBuf;
LPOICMETER lpOICMeter;
{


    WORD      wBlocksPerRow;
    WORD      wBlocksPerAveragedRow;
    WORD      wBlocksPerColumn;
    WORD      wBytesPerStrip;
    WORD      wBlockedScanWidth;
    WORD      wBlockedScanHeight;
    WORD      wBytesPerBlockedStrip;
    WORD      wBytesPerAveragedStrip;


    WORD      wFlags;
    LPSTR     lpSource;
    LPSTR     lpDest;
    LPSTR     lpTmp;

    WORD      i;
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


    /*  Prometer  */

    EnableWindow (lpOICMeter -> hWnd, FALSE);
    ProOpen (lpOICMeter -> hWnd, NULL, MakeProcInstance (lpOICMeter -> lpfnAbandon, lpOICMeter -> hInst), lpOICMeter -> lpCaption);
    ProSetBarRange (wBlocksPerColumn);
    ProSetText (ID_STATUS1, lpOICMeter -> lpMessage1);


// LZW decompression stuff


        bIsBufEmpty   = TRUE;
        bIsLastRead  = FALSE;
        wInByteCount = 2048;
        CurrChunkSize = 2048;
        hLZWFile      = hInFile;




    for (i = 0; (i < wBlocksPerColumn  && ! bAbandon); i++)
    {



        /*  Grab a strip of CPI (compressed) raw data      */

//      _dos_read (hInFile, lpSource, (wBytesPerAveragedStrip << 1), &bytes);
 
        {

            WORD  wRLEBytes;


            if (i == 0)
                wFlags = 0;
            else
                wFlags = 1;

            lpTmp = lpSource;

//          DecodeLZW (hInFile, lpLZWDecodeBuf, lpDest, wBytesPerStrip, wFlags);
///         DecodeLZW (hInFile, lpLZWDecodeBuf, lpDest, (wBytesPerAveragedStrip << 1), wFlags);

            wRLEBytes = *lpRLECounts++;
            DecompressLZW (hInFile, lpLZWDecodeBuf, lpDest, (int) wRLEBytes, wFlags, LZW_CPI);
 

            lpTmp     = lpSource;
            lpSource  = lpDest;
            lpDest    = lpTmp;

        }       


        /*  RLE Test  */

        #ifdef NEVER
        {
 
            DecodeRLE (lpDest, (wBytesPerAveragedStrip << 1), i);   // (i is wFlags)

            lpTmp     = lpSource;
            lpSource  = lpDest;
            lpDest    = lpTmp;

        } 
        #endif
  

        {
            DecodeRLE2 ((WORD FAR *) lpDest, lpSource, (wBytesPerAveragedStrip << 1));  

            lpTmp     = lpSource;
            lpSource  = lpDest;
            lpDest    = lpTmp;

        }    


        /*  Perform the initial dequantization of DCT's from user-defined "squash rate"  */

        DeQuantizeDCTs (&lpDest, &lpSource, wBlockedScanWidth, wBlocksPerRow, wBlocksPerAveragedRow);

        lpTmp     = lpSource;
        lpSource  = lpDest;
        lpDest    = lpTmp;


        /*  Perform Inverse Discrete Cosine Transform  (on blocked data)   */
        InverseCosineTransform (&lpDest, &lpSource, wBlockedScanWidth, wBlocksPerRow, wBlocksPerAveragedRow, lpXFRMBuf);

        /*  Data now in adjacent blocks, put back into strips  */

        lpTmp     = lpSource;
        lpSource  = lpDest;
        lpDest    = lpTmp;
  

        BlocksToStrips (&lpDest, &lpSource, wBlockedScanWidth, wBlocksPerRow, wBlocksPerAveragedRow);


        /*  "De-Average adjacent pixels           */

        lpTmp     = lpSource;
        lpSource  = lpDest;
        lpDest    = lpTmp;


        DeAverageStrip (&lpDest, &lpSource, wBlockedScanWidth, wBlocksPerRow, wBlocksPerAveragedRow);

        /*  Transform from "YCRCB" to RGB            */

        lpTmp     = lpSource;
        lpSource  = lpDest;
        lpDest    = lpTmp;

        YCRCBToRGB (&lpDest, &lpSource, wBlockedScanWidth);



        /*  Convert from "blocks"  into triplets  */

        lpTmp     = lpSource;
        lpSource  = lpDest;
        lpDest    = lpTmp;
        StripsToTriplets (&lpDest, &lpSource, wBlocksPerRow, wBlockedScanWidth, wScanWidth);


        /*  Write out the file  */


// Test last strip here..

        {

            WORD  wWriteBytes;

            if (i == (wBlocksPerColumn - 1))
            {
                WORD wUnusedLines;

                wUnusedLines = (wBlocksPerColumn << 3) - wScanHeight;
                wWriteBytes = (8 - wUnusedLines) * wBytesPerRow;
            }
            else
                wWriteBytes = wBytesPerStrip;

            if (WriteFile (hOutFile, lpDest, (LONG) wWriteBytes) != (LONG) wWriteBytes)
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

    EnableWindow (lpOICMeter -> hWnd, TRUE);
    ProClose ();

    if (bAbandon)
        return (FALSE);
    else
        return (TRUE);
} 



void StripsToTriplets (plpDest, plpSource, wBlocksPerRow, wBlockedScanWidth, wScanWidth) 
LPSTR *plpDest;
LPSTR *plpSource;
WORD  wBlocksPerRow;
WORD  wBlockedScanWidth;
WORD  wScanWidth;
{

    LPSTR lpSource;
    LPSTR lpDest;
    LPSTR lpTmp;

    lpSource  = *plpSource;
    lpDest    = *plpDest;


    /*  Blocks are always 8 wide by 8 high.  Strips are padded to an even
        integer multiple of blocks  */

    PlanesToTriplets (lpDest, lpSource, wBlockedScanWidth, 8);

    lpTmp     = lpSource;
    lpSource  = lpDest;
    lpDest    = lpTmp;
  
    ClipScanlines (lpDest, lpSource, (wScanWidth * 3), (wBlockedScanWidth * 3), 8);

    *plpDest    = lpDest;
    *plpSource  = lpSource;

}





#ifdef CLIPSCANLINES
void ClipScanLines (lpDest, lpSource, wDestWidth, wSourceWidth, wNumRows)
LPSTR lpDest;
LPSTR lpSource;
WORD  wDestWidth;
WORD  wSourceWidth;
WORD  wNumRows;
{

    LPSTR lpSourcePtr;
    LPSTR lpDestPtr;
    WORD  i;
    WORD  wBytesPerSourceRow;
    WORD  wBytesPerDestRow;
    lpSourcePtr = lpSource;
    lpDestPtr   = lpDest;

    wBytesPerSourceRow  = wSourceWidth;
    wBytesPerDestRow    = wDestWidth;


  	for (i = 0; i < wNumRows; i++)
	  {
		    _fmemcpy (lpDestPtr, lpSourcePtr, wBytesPerDestRow);
		    lpSourcePtr += wBytesPerSourceRow;
		    lpDestPtr += wBytesPerDestRow;
		}
}

#endif


#ifndef GOFAST
void PlanesToTriplets (lpDest, lpSource, wScanWidth, wNumRows)
LPSTR lpDest;
LPSTR lpSource;
WORD  wScanWidth;
WORD  wNumRows;
{


    int i;
    int j;
    LPSTR  lpRedSource;
    LPSTR  lpGreenSource;
    LPSTR  lpBlueSource;

    LPSTR  lpRedDest;
    LPSTR  lpGreenDest;
    LPSTR  lpBlueDest;
    WORD   wNumPixels;

    wNumPixels = wScanWidth * wNumRows;


    lpRedSource   = lpSource;
    lpGreenSource = lpRedSource + wNumPixels;
    lpBlueSource  = lpGreenSource + wNumPixels;

    lpRedDest   = lpDest;
    lpGreenDest = lpRedDest + 1;
    lpBlueDest  = lpGreenDest + 1;

    for (i = 0; i < wNumRows; i++)
    {
        for (j = 0; j < wScanWidth; j++)
        {
    
            *lpRedDest    = *lpRedSource++;
            *lpGreenDest  = *lpGreenSource++;
            *lpBlueDest   = *lpBlueSource++;

            lpRedDest   += 3;
            lpGreenDest += 3;
            lpBlueDest  += 3;
        }
    }

}

#endif

void YCRCBToRGB (plpDest, plpSource, wBlockedScanWidth) 
LPSTR *plpDest;
LPSTR *plpSource;
WORD  wBlockedScanWidth;
{

    int  Luminance; 
    int  ChromR;
    int  ChromB;

    int Red, Green, Blue; 
    WORD  i;


    LPSTR lpRedPtr;
    LPSTR lpGreenPtr;
    LPSTR lpBluePtr;

    LPSTR lpLumPtr;
    LPSTR lpCrPtr;
    LPSTR lpCbPtr;


    LPSTR lpSource;
    LPSTR lpDest;
    WORD  wNumRows;
    WORD  wNumPixels;
    int   Norm;
    int   Normh;


    lpSource = *plpSource;
    lpDest   = *plpDest;

    wNumRows   = 8;
    wNumPixels = wBlockedScanWidth * wNumRows;
    Normh = 512;
    Norm = 64;    // (1024 / 16)

    lpLumPtr    = lpSource;
    lpCrPtr     = lpLumPtr + wNumPixels;
    lpCbPtr     = lpCrPtr  + wNumPixels;

    lpRedPtr    = lpDest;
    lpGreenPtr  = lpRedPtr    + wNumPixels;
    lpBluePtr   = lpGreenPtr  + wNumPixels;

    for (i = 0; i < wNumPixels; i++)
    {


        //  Get Red value


        Luminance = (BYTE) *lpLumPtr++;
        ChromR    = (BYTE) *lpCrPtr++;
        ChromB    = (BYTE) *lpCbPtr++;


        ChromR -= 128;
        ChromB -= 128;



        Red =  (Luminance << 6);    // (Luminance * 64) (1024 / 16)
        Red += (ChromR * 90);       // (1436 / 16)

        Red = (Red + Normh) >> 6;

        if (Red > 255)
            Red = 255;
        else
            if (Red < 0)
                Red = 0;


        //  Get Green value


        Green =  (Luminance << 6);    
        Green += (ChromR * -46);    // (-731 / 16)
        Green += (ChromB * -22);    // (-352 / 16)  


        Green = (Green + Normh) >> 6;

        if (Green > 255)
            Green = 255;
        else
            if (Green < 0)
               Green = 0;



        //  Get Blue value

        Blue =  (Luminance << 6);
        Blue += (ChromB * 113);   // (1815 / 16)

        Blue = (Blue + Normh) >> 6;

        if (Blue > 255)
            Blue = 255;
        else
            if (Blue < 0)
                Blue = 0;


        *lpRedPtr++   = (BYTE) Red;
        *lpGreenPtr++ = (BYTE) Green;
        *lpBluePtr++  = (BYTE) Blue;

    }   


    *plpDest    = lpDest;
    *plpSource  = lpSource;


}




void DeAverageStrip (plpDest, plpSource, wBlockedScanWidth, wBlocksPerRow, wBlocksPerAveragedRow)
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

    #ifndef GOFAST
    int   i;
    int   j;
    #endif

    LPSTR lpSrcLumPtr;
    LPSTR lpDestLumPtr;

    LPSTR lpSrcCrPtr;
    LPSTR lpSrcCbPtr;

    LPSTR lpDestCrPtr1;
    LPSTR lpDestCrPtr2;

    LPSTR lpDestCbPtr1;
    LPSTR lpDestCbPtr2;


    WORD  wBytesPerAveragedRow;

    lpSource = *plpSource;
    lpDest   = *plpDest;


    wNumRows   = 8;
    wNumPixels = wBlockedScanWidth * wNumRows;

    wBytesPerAveragedRow    = 8 * wBlocksPerAveragedRow;

    lpSrcLumPtr    = lpSource;
    lpDestLumPtr   = lpDest;


    lpSrcCrPtr = lpSrcLumPtr + wNumPixels;
    lpSrcCbPtr = lpSrcCrPtr  + (wNumPixels >> 1);  // 4 rows down


    /*  Point to "dest" block  */

    lpDestCrPtr1    = lpDestLumPtr + wNumPixels;
    lpDestCbPtr1    = lpDestCrPtr1 + wNumPixels;

 
    /*  Point to adjacent block  */

    lpDestCrPtr2    = lpDestCrPtr1 + 1;
    lpDestCbPtr2    = lpDestCbPtr1 + 1;


    /*  First just transfer Luminance plane verbatim  */

    _fmemcpy (lpDestLumPtr, lpSrcLumPtr, wNumPixels);



    /*  Next traverse 8 X 8 blocks, 	de-averaging adjacent values in each plane  */
 

    #ifdef GOFAST
    DeAvgCr_A (lpDestCrPtr1, lpSrcCrPtr, wNumPixels);
    #else

    for (i = 0; i < (wNumPixels >> 1); i++)
    {
        *lpDestCrPtr1 = *lpSrcCrPtr;
        *lpDestCrPtr2 = *lpSrcCrPtr;
        *lpDestCbPtr1 = *lpSrcCbPtr;
        *lpDestCbPtr2 = *lpSrcCbPtr;

        lpSrcCrPtr++;
        lpSrcCbPtr++;

        lpDestCrPtr1 += 2;
        lpDestCrPtr2 += 2;
        lpDestCbPtr1 += 2;
        lpDestCbPtr2 += 2;



    }     
    #endif


    *plpDest    = lpDest;
    *plpSource  = lpSource;

}





void InverseCosineTransform (plpDest, plpSource, wBlockedScanWidth, wBlocksPerRow, wBlocksPerAveragedRow, lpXFRMBuf)
LPSTR *plpDest;
LPSTR *plpSource;
WORD  wBlockedScanWidth;
WORD  wBlocksPerRow;
WORD  wBlocksPerAveragedRow;
LPSTR lpXFRMBuf;
{

    LPSTR lpSource;
    LPSTR lpDest;

    WORD  wBytesPerLumRow;
    WORD  wBytesPerChromRow;
    WORD  wNumRows;
    WORD  wNumPixels;
    WORD  i;

    LPSTR lpDestLumPtr;
    LPSTR lpDestCrPtr;
    LPSTR lpDestCbPtr;


    int   FAR *lpSrcLumPtr;
    int   FAR *lpSrcCrPtr;
    int   FAR *lpSrcCbPtr;



    lpSource = *plpSource;
    lpDest   = *plpDest;



    wNumRows   = 8; 
    wNumPixels = wBlockedScanWidth * wNumRows;

    wBytesPerLumRow         = (wBlocksPerRow * 8);
    wBytesPerChromRow       = (wBlocksPerAveragedRow * 8);


    lpSrcLumPtr = (int FAR *) lpSource;
    lpSrcCrPtr  = lpSrcLumPtr + wNumPixels;
    lpSrcCbPtr  = lpSrcCrPtr  + (wNumPixels >> 1);   // Only 4 rows down

    lpDestLumPtr = lpDest;
    lpDestCrPtr  = lpDestLumPtr + wNumPixels;
    lpDestCbPtr  = lpDestCrPtr  + (wNumPixels >> 1);   // Only 4 rows down


    /*  Transform Luminance  Blocks  */

    for (i = 0; i < wBlocksPerRow; i++)
    {

        idct (lpDestLumPtr, lpSrcLumPtr, 8, lpXFRMBuf);

        lpSrcLumPtr  += 64;
        lpDestLumPtr += 64;
    }


    /*  Transform ChromR  Blocks  */

    for (i = 0; i < wBlocksPerAveragedRow; i++)
    {
        idct (lpDestCrPtr, lpSrcCrPtr, 8, lpXFRMBuf);

        lpSrcCrPtr   += 64;
        lpDestCrPtr  += 64;
    }   


    /*  Transform ChromR  Blocks  */

    for (i = 0; i < wBlocksPerAveragedRow; i++)
    {
        idct (lpDestCbPtr, lpSrcCbPtr, 8, lpXFRMBuf);

        lpSrcCbPtr   += 64;
        lpDestCbPtr  += 64;
    }   



    *plpDest    = lpDest;
    *plpSource  = lpSource;


}






/*
   2-dimensional inverse discrete cosine transform
   input range: -1023..1023
   output is divided by 32 and level shifted by 128
   output range: 0..255
 */

void idct (xout, xfrm, wValuesPerRow, lpXFRMBuf)
BYTE FAR *xout;
int FAR   *xfrm;
WORD      wValuesPerRow;
LPSTR     lpXFRMBuf;
{

    #define   ilong int  


    ilong   i, j, p;
    ilong            k, k0[8];

//  ilong            a;


    int   FAR *xTmpFrm;
  
//  long FAR *lpxTmpOut;
    int  FAR *lpxTmpOut;


    ilong           k10, k11, k12, k13, k14, k15, k16, k17, k20, 
                    k21, k22, k23, k24, k25, k26, k27, k30, k31, k32, k33, 
                    k34, k35, k36, k37, k44, k45, k46, k47; 


    ilong            c1p4 = 23; 
    ilong            c1p8 = 30; 
    ilong            s1p8 = 12; 
    ilong            c1p16 = 31; 
    ilong            s1p16 = 6; 
    ilong            c3p16 = 27; 
    ilong            s3p16 = 18; 
    ilong            norm = 32;

    /* 1-dimensional transform on rows */


    lpxTmpOut = (int FAR *) lpXFRMBuf;
    xTmpFrm = (int FAR *) xfrm;

    p = 0;
    for (i = 0; i < 8; ++i) 
    {


        k30 = xTmpFrm[p];
        k31 = xTmpFrm[p + 4];
        k32 = xTmpFrm[p + 2];
        k33 = xTmpFrm[p + 6];
        k44 = xTmpFrm[p + 1];
        k45 = xTmpFrm[p + 5];
        k46 = xTmpFrm[p + 3];
        k47 = xTmpFrm[p + 7];


        k34 = (s1p16 * k44 - c1p16 * k47) / norm;
        k35 = (c3p16 * k45 - s3p16 * k46) / norm;
        k36 = (s3p16 * k45 + c3p16 * k46) / norm;
        k37 = (c1p16 * k44 + s1p16 * k47) / norm;

        k20 = c1p4 * (k30 + k31) / norm;
        k21 = c1p4 * (k30 - k31) / norm;
        k22 = (s1p8 * k32 - c1p8 * k33) / norm;
        k23 = (c1p8 * k32 + s1p8 * k33) / norm;
        k24 = k34 + k35;
        k25 = k34 - k35;
        k26 = k37 - k36;
        k27 = k37 + k36;

        k10 = k20 + k23;
        k11 = k21 + k22;
        k12 = k21 - k22;
        k13 = k20 - k23;
        k14 = k24;
        k15 = c1p4 * (k26 - k25) / norm;
        k16 = c1p4 * (k26 + k25) / norm;
        k17 = k27;

        lpxTmpOut[p] = k10 + k17;
        lpxTmpOut[p + 1] = k11 + k16;
        lpxTmpOut[p + 2] = k12 + k15;
        lpxTmpOut[p + 3] = k13 + k14;
        lpxTmpOut[p + 4] = k13 - k14;
        lpxTmpOut[p + 5] = k12 - k15;
        lpxTmpOut[p + 6] = k11 - k16;
        lpxTmpOut[p + 7] = k10 - k17;

        p += wValuesPerRow;
    }

    /* 1-dimensional transform on columns */
    for (j = 0; j < 8; ++j) {


        k30 = lpxTmpOut[j];
        k31 = lpxTmpOut[j + (4 * wValuesPerRow)];
        k32 = lpxTmpOut[j + (2 * wValuesPerRow)];
        k33 = lpxTmpOut[j + (6 * wValuesPerRow)];
        k44 = lpxTmpOut[j + (wValuesPerRow)];
        k45 = lpxTmpOut[j + (5 * wValuesPerRow)];
        k46 = lpxTmpOut[j + (3 * wValuesPerRow)];
        k47 = lpxTmpOut[j + (7 * wValuesPerRow)];   


        k34 = (s1p16 * k44 - c1p16 * k47) / norm;
        k35 = (c3p16 * k45 - s3p16 * k46) / norm;
        k36 = (s3p16 * k45 + c3p16 * k46) / norm;
        k37 = (c1p16 * k44 + s1p16 * k47) / norm;

        k20 = c1p4 * (k30 + k31) / norm;
        k21 = c1p4 * (k30 - k31) / norm;
        k22 = (s1p8 * k32 - c1p8 * k33) / norm;
        k23 = (c1p8 * k32 + s1p8 * k33) / norm;

        k24 = k34 + k35;
        k25 = k34 - k35;
        k26 = k37 - k36;
        k27 = k37 + k36;

        k10 = k20 + k23;
        k11 = k21 + k22;
        k12 = k21 - k22;
        k13 = k20 - k23;
        k14 = k24;

        k15 = c1p4 * (k26 - k25) / norm;
        k16 = c1p4 * (k26 + k25) / norm;

        k17 = k27;

        k0[0] = k10 + k17;
        k0[1] = k11 + k16;
        k0[2] = k12 + k15;
        k0[3] = k13 + k14;
        k0[4] = k13 - k14;
        k0[5] = k12 - k15;
        k0[6] = k11 - k16;
        k0[7] = k10 - k17;   


        k0[0] <<= 3;
        k0[1] <<= 3;
        k0[2] <<= 3;
        k0[3] <<= 3;
        k0[4] <<= 3;
        k0[5] <<= 3;
        k0[6] <<= 3;
        k0[7] <<= 3;



        p = j;
        for (i = 0; i < 8; ++i) 
        {
            if (k0[i] < 0) 
            {
                k = (k0[i] - 16) / 32 + 128;
                if (k < 0)
                    k = 0;
            } 
            else 
            {
                k = (k0[i] + 16) / 32 + 128;
                if (k > 255)
                    k = 255;
            }
            lpxTmpOut[p] = k;

            p += wValuesPerRow;
        }
    }


    {


        for (i = 0; i < 8; i++)
        {

            for (j = 0; j < 8; j++)
               xout [j] = (BYTE) lpxTmpOut[j];

            xout    += wValuesPerRow;
            lpxTmpOut += wValuesPerRow;

        }

    }





}



  
void BlocksToStrips (plpDest, plpSource, wValuesPerRow, wBlocksPerRow, wBlocksPerAveragedRow)
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

    /*  "De-block" Luminance strip  */

    #ifdef GOFAST
    BlkToStrL_A (lpDest, lpSource, wBlocksPerRow, wValuesPerRow);
    #else
    for (i = 0; i < wBlocksPerRow; i++)
    {

        lpSourcePtr = lpSource;
        lpDestPtr   = lpDest;

        for (j = 0; j < 8; j++)                 // Rows
        {   
            for (k = 0; k < 8; k++)             // Pixels
                lpDestPtr [k] = *lpSourcePtr++;

            lpDestPtr += wValuesPerRow; 
        }

        lpSource  += 64;
        lpDest    += 8;
    }   
    #endif


    lpSource  = *plpSource;
    lpDest    = *plpDest;

    lpSource  += wNumPixels;
    lpDest    += wNumPixels;




    /*  "De-block" ChromR  strip  */

    #ifdef GOFAST
    BlkToStrL_A (lpDest, lpSource, wBlocksPerAveragedRow, (wValuesPerRow >> 1));
    #else
    for (i = 0; i < wBlocksPerAveragedRow; i++)
    {
        lpSourcePtr = lpSource;
        lpDestPtr   = lpDest;

        for (j = 0; j < 8; j++)                 // Rows
        {   
            for (k = 0; k < 8; k++)             // Pixels
                lpDestPtr [k] = *lpSourcePtr++;

            lpDestPtr += (wValuesPerRow >> 1); 
        }

        lpSource  += 64;
        lpDest    += 8;
    }    
    #endif


    lpSource  = *plpSource;
    lpDest    = *plpDest;

    lpSource  += (wNumPixels + (wNumPixels >> 1));
    lpDest    += (wNumPixels + (wNumPixels >> 1));



    /*  "De-block" ChromB  strip  */

    #ifdef GOFAST
    BlkToStrL_A (lpDest, lpSource, wBlocksPerAveragedRow, (wValuesPerRow >> 1));
    #else
    for (i = 0; i < wBlocksPerAveragedRow; i++)
    {
        lpSourcePtr = lpSource;
        lpDestPtr   = lpDest;

        for (j = 0; j < 8; j++)                 // Rows
        {   
            for (k = 0; k < 8; k++)             // Pixels
                lpDestPtr [k] = *lpSourcePtr++;

            lpDestPtr += (wValuesPerRow >> 1); 
        }

        lpSource  += 64;
        lpDest    += 8;
    }   
    #endif

}
   
void DeQuantizeDCTs (plpDest, plpSource, wBlockedScanWidth, wBlocksPerRow, wBlocksPerAveragedRow)
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



   
    /*  DeQuantize Luminance Strip  */


    for (i = 0; i < wBlocksPerRow; i++)
    {
        DeQuantizeBlock (lpDestLumPtr, lpSrcLumPtr, TRUE);
  
        lpSrcLumPtr  += 64;
        lpDestLumPtr += 64;

    }


    /*  DeQuantize Chrominance Strip  */

    for (i = 0; i < wBlocksPerRow; i++)
    {
        DeQuantizeBlock (lpDestCrPtr, lpSrcCrPtr, FALSE);

        lpSrcCrPtr  += 64;
        lpDestCrPtr += 64;


    }

    *plpDest    = lpDest;
    *plpSource  = lpSource;

}




  
#ifndef GOFAST

int DeQuantizeBlock (lpDest, lpSource, bIsLuminance)
int FAR *lpDest;
int FAR *lpSource;
BOOL    bIsLuminance;
{
    int  i, k;

    for (i = 0; i < 64; i++)
    {
        k = zigzag [i];
        
        if (bIsLuminance)
            lpDest [i] = LumQuantMatrix [k] * lpSource [k];
        else
            lpDest [i] = ChromQuantMatrix [k] * lpSource [k];
        
    }

    return (0);
}

#endif



int LZWLineOut (lpBuffer, wBytesPerRow, lpDest)
LPSTR lpBuffer;
WORD wBytesPerRow;
LPSTR lpDest;
{



    _fmemcpy (lpDest, (LPSTR) lpBuffer, wBytesPerRow);

    return (TRUE);
}









/*  Word oriented run length decoder  */ 

//    Optimized 5-16-90  D. Ison  

#ifndef GOFAST
void DecodeRLE2 (lpDest, lpSource, wNumBytes)
WORD FAR *lpDest;
LPSTR lpSource;
WORD  wNumBytes;
{


    WORD  CurrVal;
    WORD  NextVal;

    BYTE  Count;  

    int   i;
    int   j;

    RLE   RLEPacket; 
    char  *PacketPtr;


    WORD  Val1, Val2, Value;

    WORD  CurrRunCount;
    BOOL  bNeedNewPacket;
    WORD  wRLEValCount;
    WORD  wNumVals;

    wNumVals = wNumBytes >> 1;
    wRLEValCount = 0;


    while (wRLEValCount < wNumVals)
    {


        Count = *lpSource++;
        Val1  = (BYTE) *lpSource++;
        Val2  = (BYTE) *lpSource++;


        Val2 <<= 8;

        Value = (Val1 | Val2);

        for (j = 0; j < Count; j++)
        {
            *lpDest++ = Value;
            wRLEValCount++;
        }


    }

}
#endif
  








/*  Word oriented run length decoder  */ 
/*  Temporarily out of service...     */

#ifdef NEVER

DecodeRLE (lpDest, wNumBytes, wFlags)
WORD FAR *lpDest;
WORD  wNumBytes;
WORD  wFlags;
{


    WORD  CurrVal;
    WORD  NextVal;

    BYTE  Count;  

    int   i;
    int   j;

    RLE   RLEPacket; 
    char  *PacketPtr;


    WORD  Val1, Val2, Value;

    WORD  CurrRunCount;
    BOOL  bNeedNewPacket;
    WORD  wRLEValCount;
    WORD  wNumVals;

    wNumVals = wNumBytes >> 1;
    wRLEValCount = 0;


    while (wRLEValCount < wNumVals)
    {

        Count = get_byte ();
        Val1  = get_byte ();
        Val2  = get_byte ();

        Val2 <<= 8;

        Value = (Val1 | Val2);

        for (j = 0; j < Count; j++)
        {
            *lpDest++ = Value;
            wRLEValCount++;
        }


    }

}

#endif
