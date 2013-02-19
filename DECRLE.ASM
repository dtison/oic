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

        public    _DecodeRLE2  

;
;       DecodeRLE2    (lpDest, lpSource, wNumBytes);
;                      6  8     10  12    14      
    

wNumBytes   equ     Word Ptr  [bp + 14]

wNumVals    equ     Word Ptr  [bp - 6]


_DecodeRLE2     proc     far

                push    bp
                mov     bp,sp
          
                sub     sp,10
                
                push    si
                push    di
                push    ds
                push    es


                lds     si,DWord Ptr [bp + 10]    ; Make DS:SI = lpSource
                les     di,DWord Ptr [bp + 6]     ; Make ES:DI = lpDest


                mov     ax,wNumBytes
                shr     ax,1
                mov     wNumVals,ax

                xor     cx,cx
                xor     dx,dx                     ; DX is wRLEValCount

DecLoop:        

                lodsb
                mov     cl,al
                add     dx,cx

                lodsw

                rep     stosw

                cmp     dx,wNumVals
                jl      DecLoop


                pop     es
                pop     ds
                pop     di
                pop     si

                add     sp,10
                pop     bp
                ret
 
_DecodeRLE2     endp

                end
