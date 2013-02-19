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



        public      _AvgChrom  

;
;       AvgChrom   (lpDestCrPtr, lpSrcCrPtr, wNumPixels);
;                     6  8         10  12     14        
    


wNumPixels  equ     Word Ptr  [bp + 14]


SrcCrPtr    equ     Word Ptr  [bp - 4]
SrcCbPtr    equ     Word Ptr  [bp - 6]

DestCrPtr   equ     Word Ptr  [bp - 8]
DestCbPtr   equ     Word Ptr  [bp - 10]



_AvgChrom       proc     far



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
                add     bx,wNumPixels
                mov     SrcCbPtr,bx

                mov     bx,di
                mov     DestCrPtr,bx
                mov     ax,wNumPixels
                shr     ax,1
                add     bx,ax
                mov     DestCbPtr,bx
                
                cld

                mov     cx,wNumPixels 
PixelLoop:      push    cx


;  Average and store adjacent Cr values

                mov     bx,SrcCrPtr
                mov     ax,Word Ptr [bx]        ; Get SrcCrPtr1 and SrcCrPtr2
                xor     cx,cx
                xchg    cl,ah
                add     ax,cx
                shr     ax,1                    ; AL = avg (Cr1 + Cr2) / 2

                mov     bx,DestCrPtr
                mov     Byte Ptr es:[bx],al


;  Average and store adjacent Cb values

                mov     bx,SrcCbPtr
                mov     ax,Word Ptr [bx]        ; Get SrcCbPtr1 and SrcCbPtr2
                xor     cx,cx
                xchg    cl,ah
                add     ax,cx
                shr     ax,1                    ; AL = avg (Cr1 + Cr2) / 2

                mov     bx,DestCbPtr
                mov     Byte Ptr es:[bx],al



; Move our pointers for next values

                add     SrcCrPtr,2
                add     SrcCbPtr,2

                inc     DestCrPtr
                inc     DestCbPtr


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
 
_AvgChrom       endp




                end   
