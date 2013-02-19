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

        public    _PlanesToTriplets  

;
;       PlanesToTriplets   (lpDest, lpSource, wScanWidth, wNumRows);  
;                           6  8   10  12    14          16
    

wScanWidth  equ     Word Ptr  [bp + 14]
wNumRows    equ     Word Ptr  [bp + 16]


wNumPixels  equ     Word Ptr  [bp - 4]
GreenSource equ     Word Ptr  [bp - 6]
BlueSource  equ     Word Ptr  [bp - 8]


_PlanesToTriplets       proc     far



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


; Set up pointers
; Calc offsets for green and blue planes

                mov     bx,wNumPixels       
                mov     GreenSource,bx
                add     bx,wNumPixels
                mov     BlueSource,bx

                cld

                mov     cx,wNumRows
RowLoop:        push    cx

                mov     cx,wScanWidth
PixelLoop:      push    cx
                 

                mov     al,Byte Ptr [si]

                mov     bx,GreenSource
                mov     ah,Byte Ptr [si + bx]

                mov     bx,BlueSource  
                mov     dl,Byte Ptr [si + bx]


                stosb
                mov     al,ah
                stosb
                mov     al,dl
                stosb


                inc     si


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
 
_PlanesToTriplets         endp




                end   
