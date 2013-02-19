char Buffer [4000];

main ()
{
    int i;
    unsigned bytes;

    char far *lpSource;
    char far *lpDest;

    lpDest = (char far *) Buffer; 


    lpSource = (char far *) 0xb000;
    lpSource = ((long) lpSource << 16);


    for (i = 0; i < 4000; i++)
    {
        *lpDest++ = *lpSource;
        lpSource += 2;
    }

    _dos_write (1, (char far *) Buffer, 2000, &bytes);


}
