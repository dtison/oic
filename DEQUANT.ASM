
        .xlist
        include cmacros.inc
        .list

ifndef SEGNAME
  SEGNAME equ <TEXT>
endif

createSeg _%SEGNAME, CodeSeg, word, public, CODE

        INCLUDE CPIASM.MAC
        .MODEL  MEDIUM


        .DATA

extrn   _zigzag           : WORD
extrn   _LumQuantMatrix   : BYTE
extrn   _ChromQuantMatrix : BYTE

        .CODE   


        public    _DeQuantizeBlock  

;
;       DeQuantizeBlock   (lpDest, lpSource, bIsLuminance);
;                           6  8     10  12        14 
    

bIsLuminance    equ     Word Ptr  [bp + 14]

old_ds          equ     Word Ptr  [bp - 4]
MatrixTable     equ     Word Ptr  [bp - 8]
i               equ     Word Ptr  [bp - 10]
  
_DeQuantizeBlock       proc     far



                push    bp
                mov     bp,sp
          
                sub     sp,30
                
                push    si
                push    di
                push    ds
                push    es


                mov     ax,ds
                mov     old_ds,ax

                lea     ax,_LumQuantmatrix
                mov     MatrixTable + 2,ax

                lea     ax,_ChromQuantmatrix
                mov     MatrixTable,ax

                lds     si,DWord Ptr [bp + 10]  ; Make DS:SI = lpSource
                les     di,DWord Ptr [bp + 6]   ; Make ES:DI = lpDest

                cld

; Make sure bIsLuminance really contains 1 if not 0

                cmp     bIsLuminance,0
                je      LuminanceOK
                mov     bIsLuminance,1



                 
LuminanceOK:    mov     i,0

                mov     cx,64

ValueLoop:      push    cx

                push    es

                mov     es,old_ds
                lea     bx,_zigzag
                mov     ax,i
                shl     ax,1                    ; Word Index (zigzag)
                add     bx,ax
                mov     dx,Word Ptr es:[bx]

                mov     ax,bIsLuminance
                shl     ax,1
                lea     bx,MatrixTable
                add     bx,ax
                mov     bx,Word Ptr ss:[bx]     ; Get address off the stack

                add     bx,dx
                xor     cx,cx
                mov     cl,Byte Ptr es:[bx]        ; Get Lum (if bIsLuminance = 1) or Chrom Matrix 

                shl     dx,1
                mov     bx,dx
                mov     ax,Word Ptr [si + bx]

                mul     cx

                pop     es

                stosw


                pop     cx

                inc     i
                loop    ValueLoop 

done:
                pop     es
                pop     ds
                pop     di
                pop     si
                add     sp,30
                pop     bp
                ret
 
_DeQuantizeBlock               endp




                end   
