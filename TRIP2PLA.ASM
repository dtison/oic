        .xlist
        include cmacros.inc
        .list

ifndef SEGNAME
  SEGNAME equ <TEXT>
endif

createSeg _%SEGNAME, CodeSeg, word, public, CODE

        INCLUDE CPIASM.MAC
        .MODEL  MEDIUM

        .CODE   


        public    _TripletsToPlanes  

;
;       TripletsToPlanes   (lpDest, lpSource, wScanWidth, wNumRows);  
;                         6  8     10  12    14          16
    

lpDest      equ               [bp + 6 ]
lpSource    equ               [bp + 10]
wScanWidth  equ     Word Ptr  [bp + 14]
wNumRows    equ     Word Ptr  [bp + 16]


wNumPixels  equ     Word Ptr  [bp - 4]
RedDest     equ     Word Ptr  [bp - 12]
GreenDest   equ     Word Ptr  [bp - 14]
BlueDest    equ     Word Ptr  [bp - 16]


_TripletsToPlanes       proc     far



                push    bp
                mov     bp,sp
          
                sub     sp,20
                
                push    si
                push    di
                push    ds
                push    es

                lds     si,DWord Ptr [bp + 10]   ; Make DS:SI = lpSource
                les     di,DWord Ptr [bp + 6]    ; Make ES:DI = lpDest

; Calculate wNumPixels = wNumRows * wScanWidth 

                mov     cx,wNumRows
                mov     ax,wScanWidth
                mul     cx
                mov     wNumPixels,ax


; Set up Dest pointers


                mov     bx,wNumPixels
                mov     GreenDest,bx
                add     bx,wNumPixels
                mov     BlueDest,bx

                cld

                mov     cx,wNumRows
RowLoop:        push    cx

                mov     cx,wScanWidth
PixelLoop:      push    cx
                 


                lodsb
                mov     dl,al
                lodsb      
                mov     ah,al
                lodsb      


                mov     Byte Ptr es:[di],dl  ; Store Red Value

                mov     bx,GreenDest
                mov     Byte Ptr es:[di + bx],ah  ; Store Green Value

                mov     bx,BlueDest
                mov     Byte Ptr es:[di + bx],al  ; Store Blue Value



; Move our pointers for next values

                inc     di


                pop     cx
                loop    PixelLoop
                pop     cx
                loop    RowLoop


                pop     es
                pop     ds
                pop     di
                pop     si
                add     sp,20
                pop     bp
                ret
 
_TripletsToPlanes         endp




                end   
