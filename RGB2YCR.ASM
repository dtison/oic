
;  4-25-90  This version has Chrom R and B "pushed" a little to match (modified) ecomp


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


        public    _RGBToYCRCB  

;
;       RGBToYCRCB    (lpDest, lpSource, wScanWidth);
;                     6  8     10  12    14        
    

lpDest      equ               [bp + 6 ]
lpSource    equ               [bp + 10]
wScanWidth  equ     Word Ptr  [bp + 14]
wNumRows    equ     Word Ptr  [bp + 16]


wNumPixels  equ     Word Ptr  [bp - 4]
LumPtr      equ     Word Ptr  [bp - 6]
ChromRPtr   equ     Word Ptr  [bp - 8]
ChromBPtr   equ     Word Ptr  [bp - 10]
RedPtr      equ     Word Ptr  [bp - 12]
GreenPtr    equ     Word Ptr  [bp - 14]
BluePtr     equ     Word Ptr  [bp - 16]
Luminance   equ     Word Ptr  [bp - 18]
ChromR      equ     Word Ptr  [bp - 20]
ChromB      equ     Word Ptr  [bp - 22]
Red         equ     Byte Ptr  [bp - 24]
Green       equ     Byte Ptr  [bp - 26]
Blue        equ     Byte Ptr  [bp - 28]

; Normh       equ      0  ; ("push" Chrom with new val below)

  Normh       equ     64  ; (def)
; Normh       equ     512



_RGBToYCRCB       proc     far



                push    bp
                mov     bp,sp
          
                sub     sp,30
                
                push    si
                push    di
                push    ds
                push    es

                lds     si,DWord Ptr [bp + 10]   ; Make DS:SI = lpSource
                les     di,DWord Ptr [bp + 6]    ; Make ES:DI = lpDest

; Calculate wNumPixels = wNumRows * wScanWidth 

                mov     wNumRows,8

                mov     cx,wNumRows
                mov     ax,wScanWidth
                mul     cx
                mov     wNumPixels,ax


; Set up  pointers


                mov     bx,0
                mov     RedPtr,bx
                add     bx,wNumPixels
                mov     GreenPtr,bx
                add     bx,wNumPixels
                mov     BluePtr,bx
                

                mov     bx,0
                mov     LumPtr,bx
                add     bx,wNumPixels
                mov     ChromRPtr,bx
                add     bx,wNumPixels
                mov     ChromBPtr,bx


                cld

                mov     cx,wNumPixels 
PixelLoop:      push    cx
                 

                mov     bx,RedPtr
                mov     al,Byte Ptr [si + bx]     ;  Get Red Value
                shr     al,1
                shr     al,1
                mov     Red, al

                mov     bx,GreenPtr
                mov     al,Byte Ptr [si + bx]     ;  Get Green Value
                shr     al,1
                shr     al,1
                mov     Green, al


                mov     bx,BluePtr
                mov     al,Byte Ptr [si + bx]     ;  Get Blue Value
                shr     al,1
                shr     al,1
                mov     Blue, al



; Get Luminance Value

                xor     ah,ah   
                mov     al,Red
                mov     cx,153
                mul     cx

                mov     bx,ax

                xor     ah,ah   
                mov     al,Green
                mov     cx,300
                mul     cx

                add     bx,ax

                xor     ah,ah   
                mov     al,Blue 
                mov     cx,58  
                mul     cx

                add     bx,ax
                add     bx,64                     ; (DONT "PUSH" LUMINANCE)
                mov     cl,7
                shr     bx,cl


                cmp     bx,0
                jg      lugr1
                mov     bx,0
                jmp     short lu1

lugr1:          cmp     bx,255
                jle     lu1
                mov     bx,255

lu1:            mov     Luminance, bx


; Get Chrominance (R) Value

                xor     ah,ah   
                mov     al,Red
                mov     cl,8
                shl     ax,cl                     ; (Red * 256)

                mov     bx,ax

                xor     ah,ah   
                mov     al,Green
                mov     cx,-214
                imul    cx

                add     bx,ax

                xor     ah,ah   
                mov     al,Blue 
                mov     cx,-41 
                imul    cx

                add     bx,ax

                cmp     bx,0
                jg      crgr1
                sub     bx,Normh
                jmp     short cr2
crgr1:          add     bx,Normh


cr2:            mov     cl,7
                sar     bx,cl
                add     bx,128



                cmp     bx,0
                jg      crgr2
                mov     bx,0
                jmp     short cr3

crgr2:          cmp     bx,255
                jle     cr3
                mov     bx,255

cr3:            mov     ChromR, bx



                



; Get Chrominance (B) Value

                xor     ah,ah   
                mov     al,Red  
                mov     cx,-87 
                imul    cx

                mov     bx,ax

                xor     ah,ah   
                mov     al,Green
                mov     cx,-169
                imul    cx

                add     bx,ax

                xor     ah,ah   
                mov     al,Blue 
                mov     cl,8
                shl     ax,cl

                add     bx,ax

                cmp     bx,0
                jg      cbgr1
                sub     bx,Normh
                jmp     short cb2
cbgr1:          add     bx,Normh


cb2:            mov     cl,7
                sar     bx,cl
                add     bx,128

                cmp     bx,0
                jg      cbgr2
                mov     bx,0
                jmp     short cb3

cbgr2:          cmp     bx,255
                jle     cb3
                mov     bx,255

cb3:            mov     ChromB, bx


                mov     ax,Luminance
                mov     bx,LumPtr 
                mov     Byte Ptr es:[di + bx],al  ; Store Luminance Value

                mov     ax,ChromR
                mov     bx,ChromRPtr 
                mov     Byte Ptr es:[di + bx],al  ; Store ChromR Value

                mov     ax,ChromB
                mov     bx,ChromBPtr 
                mov     Byte Ptr es:[di + bx],al  ; Store ChromB Value


; Move our pointers for next values

                inc     si
                inc     di


                pop     cx
                dec     cx
                cmp     cx,0
                je      done
                jmp     FAR Ptr PixelLoop



done:
                pop     es
                pop     ds
                pop     di
                pop     si
                add     sp,30
                pop     bp
                ret
 
_RGBToYCRCB               endp




                end   
