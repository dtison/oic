#include "usquash.h"
#include <math.h>
#include <stdio.h>

int NEAR ScaleFactor (int);   // Internal function

void CalcMatrices (CompLevel) 
int CompLevel; 
{

    if (CompLevel == 1)     // Special case
        CompLevel = 0;

    CompFactor = ScaleFactor (100 - CompLevel);

    CompFactor = Q_FACTOR_MAX - (CompFactor - 1) * (Q_FACTOR_MAX - Q_FACTOR_MIN) / (100 - 1); 


    /*  Set up Luminance / Chrominance quantization matrices  */

    {
        int i;
        long kq;


        for (i = 0; i < 64; ++i) 
        {
            kq = (CompFactor * q0 [0] [i] + 5) / 10;

            if (kq < 1)
                kq = 1;
            if (kq > 255)
                kq = 255;

            LumQuantMatrix [i] = (BYTE) kq;
        }



        for (i = 0; i < 64; ++i) 
        {
            kq = (CompFactor * q0 [1] [i] + 5) / 10;

            if (kq < 1)
                kq = 1;
            if (kq > 255)
                kq = 255;

            ChromQuantMatrix [i] = (BYTE) kq;
        }

    }

}



int NEAR ScaleFactor (nFactor)
int nFactor; 
{
    double v1;
    double v2;


    v1 = 100.0 / (log (100.0));

    v2 = log ((double) nFactor) * v1;

    v2 = v2 - 3.0;

    return ((int) v2);
}



int CPIOICFixupFile (hFile, lpPalette, lpCPIImageSpec)
int hFile;
LPSTR lpPalette;
LPCPIIMAGESPEC lpCPIImageSpec;
{


    int nRetval = TRUE;

  /*  Finish up CPI file header information and close files  */


    lpCPIImageSpec -> ImgDataOffset = dwCPIDataPos; 
    lpCPIImageSpec -> PalDataOffset = dwCPIPalPos; 

    lseek (hFile, 64L, SEEK_SET);

    _dos_write (hFile, (LPSTR) lpCPIImageSpec, sizeof (CPIIMAGESPEC), &bytes);


    if (lpCPIImageSpec -> ImgDataOffset > lpCPIImageSpec -> PalDataOffset)  
        _dos_write (hFile, lpPalette, (wRLECounts << 1), &bytes);


    return (nRetval);
}



  
_printf (pMessage, hWndOIC, bIsWindows)
PSTR pMessage;
HANDLE  hWndOIC;
BOOL bIsWindows;
{

    return (0);
}



