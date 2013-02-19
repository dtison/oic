/*  
  
 This program counts colors

*/

#include "squash.h"



#define PRINT

#define GOFAST


BOOL bTestMode;

    char far *lpBuffer;
    char far *lpwHist;
    char far *lpbHist;

    char far *Palette;



long dwBreaks;




        long  RedDiff;
        unsigned Red;
        unsigned LastRed;
        unsigned AvgRed;


        long  GreenDiff;
        unsigned Green;
        unsigned LastGreen;
        unsigned AvgGreen;


        long  BlueDiff;
        unsigned Blue;
        unsigned LastBlue;
        unsigned AvgBlue;




        long dwNumPixels;


void main (argc,argv)
int argc;
char *argv[];
{
    char *result;
    short i;
    unsigned char ch;

    char *Oldname, *Newname;


   if (argc < 1)
   {   
       fputs ("\nusage:  cn        <filename>.cpi  \n\n",stdout); 
       exit (0);

  }

   if ((lpBuffer = _fmalloc (32768)) == NULL)
   { 
       fputs ("\nCan't allocate Scan buffer  Exiting to DOS\n\n",stdout); 
       exit (1);
   }


   if ((lpwHist = _fmalloc (65500)) == NULL)
   { 
       fputs ("\nCan't allocate encode buffer  Exiting to DOS\n\n",stdout); 
       exit (1);
   }


   if ((Palette = _fmalloc (1024)) == NULL)
   { 
       fputs ("\nCan't allocate palette buffer  Exiting to DOS\n\n",stdout); 
       exit (1);
   }


   if ((lpbHist = _fmalloc (32768)) == NULL)   /*  Need 40970 bytes  */

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


   if (! cpisquash (in_filename,out_filename)) 
       exit (-1);

}







int cpisquash (oldname, newname) 
char *oldname;
char *newname;
{
    int bytes;
    int scanbytes;
    int scanlines; 
    int i;
    short j;
    char retval = 0;
    int count;
    char *ptr;
    int *int_ptr;
    int num_lines;               
    long curr_offset = 0;
    long file_length;
    long next_buf_size;
    char *buf_ptr;
    int done = FALSE;
    int read_size;
    int linecount = 0;
    int lastbuf = FALSE;
    CPIFILESPEC   CPIFileSpec;
    CPIIMAGESPEC  CPIImageSpec;

    
    { 
        WORD wRowsPerStrip;
        WORD wBytesPerRow;
        WORD wBytesPerPixel;
        WORD wBytesPerStrip;

        char far *lpPtr;
  
        if (_dos_open (oldname, O_RDONLY, &infh) != 0)
            return (retval);



        /*  Get system metrics & image file sizes  */
    
        ReadCPIHdr (infh, (LPCPIFILESPEC) &CPIFileSpec, (LPCPIIMAGESPEC) &CPIImageSpec);

        wImageWidth  = CPIImageSpec.X_Length;
        wImageHeight = CPIImageSpec.Y_Length;


        dwNumPixels = ((long) wImageWidth * (long) wImageHeight);

        /*  Point to input data  */

        lseek (infh, CPIImageSpec.ImgDataOffset, SEEK_SET);




        wBytesPerPixel  = CPIImageSpec.BitsPerPixel >> 3;
        wBytesPerRow    = wImageWidth * wBytesPerPixel;

        wRowsPerStrip   = 32000 / wBytesPerRow;
        wBytesPerStrip  = wRowsPerStrip * wBytesPerRow;


        lmemset (lpwHist, 0, 65535);
        lmemset (lpbHist, 0, 32768);



        LastRed   = 0;
        LastGreen = 0;
        LastBlue  = 0;


        for (i = 0; i < wImageHeight; i++)
        {

            _dos_read (infh, lpBuffer, wBytesPerRow, &bytes);


            dwBreaks += CountBreaks (lpBuffer, wImageWidth);



        }

    }     

//  printf ("Breaks: %ld\n",dwBreaks);

    if (dwBreaks == 0)
        dwBreaks == 1;


    printf ("\nColorspace transitions detected: %ld\n",dwBreaks);
    printf ("Number pixels tested: %ld\n\n",dwNumPixels);

    printf ("\"Smoothness\" coefficent: %ld\n",(dwNumPixels / dwBreaks) + 1);

    _dos_close (infh);

    return (retval);
}


CountBreaks (lpBuffer, wSize)
LPSTR lpBuffer;
WORD  wSize;
{

    int i;

    WORD  wDiffAmt;
    WORD  wBreaks = 0;

    double Amt;
    double Amt2;

    int   Diff;

    for (i = 0; i < wSize; i++)
    {

        Red     = (unsigned char) *lpBuffer++;
        Green   = (unsigned char) *lpBuffer++;
        Blue    = (unsigned char) *lpBuffer++;

        Red >>= 3;
        Green >>= 3;
        Blue >>= 3;


        Diff    = Red - LastRed;
        wDiffAmt = (Diff * Diff);

        Diff    = Green - LastGreen;
        wDiffAmt += (Diff * Diff);

        Diff    = Blue - LastBlue;
        wDiffAmt += (Diff * Diff);


        LastRed = Red;
        LastGreen = Green;
        LastBlue = Blue;


//      Amt2 = (double) wDiffAmt;
//      Amt = sqrt (Amt2);

//      if (Amt > 64.0)
//          wBreaks++;


        if (wDiffAmt > 128)
            wBreaks++;

    }

    return (wBreaks);
}
