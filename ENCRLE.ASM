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


        public    _EncodeRLE  

;
;       EncodeRLE    (lpDest, lpSource, wNumBytes, wOutBytes);
;                     6  8     10  12    14          16 
    

wNumBytes   equ     Word Ptr  [bp + 14]
wOutBytes   equ     Word Ptr  [bp + 16]

wRLEByteCnt equ     Word Ptr  [bp - 6]
wNumVals    equ     Word Ptr  [bp - 8]
wValCount   equ     Word Ptr  [bp - 10]


_EncodeRLE      proc     far



                push    bp
                mov     bp,sp
          
                sub     sp,10
                
                push    si
                push    di
                push    ds
                push    es

                lds     si,DWord Ptr [bp + 10]    ; Make DS:SI = lpSource
                les     di,DWord Ptr [bp + 6]     ; Make ES:DI = lpDest

                mov     bx,1                      ; BX is CurrRunCount
                mov     wRLEByteCnt,0


; Calculate wNumValues = wNumBytes >> 1, etc

                mov     ax,wNumBytes
                shr     ax,1
                mov     wNumVals,ax

                mov     cx,1                      ; CX is wVals


                lodsw                             
                mov     dx,ax                     ; DX = wCurrVal

RLELoop:          

                lodsw                             ; AX = wNextVal
                inc     cx

                cmp     ax,dx
                jne     NeedNewPacket

                inc     bx
                cmp     bx,255

                je      NeedNewPacket


DoPacket:       cmp     cx,wNumVals
                jl      RLELoop

Done:

                call    LastPacket

                pop     es
                pop     ds
                pop     di
                pop     si

                mov     bx,wOutBytes
                mov     ax,wRLEByteCnt
                mov     Word Ptr [bx],ax

                add     sp,10
                pop     bp
                ret
 
_EncodeRLE             endp

; This is done to minimize code branching  D. Ison  05-04-90


NeedNewPacket:  push    ax                        ; Save "NextVal"

                mov     al,bl                     ; BX was CurrRunCount
                stosb
                mov     ax,dx
                stosw

                pop     ax                        ; AX is "NextVal"

                cmp     bl,255
                je      IsFull     


                mov     dx,ax                     ; CurrVal = NextVal
                jmp     short NotFull   


IsFull:         lodsw
                mov     dx,ax                     ; CurrVal = *lpSource++;
                inc     cx

NotFull:   

                mov     bx,1                      ; wCurrRunCount = 1
                add     wRLEByteCnt,3

                jmp     short DoPacket



LastPacket      proc    near

                cmp     bx,1
                jle     LastPackDone

                mov     al,bl                     ; BX was CurrRunCount
                stosb
                mov     ax,dx
                stosw

                add     wRLEByteCnt,3             ; 3 more bytes written

LastPackDone:   ret

LastPacket      endp

                end   
