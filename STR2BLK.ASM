
;  


        DOSSEG
        .MODEL  MEDIUM
        .CODE   


        public    _StrToBlkCr_A
        public    _StrToBlkL_A

;
;       StrToBlkCr_A (lpDestCrPtr, lpSrcCrPtr, wNumBlocks, wValsPerRow);
;                           6  8        10  12        14          16
    

lpDest      equ               [bp + 6]
lpSource    equ               [bp + 10]
wNumBlocks  equ     Word Ptr  [bp + 14]
wValsPerRow equ     Word Ptr  [bp + 16]

SrcPtr      equ     Word Ptr  [bp - 4]
DestPtr     equ     Word Ptr  [bp - 8]
wAdjVals    equ     Word Ptr  [bp - 10]


_StrToBlkCr_A   proc     far



                push    bp
                mov     bp,sp
          
                sub     sp,30
                
                push    si
                push    di
                push    ds
                push    es

                lds     si,DWord Ptr [bp + 10]  ; Make DS:SI = lpSource
                les     di,DWord Ptr [bp + 6]   ; Make ES:DI = lpDest


                mov     ax, wValsPerRow
                sub     ax,8
                mov     wAdjVals,ax

                mov     SrcPtr,si
                mov     DestPtr,di
                

                cld

                mov     cx,wNumBlocks 
PixelLoop:      push    cx


                mov     si,SrcPtr
                mov     di,DestPtr



; Move 8 bytes 4 times.. 

                mov     cx,4
                rep     movsw                   ; Move 8 bytes (1st)
                add     si,wAdjVals

                mov     cx,4
                rep     movsw                   ; Move 8 bytes (2nd)
                add     si,wAdjVals

                mov     cx,4
                rep     movsw                   ; Move 8 bytes (3rd)
                add     si,wAdjVals

                mov     cx,4
                rep     movsw                   ; Move 8 bytes (4th)



                mov     si,SrcPtr
                add     si,8                    ; Get next "4-row" block




; Move 8 bytes 4 times again.. 

                mov     cx,4
                rep     movsw                   ; Move 8 bytes (1st)
                add     si,wAdjVals

                mov     cx,4
                rep     movsw                   ; Move 8 bytes (2nd)
                add     si,wAdjVals

                mov     cx,4
                rep     movsw                   ; Move 8 bytes (3rd)
                add     si,wAdjVals

                mov     cx,4
                rep     movsw                   ; Move 8 bytes (4th)


                add     SrcPtr,16
                add     DestPtr,64 


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
 
_StrToBlkCr_A   endp








;
;       StrToBlkL_A (lpDestPtr, lpSrcPtr, wNumBlocks, wValsPerRow);
;                     6  8       10  12     14          16
    

lpDest      equ               [bp + 6]
lpSource    equ               [bp + 10]
wNumBlocks  equ     Word Ptr  [bp + 14]
wValsPerRow equ     Word Ptr  [bp + 16]

SrcPtr      equ     Word Ptr  [bp - 4]
DestPtr     equ     Word Ptr  [bp - 8]
wAdjVals    equ     Word Ptr  [bp - 10]



_StrToBlkL_A    proc     far



                push    bp
                mov     bp,sp
          
                sub     sp,30
                
                push    si
                push    di
                push    ds
                push    es

                lds     si,DWord Ptr [bp + 10]  ; Make DS:SI = lpSource
                les     di,DWord Ptr [bp + 6]   ; Make ES:DI = lpDest


                mov     ax, wValsPerRow
                sub     ax,8
                mov     wAdjVals,ax

                mov     SrcPtr,si
                mov     DestPtr,di
                

                cld

                mov     cx,wNumBlocks 
PixelLoopL:     push    cx


                mov     si,SrcPtr
                mov     di,DestPtr



; Move 8 bytes 8 times.. 

                mov     cx,4
                rep     movsw                   ; Move 8 bytes (1st)
                add     si,wAdjVals

                mov     cx,4
                rep     movsw                   ; Move 8 bytes (2nd)
                add     si,wAdjVals

                mov     cx,4
                rep     movsw                   ; Move 8 bytes (3rd)
                add     si,wAdjVals

                mov     cx,4
                rep     movsw                   ; Move 8 bytes (4th)
                add     si,wAdjVals

                mov     cx,4
                rep     movsw                   ; Move 8 bytes (5th)
                add     si,wAdjVals

                mov     cx,4
                rep     movsw                   ; Move 8 bytes (6th)
                add     si,wAdjVals

                mov     cx,4
                rep     movsw                   ; Move 8 bytes (7th)
                add     si,wAdjVals

                mov     cx,4
                rep     movsw                   ; Move 8 bytes (8th)


                add     SrcPtr,8 
                add     DestPtr,64 


                pop     cx
                loop    PixelLoopL


doneL:
                pop     es
                pop     ds
                pop     di
                pop     si
                add     sp,30
                pop     bp
                ret
 
_StrToBlkL_A    endp







                end   
