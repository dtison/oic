/*  
  
 This program compresses CPi files.  
 It is intended to be the reference for OIC compression.
 on RGB (CPI) file data.

*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <malloc.h> 
#include <cpi.h>
#include <cpifmt.h>
#include <oic.h>

#ifdef TEST
extern BOOL bTestMode;
extern int  hTestFile;
#endif

int   infh;
int   outfh;
char *scan_buf;
char in_filename[13];
char out_filename[13];
unsigned bytes;

DWORD   dwCPIPalPos;
DWORD   dwCPIDataPos; 

int   CompLevel;        // (Pre-scaled)

WORD wImageWidth;
WORD wImageHeight;

WORD wRLECounts;

int cpisquash (char *, char *, char far *, char far *, char far *, LPSTR);


// Put in for IP 3.1 release   9/90.
// For progress dialog & cancel stuff

BOOL bAbandon;

void main (argc,argv)
int argc;
char *argv[];
{
   char *result;


   char far*ScanBuffer, far *Palette, far *EncBuffer;


   char far *lpHashTable;

   if (argc < 3)
   {   
       fputs ("\nusage:  squash    <filename>.cpi  <filename>.cpi  <factor>\n\n",stdout); 
       exit (0);
   }



    #ifdef TEST
    if (argc > 3)
        if (strnicmp (argv [4], "TEST", 4) == 0)
            bTestMode = TRUE;
        else
            bTestMode = FALSE;
    if (bTestMode)
        printf ("\n\nTest mode \n\n\n");
    #endif



/*  Handle the quality factor stuff  */


    CompLevel = atoi (argv [3]);

    if (CompLevel == 0)
        CompLevel = 55;   // (Default) (results in 81 on EFI scale)

    if (CompLevel < 5)
        CompLevel = 5;
    else
        if (CompLevel > 100)
            CompLevel = 100;


    CalcMatrices (CompLevel);



   if ((ScanBuffer = _fmalloc ((size_t) 65000)) == NULL)
   { 
       fputs ("\nCan't allocate Scan buffer  Exiting to DOS\n\n",stdout); 
       exit (1);
   }


   if ((Palette = _fmalloc (1024)) == NULL)
   { 
       fputs ("\nCan't allocate palette buffer  Exiting to DOS\n\n",stdout); 
       exit (1);
   }

   if ((EncBuffer = _fmalloc ((size_t) 65000)) == NULL)
   { 
       fputs ("\nCan't allocate encode buffer  Exiting to DOS\n\n",stdout); 
       exit (1);
   }

   if ((lpHashTable = _fmalloc ((size_t) 48000)) == NULL)   /*  Need 40970 bytes  */

   { 
       fputs ("\nCan't allocate hash buffer  Exiting to DOS\n\n",stdout); 
       exit (1);
   }



   strncpy (in_filename,argv[1],12);
   strncpy (out_filename,argv[2],12);


   if ((result = (strchr (in_filename,'.'))) != NULL)
       *result = '\0';
   strncat (in_filename,".CPI",4);

   if ((result = (strchr (out_filename,'.'))) != NULL)
       *result = '\0';
   strncat (out_filename,".CPI",4);


   if (! cpisquash (in_filename,out_filename, ScanBuffer, EncBuffer, Palette, lpHashTable)) 
       exit (-1);

}







int cpisquash (oldname, newname, scan_buf, enc_buf, Palette, lpHashTable) 
char *oldname;
char *newname;
char far *scan_buf;
char far *enc_buf;
char far *Palette;
LPSTR  lpHashTable;
{
    int bytes;
    char retval = 0;
    long curr_offset = 0;
    int done = FALSE;
    int linecount = 0;
    int lastbuf = FALSE;
    CPIFILESPEC   CPIFileSpec;
    CPIIMAGESPEC  CPIImageSpec;

    
    if (Palette == NULL)      /*  Means use input file's palette  */
        return (retval);  
    else
    { 
        WORD wBytesPerRow;
        WORD wBytesPerPixel;

  
        if (_dos_open (oldname, O_RDONLY, &infh) != 0)
            return (retval);

        if (_dos_creat (newname, _A_NORMAL, &outfh) != 0)
            return (retval);



        /*  Get system metrics & image file sizes  */
    
        ReadCPIHdr (infh, (LPCPIFILESPEC) &CPIFileSpec, (LPCPIIMAGESPEC) &CPIImageSpec);

        wImageWidth  = CPIImageSpec.X_Length;
        wImageHeight = CPIImageSpec.Y_Length;


        /*  Point to input data  */

        lseek (infh, CPIImageSpec.ImgDataOffset, SEEK_SET);



        /*  Initialize Header  */


        InitCPIFileSpec ((LPCPIFILESPEC) &CPIFileSpec);

        _dos_write (outfh, (LPSTR) &CPIFileSpec, sizeof (CPIFILESPEC), &bytes);


        /*  Write out Image Spec stuff  */
        {

            InitCPIImageSpec ((LPCPIIMAGESPEC) &CPIImageSpec);


            CPIImageSpec.ImgDescTag       = 1;
            CPIImageSpec.ImgSpecLength    = sizeof (CPIIMAGESPEC);
            CPIImageSpec.X_Origin         = 0;
            CPIImageSpec.Y_Origin         = 0;   
            CPIImageSpec.X_Length         = wImageWidth;
            CPIImageSpec.Y_Length         = wImageHeight;


            CPIImageSpec.BitsPerPixel     = 24;

            CPIImageSpec.NumberPlanes     = 1;
            CPIImageSpec.NumberColors     = 256;
            CPIImageSpec.Orientation      = 0;    
            CPIImageSpec.Compression      = 2;    // OIC
            CPIImageSpec.ImgSize          = (DWORD) ((DWORD) wImageWidth * 
                                            (DWORD) wImageHeight * 3L);

            CPIImageSpec.ImgSequence      = 0;
            CPIImageSpec.NumberPalEntries = 0;
            CPIImageSpec.NumberBitsPerEntry = 24;
            CPIImageSpec.AttrBitsPerEntry = 0;
            CPIImageSpec.FirstEntryIndex  = NULL;
            CPIImageSpec.RGBSequence      = 0;
            CPIImageSpec.NextDescriptor   = NULL;
            CPIImageSpec.ApplSpecReserved = NULL;        
 
            CPIImageSpec.CompLevel        = (BYTE) CompLevel;
            CPIImageSpec.CodeSize         = 8;

            _dos_write (outfh, (LPSTR) &CPIImageSpec, sizeof (CPIIMAGESPEC), &bytes);

        }


        /*  Find out how many strips so we know how big to make RLECounts Buffer  */

        {

            WORD wRLECountsSize;

            wRLECounts = (wImageHeight + 7) >> 3;  // wRLECounts is same as number of strips and is a counter 
                                                   // of how many RLE byte counts will be used in the compression.
            wRLECountsSize = (wRLECounts << 1);

            lmemset (Palette, 0, wRLECountsSize);

            dwCPIPalPos = tell (outfh);

            _dos_write (outfh, Palette, wRLECountsSize, &bytes);

            dwCPIDataPos = tell (outfh);


        }


        wBytesPerPixel  = CPIImageSpec.BitsPerPixel >> 3;
        wBytesPerRow    = wImageWidth * wBytesPerPixel;

        CompressOIC (infh, outfh, enc_buf, scan_buf, wBytesPerRow, wImageWidth, wImageHeight, (WORD FAR *) Palette, lpHashTable, NULL);
        //          -->     -->     -->                                                                                           ("NULL" is progress meter stuff - N/A in DOS)
    }     

    CPIOICFixupFile (outfh, Palette, (LPCPIIMAGESPEC) &CPIImageSpec);




    /*  Code to calculate compression ratio  */

    {

        DWORD dwInFileSize;
        DWORD dwOutFileSize;
        double Ratio; 


        dwInFileSize  = lseek (infh, 0L, SEEK_END);
        dwOutFileSize = lseek (outfh, 0L, SEEK_END);


        Ratio = ((double) dwInFileSize / (double) dwOutFileSize);

        printf ("%4.1f : 1\n",Ratio);

    }

    _dos_close (infh);
    _dos_close (outfh);

    return (retval);
}


