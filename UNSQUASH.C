/*  
  
 This program decompresses CPI files.  
 It is intended to be the definitive model in implementing a "mega compression"  
  on RGB (CPI) file data.


*/


#include "usquash.h"
#include <cpi.h>
#include "oic.h"

int cpiunsquash (char *, char *, char far *, char far *, char far *);


BOOL bAbandon;

main (argc,argv)
int argc;
char *argv[];
{
   char *result;
   char far *ScanBuffer, far *Palette, far *EncBuffer;




   if (argc < 3)
   {   
       fputs ("\nusage:  squash    <filename>.cpi  <filename>.cpi\n\n",stdout); 
       exit (0);
   }


   if ((ScanBuffer = _fmalloc ((size_t) 65000)) == NULL)
   { 
       fputs ("\nCan't allocate Scan buffer  Exiting to DOS\n\n",stdout); 
       exit (1);
   }


   if ((Palette = _fmalloc ((size_t) 1024)) == NULL)
   { 
       fputs ("\nCan't allocate palette buffer  Exiting to DOS\n\n",stdout); 
       exit (1);
   }

   if ((EncBuffer = _fmalloc ((size_t) 65000)) == NULL)
   { 
       fputs ("\nCan't allocate encode buffer  Exiting to DOS\n\n",stdout); 
       exit (1);
   }

   if ((lpLZWDecodeBuf = _fmalloc ((size_t) 65000)) == NULL)
   { 
       fputs ("\nCan't allocate decode buffer  Exiting to DOS\n\n",stdout); 
       exit (1);
   }



   if ((lpLZWIOBuf = _fmalloc ((size_t) 4096)) == NULL)   
   { 
       fputs ("\nCan't allocate decode buffer  Exiting to DOS\n\n",stdout); 
       exit (1);
   }

   if ((lpXFRMBuf = _fmalloc ((size_t) 43690)) == NULL)   // Also used for idct (needs 21845)
   { 
       fputs ("\nCan't allocate decode buffer  Exiting to DOS\n\n",stdout); 
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


   if (! cpiunsquash (in_filename,out_filename, ScanBuffer, EncBuffer, Palette)) 
       exit (-1);

   return (1);

}







int cpiunsquash (oldname, newname, scan_buf, enc_buf, Palette) 
char *oldname;
char *newname;
char far *scan_buf;
char far *enc_buf;
char far *Palette;
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

        CalcMatrices (CPIImageSpec.CompLevel);



        /*  Find out how many strips so we know how many RLECounts are in the palette position  */

        {

            WORD wRLECountsSize;

            wRLECounts = (wImageHeight + 7) >> 3;  // wRLECounts is same as number of strips and is a counter 
                                                   // of how many RLE byte counts will be used in the compression.
            wRLECountsSize = (wRLECounts << 1);

            lseek (infh,CPIImageSpec.PalDataOffset, SEEK_SET);

            _dos_read (infh, Palette, wRLECountsSize, &bytes);  // Get wRLECounts
      
            /*  Point to input data  */

            lseek (infh, CPIImageSpec.ImgDataOffset, SEEK_SET);


        }






        /*  Initialize Output file  */


        InitCPIFileSpec ((LPCPIFILESPEC) &CPIFileSpec);

        _dos_write (outfh, (LPSTR) &CPIFileSpec, sizeof (CPIFILESPEC), &bytes);



        /*  Write out Image Spec stuff  */
        {

            InitCPIImageSpec ((LPCPIIMAGESPEC) &CPIImageSpec);


            CPIImageSpec.ImgDescTag       = 1;
            CPIImageSpec.ImgType          = CPIFMT_RGB;
            CPIImageSpec.ImgSpecLength    = sizeof (CPIIMAGESPEC);
            CPIImageSpec.X_Origin         = 0;
            CPIImageSpec.Y_Origin         = 0;   
            CPIImageSpec.X_Length         = wImageWidth;
            CPIImageSpec.Y_Length         = wImageHeight;


            CPIImageSpec.BitsPerPixel     = 24;

            CPIImageSpec.NumberPlanes     = 1;
            CPIImageSpec.NumberColors     = 0;
            CPIImageSpec.Orientation      = 0;    
            CPIImageSpec.Compression      = 0;    // Un-OIC'ed
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
 
            CPIImageSpec.CompLevel        = 0;
            CPIImageSpec.CodeSize         = 0;

            _dos_write (outfh, (LPSTR) &CPIImageSpec, sizeof (CPIIMAGESPEC), &bytes);

        }


        dwCPIPalPos = dwCPIDataPos  = tell (outfh);



        wBytesPerPixel  = CPIImageSpec.BitsPerPixel >> 3;
        wBytesPerRow    = wImageWidth * wBytesPerPixel;


        DecompressOIC (infh, outfh, enc_buf, scan_buf, wBytesPerRow, wImageWidth, wImageHeight, (WORD FAR *) Palette, lpXFRMBuf, lpLZWDecodeBuf, NULL);

    }     

    CPIOICFixupFile (outfh, (LPSTR) Palette, (LPCPIIMAGESPEC) &CPIImageSpec);


    _dos_close (infh);
    _dos_close (outfh);

    return (retval);
}



