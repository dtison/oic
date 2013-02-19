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

        public    _DeAvgCr_A  

;   DeAvgCr_A   (lpDestCrPtr1, lpSrcCrPtr, wNumPixels);
;                  6  8       10  12        14        
    

wNumPixels  equ     Word Ptr  [bp + 14]


SrcCrPtr    equ     Word Ptr  [bp - 4]
SrcCbPtr    equ     Word Ptr  [bp - 6]

DestCrPtr   equ     Word Ptr  [bp - 8]
DestCbPtr   equ     Word Ptr  [bp - 10]



_DeAvgCr_A        proc     far



                push    bp
                mov     bp,sp
          
                sub     sp,30
                
                push    si
                push    di
                push    ds
                push    es

                lds     si,DWord Ptr [bp + 10]   ; Make DS:SI = lpSource
                les     di,DWord Ptr [bp + 6]    ; Make ES:DI = lpDest


; Set up  pointers


                mov     bx,si
                mov     SrcCrPtr,bx
                mov     ax,wNumPixels
                shr     ax,1
                add     bx,ax
                mov     SrcCbPtr,bx

                mov     bx,di
                mov     DestCrPtr,bx
                add     bx,wNumPixels
                mov     DestCbPtr,bx
                
                cld

                mov     cx,wNumPixels 
                shr     cx,1

PixelLoop:      push    cx


                mov     si,SrcCrPtr
                lodsb

                mov     ah,al
                mov     di,DestCrPtr
                stosw


                mov     si,SrcCbPtr
                lodsb

                mov     ah,al
                mov     di,DestCbPtr
                stosw

; Move our pointers for next values

                add     DestCrPtr,2
                add     DestCbPtr,2

                inc     SrcCrPtr
                inc     SrcCbPtr


                pop     cx
                loop    PixelLoop


done:
                pop     es
                pop     ds
                pop     di
                pop     si
                add     sp,30
                pop     bp
                ret
 
_DeAvgCr_A        endp




                end   
