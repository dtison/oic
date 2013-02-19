        .xlist

        include cmacros.inc
        .list


ifndef SEGNAME
  SEGNAME equ <TEXT>
endif

createSeg _%SEGNAME, CodeSeg, word, public, CODE

        INCLUDE CPIASM.MAC
        .MODEL  MEDIUM

        ifdef USEDATA
        .DATA
                
            db  49h,6Dh,61h,67h,65h,20h,43h,6Fh,6Dh,70h,72h,65h,73h,73h,69h,6Fh 
            db  6Eh,20h,53h,79h,73h,74h,65h,6Dh,20h,0Dh,0Ah,43h,72h,65h,61h,74h
            db  65h,64h,20h,62h,79h,20h,20h,44h,61h,76h,69h,64h,20h,54h,2Eh,20h
            db  49h,73h,6Fh,6Eh,20h,0Dh,0Ah,53h,70h,72h,69H,6Eh,67h,20h,31h,39h
            db  39h,30h,2Eh,0Dh,0Ah,20h,20h,20h,20h,20h,20H,20h,20h,20h,20h,20h
        endif

        .CODE   





        public    _fdct  

;
;       fdct    (xfrm, xin,     wValsPerRow);
;                6 8   10 12    14        
    
c1p4        equ     23
c1p8        equ     30
s1p8        equ     12
c1p16       equ     31
s1p16       equ      6
c3p16       equ     27
s3p16       equ     18
k4_offset   equ     24

wValsPerRow equ     Word Ptr  [bp + 14]


k           equ     Word Ptr  [bp - 4]
k4          equ     Word Ptr  [bp - k4_offset]  ; k4 is 16 bytes wide

k00         equ     Word Ptr  [bp - 26]
k01         equ     Word Ptr  [bp - 28]
k02         equ     Word Ptr  [bp - 30]
k03         equ     Word Ptr  [bp - 32]
k04         equ     Word Ptr  [bp - 34]
k05         equ     Word Ptr  [bp - 36]
k06         equ     Word Ptr  [bp - 38]
k07         equ     Word Ptr  [bp - 40]
k10         equ     Word Ptr  [bp - 42]
k11         equ     Word Ptr  [bp - 44]
k12         equ     Word Ptr  [bp - 46]
k13         equ     Word Ptr  [bp - 48]
k14         equ     Word Ptr  [bp - 50]
k15         equ     Word Ptr  [bp - 52]
k16         equ     Word Ptr  [bp - 54]
k17         equ     Word Ptr  [bp - 56]
k20         equ     Word Ptr  [bp - 58]
k21         equ     Word Ptr  [bp - 60]
k22         equ     Word Ptr  [bp - 62]
k23         equ     Word Ptr  [bp - 64]
k24         equ     Word Ptr  [bp - 66]
k25         equ     Word Ptr  [bp - 68]
k26         equ     Word Ptr  [bp - 70]
k27         equ     Word Ptr  [bp - 72]
k34         equ     Word Ptr  [bp - 74]
k35         equ     Word Ptr  [bp - 76]
k36         equ     Word Ptr  [bp - 78]
k37         equ     Word Ptr  [bp - 80]
xfrmptr     equ     Word Ptr  [bp - 82]
xinptr      equ     Word Ptr  [bp - 84]
col_count   equ     Word Ptr  [bp - 86]
pptr        equ     Word Ptr  [bp - 88]




_fdct           proc     far



                push    bp
                mov     bp,sp
          
                sub     sp,100
                
                push    si
                push    di
                push    ds
                push    es

                lds     si,DWord Ptr [bp + 10]   ; Make DS:SI = lpxin   (source)
                les     di,DWord Ptr [bp + 6]    ; Make ES:DI = lpxfrm  (dest)




; Set up  pointers

                mov     xinptr,si
                mov     xfrmptr,di


                cld

; 1-dimensional transform on rows   

                mov     cx,8
RowLoop:        push    cx
                 
                mov     si,xinptr

                xor     cx,cx

                lodsw                             ; xin [p] in AL, xin [p + 1] in AH
                mov     cl,al
                mov     k00,cx
                mov     cl,ah
                mov     k01,cx

                lodsw                            
                mov     cl,al
                mov     k02,cx
                mov     cl,ah
                mov     k03,cx

                lodsw      
                mov     cl,al
                mov     k04,cx
                mov     cl,ah
                mov     k05,cx

                lodsw      
                mov     cl,al
                mov     k06,cx
                mov     cl,ah
                mov     k07,cx



                mov     ax,k00
                mov     cx,ax
                mov     dx,k07

                add     ax,dx
                shl     ax,1
                shl     ax,1

                sub     cx,dx
                shl     cx,1
                shl     cx,1
    
                mov     k10,ax
                mov     k17,cx
                mov     k27,cx



                mov     ax,k01
                mov     cx,ax
                mov     dx,k06

                add     ax,dx
                shl     ax,1
                shl     ax,1

                sub     cx,dx
                shl     cx,1
                shl     cx,1
    
                mov     k11,ax
                mov     k16,cx


                mov     ax,k02
                mov     cx,ax
                mov     dx,k05

                add     ax,dx
                shl     ax,1
                shl     ax,1

                sub     cx,dx
                shl     cx,1
                shl     cx,1
    
                mov     k12,ax
                mov     k15,cx


                mov     ax,k03
                mov     cx,ax
                mov     dx,k04

                add     ax,dx
                shl     ax,1
                shl     ax,1

                sub     cx,dx
                shl     cx,1
                shl     cx,1
    
                mov     k13,ax
                mov     k14,cx


; While we're at it, set k24

                mov     k24,cx



                mov     ax,k10
                mov     cx,ax
                mov     dx,k13 

                add     ax,dx
                sub     cx,dx
    
                mov     k20,ax
                mov     k23,cx



                mov     ax,k11
                mov     cx,ax
                mov     dx,k12

                add     ax,dx
                sub     cx,dx
    
                mov     k21,ax
                mov     k22,cx



                mov     ax,k16
                mov     cx,ax
                mov     dx,k15

                add     ax,dx
                sub     cx,dx


                push    cx                ; save difference

                sar     ax,1              ; 1 1st
                mov     cx,c1p4
                imul    cx
                mov     cx,4              ; then 4
                sar     ax,cl
                mov     k26,ax

                pop     ax
                sar     ax,1              ; 1 on first k25 & k26 calc (2 on 2nd)
                mov     cx,c1p4           
                imul    cx
                mov     cx,4              ; then 4
                sar     ax,cl
                mov     k25,ax


                mov     ax,k20
                add     ax,k21
                sub     ax,4096
                mov     cx,3
                sar     ax,cl

                mov     cx,c1p4
                imul    cx

                mov     cx,2
                sar     ax,cl

                mov     bx,xfrmptr
                mov     Word Ptr es:[bx],ax   ; xfrm [p] = a



                mov     ax,k20
                sub     ax,k21
                mov     cx,2
                sar     ax,cl

                mov     cx,c1p4
                imul    cx

                mov     cx,3
                sar     ax,cl

                mov     bx,xfrmptr
                add     bx,8                  ; (xfrm [p + 4])
                mov     Word Ptr es:[bx],ax   




                mov     ax,k23
                mov     cx,2
                sar     ax,cl

                mov     cx,c1p8
                imul    cx

                mov     bx,ax                 ; Save momentarily


                mov     ax,k22
                mov     cx,2
                sar     ax,cl

                mov     cx,s1p8
                imul    cx
                
                add     ax,bx

                mov     cx,3
                sar     ax,cl

                mov     bx,xfrmptr
                add     bx,4                  ; (xfrm [p + 2])
                mov     Word Ptr es:[bx],ax   







                mov     ax,k23
                mov     cx,2
                sar     ax,cl

                mov     cx,s1p8
                imul    cx

                mov     bx,ax                 ; Save momentarily


                mov     ax,k22
                mov     cx,2
                sar     ax,cl

                mov     cx,c1p8
                imul    cx
                
                sub     bx,ax
                mov     ax,bx

                mov     cx,3
                sar     ax,cl

                mov     bx,xfrmptr
                add     bx,12                 ; (xfrm [p + 6])
                mov     Word Ptr es:[bx],ax   





                mov     ax,k24
                mov     cx,ax
                mov     dx,k25

                add     ax,dx
                sub     cx,dx

                mov     k34,ax
                mov     k35,cx


                mov     ax,k27
                mov     cx,ax
                mov     dx,k26

                add     ax,dx
                sub     cx,dx

                mov     k37,ax
                mov     k36,cx




                mov     ax,k37
                mov     cx,2
                sar     ax,cl

                mov     cx,c1p16
                imul    cx

                mov     bx,ax                 ; Save momentarily


                mov     ax,k34
                mov     cx,2
                sar     ax,cl

                mov     cx,s1p16
                imul    cx
                
                add     ax,bx

                mov     cx,3
                sar     ax,cl

                mov     bx,xfrmptr
                add     bx,2                  ; (xfrm [p + 1])
                mov     Word Ptr es:[bx],ax   





                mov     ax,k36
                mov     cx,2
                sar     ax,cl

                mov     cx,s3p16
                imul    cx

                mov     bx,ax                 ; Save momentarily


                mov     ax,k35
                mov     cx,2
                sar     ax,cl

                mov     cx,c3p16
                imul    cx
                
                add     ax,bx

                mov     cx,3
                sar     ax,cl

                mov     bx,xfrmptr
                add     bx,10                 ; (xfrm [p + 5])
                mov     Word Ptr es:[bx],ax   






                mov     ax,k36
                mov     cx,2 
                sar     ax,cl

                mov     cx,c3p16
                imul    cx

                mov     bx,ax                 ; Save momentarily


                mov     ax,k35
                mov     cx,2 
                sar     ax,cl

                mov     cx,s3p16
                imul    cx
                
                sub     bx,ax
                mov     ax,bx

                mov     cx,3                  
                sar     ax,cl

                mov     bx,xfrmptr
                add     bx,6                  ; (xfrm [p + 3])
                mov     Word Ptr es:[bx],ax   






                mov     ax,k37
                mov     cx,2
                sar     ax,cl

                mov     cx,s1p16
                imul    cx

                mov     bx,ax                 ; Save momentarily


                mov     ax,k34
                mov     cx,2
                sar     ax,cl

                mov     cx,c1p16      ; Fixed mistake in C code 5-2-90 (was c3p16)
                imul    cx
                
                sub     bx,ax
                mov     ax,bx

                mov     cx,3  
                sar     ax,cl

                mov     bx,xfrmptr
                add     bx,14                 ; (xfrm [p + 7])
                mov     Word Ptr es:[bx],ax   



                mov     xinptr,si

                mov     ax,wValsPerRow
                shl     ax,1
                add     xfrmptr,ax

                pop     cx
                dec     cx
                cmp     cx,0
                je      rows_done

                jmp     FAR PTR RowLoop



rows_done:      mov     cx,8                
                mov     xfrmptr,di
                mov     col_count,0



ColLoop:        push    cx
                 

                mov     dx,wValsPerRow
                shl     dx,1
                mov     bx,xfrmptr

                mov     ax,es:Word Ptr [bx]      ; xfrm [j]
                mov     k00,ax

                add     bx,dx
                mov     ax,es:Word Ptr [bx]      ; xfrm [j + (wValuesPerRow)]
                mov     k01,ax

                add     bx,dx
                mov     ax,es:Word Ptr [bx]      
                mov     k02,ax

                add     bx,dx
                mov     ax,es:Word Ptr [bx]      
                mov     k03,ax

                add     bx,dx
                mov     ax,es:Word Ptr [bx]      
                mov     k04,ax

                add     bx,dx
                mov     ax,es:Word Ptr [bx]      
                mov     k05,ax

                add     bx,dx
                mov     ax,es:Word Ptr [bx]      
                mov     k06,ax

                add     bx,dx
                mov     ax,es:Word Ptr [bx]     
                mov     k07,ax


;**



                mov     ax,k00
                mov     cx,ax
                mov     dx,k07

                add     ax,dx
                sar     ax,1

                sub     cx,dx
                sar     cx,1
    
                mov     k10,ax
                mov     k17,cx
                mov     k27,cx



                mov     ax,k01
                mov     cx,ax
                mov     dx,k06

                add     ax,dx
                sar     ax,1

                sub     cx,dx
                sar     cx,1
    
                mov     k11,ax
                mov     k16,cx


                mov     ax,k02
                mov     cx,ax
                mov     dx,k05

                add     ax,dx
                sar     ax,1

                sub     cx,dx
                sar     cx,1
    
                mov     k12,ax
                mov     k15,cx


                mov     ax,k03
                mov     cx,ax
                mov     dx,k04

                add     ax,dx
                sar     ax,1

                sub     cx,dx
                sar     cx,1
    
                mov     k13,ax
                mov     k14,cx


; While we're at it, set k24

                mov     k24,cx



                mov     ax,k10
                mov     cx,ax
                mov     dx,k13 

                add     ax,dx
                sub     cx,dx
    
                mov     k20,ax
                mov     k23,cx


                mov     ax,k11
                mov     cx,ax
                mov     dx,k12

                add     ax,dx
                sub     cx,dx
    
                mov     k21,ax
                mov     k22,cx



;  ** calc k4 values





                mov     ax,k16
                mov     cx,ax
                mov     dx,k15

                add     ax,dx
                sub     cx,dx

                push    cx                ; save difference for k25 calculation

                mov     cx,2              ; 2nd k25 shr 2 first, then 3
                sar     ax,cl

                mov     cx,c1p4
                imul    cx
                mov     cx,3
                sar     ax,cl
                mov     k26,ax

                pop     ax

                mov     cx,2              ; 2 1st then 3
                sar     ax,cl
                mov     cx,c1p4
                imul    cx
                mov     cx,3
                sar     ax,cl
                mov     k25,ax



                mov     ax,k20
                add     ax,k21


                mov     cx,4              ; Note (4)
                sar     ax,cl

                mov     cx,c1p4
                imul    cx

                sar     ax,1 

                mov     k4,ax   ; k4 [0] = a



                mov     ax,k20
                sub     ax,k21
                mov     cx,2
                sar     ax,cl

                mov     cx,c1p4
                imul    cx

                mov     cx,3
                sar     ax,cl

                mov     k4 + 8,ax   ; k4 [4] = a




                mov     ax,k23
                mov     cx,2
                sar     ax,cl

                mov     cx,c1p8
                imul    cx

                mov     bx,ax                 ; Save momentarily


                mov     ax,k22
                mov     cx,2
                sar     ax,cl

                mov     cx,s1p8
                imul    cx
                
                add     ax,bx

                mov     cx,3
                sar     ax,cl

                mov     k4 + 4,ax   ; k4 [2] = a







                mov     ax,k23
                mov     cx,2
                sar     ax,cl

                mov     cx,s1p8
                imul    cx

                mov     bx,ax                 ; Save momentarily


                mov     ax,k22
                mov     cx,2
                sar     ax,cl

                mov     cx,c1p8
                imul    cx
                
                sub     bx,ax
                mov     ax,bx

                mov     cx,3
                sar     ax,cl

                mov     k4 + 12,ax   ; k4 [6] = a





                mov     ax,k24
                mov     cx,ax
                mov     dx,k25

                add     ax,dx
                sub     cx,dx

                mov     k34,ax
                mov     k35,cx


                mov     ax,k27
                mov     cx,ax
                mov     dx,k26

                add     ax,dx
                sub     cx,dx

                mov     k37,ax
                mov     k36,cx




                mov     ax,k37
                mov     cx,3                  ; Note (5)
                sar     ax,cl

                mov     cx,c1p16
                imul    cx

                mov     bx,ax                 ; Save momentarily


                mov     ax,k34
                mov     cx,3
                sar     ax,cl

                mov     cx,s1p16
                imul    cx
                
                add     ax,bx

                mov     cx,2
                sar     ax,cl


                mov     k4 + 2,ax   ; k4 [1] = a




                mov     ax,k36
                mov     cx,2
                sar     ax,cl

                mov     cx,s3p16
                imul    cx

                mov     bx,ax                 ; Save momentarily


                mov     ax,k35
                mov     cx,2
                sar     ax,cl

                mov     cx,c3p16
                imul    cx
                
                add     ax,bx

                mov     cx,3
                sar     ax,cl


                mov     k4 + 10,ax   ; k4 [5] = a





                mov     ax,k36
                mov     cx,2
                sar     ax,cl

                mov     cx,c3p16
                imul    cx

                mov     bx,ax                 ; Save momentarily


                mov     ax,k35
                mov     cx,2
                sar     ax,cl

                mov     cx,s3p16
                imul    cx
                
                sub     bx,ax
                mov     ax,bx

                mov     cx,3                  
                sar     ax,cl

                mov     k4 + 6,ax   ; k4 [3] = a






                mov     ax,k37
                mov     cx,2
                sar     ax,cl

                mov     cx,s1p16
                imul    cx

                mov     bx,ax                 ; Save momentarily


                mov     ax,k34
                mov     cx,2
                sar     ax,cl

                mov     cx,c1p16
                imul    cx
                
                sub     bx,ax
                mov     ax,bx

                mov     cx,3                 
                sar     ax,cl

                mov     k4 + 14,ax            ; k4 [7] = a




                mov     ax,col_count
                shl     ax,1                  ; Word Ptr

                mov     pptr,di
                add     pptr,ax

                mov     bx,bp
                sub     bx,k4_offset          ; BX = k4 array on stack (relative to SS) 
                mov     cx,8

InnerLoop:      push    cx

                mov     ax,Word Ptr ss:[bx]

                cmp     ax,0
                jge     L1

                sub     ax,4

;;              mov     cx,3
;;              sar     ax,cl

                cwd     
                mov     cx,8
                idiv    cx


                cmp     ax,-1023
                jge     L2

                mov     ax,-1023
                jmp     short L2


L1:

                add     ax,4

;               mov     cx,3
;               sar     ax,cl

                cwd     
                mov     cx,8
                idiv    cx
                
                cmp     ax,1023
                jle     L2
                mov     ax,1023
                  

L2:             

; Store the finished DCT    

                push    bx

                mov     bx,pptr
                mov     Word Ptr es:[bx],ax
                mov     bx,wValsPerRow
                shl     bx,1
                add     pptr,bx

                pop     bx
                add     bx,2                  ; Next position in k4 array

                pop     cx
                loop    InnerLoop



                inc     col_count
                add     xfrmptr,2


                pop     cx
                dec     cx
                cmp     cx,0
                je      cols_done

                jmp     FAR PTR ColLoop



cols_done:



done:
                pop     es
                pop     ds
                pop     di
                pop     si
                add     sp,100
                pop     bp
                ret
 
_fdct           endp




                end   
