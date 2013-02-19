            DOSSEG
            .MODEL MEDIUM 
 

            .CODE  

            public  _process_scan
            public  _count_unique



;
;   process_scan (char far *, char far *, char far *, int, unsigned, 
;                 int *, int *, int *, int *, int *, int *);
;


curr_rmin      equ     Word Ptr [bp - 6] 
curr_rmax      equ     Word Ptr [bp - 8] 
curr_gmin      equ     Word Ptr [bp - 10] 
curr_gmax      equ     Word Ptr [bp - 12] 
curr_bmin      equ     Word Ptr [bp - 14] 
curr_bmax      equ     Word Ptr [bp - 16] 
curr_ds        equ     Word Ptr [bp - 18] 


_process_scan proc    far

        push    bp
        mov     bp,sp
               sub     sp,24                   ; 24 bytes local space

               push    ds
               push    es
               push    si
               push    di


               mov     ax,ds
               mov     curr_ds,ax              ; Save this ds

; Setup min/max for each color

; Red
               mov     bx,Word Ptr [bp + 22]    ; &rmin
               mov     ax,Word Ptr [bx]
               mov     curr_rmin,ax             ; Save current min

               mov     bx,Word Ptr [bp + 24]    ; &rmax
               mov     ax,Word Ptr [bx]
               mov     curr_rmax,ax             ; Save current max


; Green 
               mov     bx,Word Ptr [bp + 26]    ; &gmin
               mov     ax,Word Ptr [bx]
               mov     curr_gmin,ax             ; Save current min

               mov     bx,Word Ptr [bp + 28]    ; &gmax
               mov     ax,Word Ptr [bx]
               mov     curr_gmax,ax             ; Save current max

; Blue
               mov     bx,Word Ptr [bp + 30]    ; &bmin
               mov     ax,Word Ptr [bx]
               mov     curr_bmin,ax             ; Save current min

               mov     bx,Word Ptr [bp + 32]    ; &bmax
               mov     ax,Word Ptr [bx]
               mov     curr_bmax,ax             ; Save current max



; Setup far buffers

               lds     si,DWord Ptr [bp + 6]
               les     di,DWord Ptr [bp + 14]


               cld

               xor     bx,bx                    ; Clear counter

process_loop:  push    bx

               xor     bx,bx

; First - get index into histogram (and compare min/max stuff). . .
; Do red * 1024 

               xor     ax,ax
               mov     cx,Word Ptr [bp + 18]    ; Bits to shift (right)
               shr     Byte Ptr [si],cl
               lodsb


red_test:      cmp     ax,curr_rmin
               jl      set_rmin
               cmp     ax,curr_rmax
               jg      set_rmax
       
red_ok:        mov     cx,10
               shl     ax,cl
               add     bx,ax

; Do green * 32 

               xor     ax,ax
               mov     cx,Word Ptr [bp + 18]    ; Bits to shift (right)
               shr     Byte Ptr [si],cl
               lodsb

grn_test:      cmp     ax,curr_gmin
               jl      set_gmin
               cmp     ax,curr_gmax
               jg      set_gmax

grn_ok:        mov     cx,5  
               shl     ax,cl
               add     bx,ax


; Add blue
               xor     ax,ax
               mov     cx,Word Ptr [bp + 18]    ; Bits to shift (right)
               shr     Byte Ptr [si],cl
               lodsb

blu_test:      cmp     ax,curr_bmin
               jl      set_bmin
               cmp     ax,curr_bmax
               jg      set_bmax

blu_ok:        add     bx,ax

; BX now points to correct histogram index (0..32k)

               shl     bx,1                            ; Make 0..64k 
               cmp     Word Ptr es:[di + bx],0FFFFh    ; Already = 64k ?
               jne     no_byte_inc

; Otherwise need to increment byte portion of histogram

               push    es
               push    di


               les     di,DWord Ptr [bp + 10]

               shr     bx,1                     ; Make 0..32k
               inc     Byte Ptr es:[di + bx]    
               shl     bx,1                     ; Make 0..64k again
                  
               pop     di
               pop     es

               mov     Word Ptr es:[di + bx],0   ; Zero word portion
               jmp     short word_zero           ; Do not increment word portion

no_byte_inc:   inc     Word Ptr es:[di + bx]     ; Increment word portion
word_zero:     pop     bx                        ; Restore counter
               add     bx,3
               cmp     bx,Word Ptr [bp + 20]
               jb      process_loop
               jmp     short do_minmax

; These are for setting min / max values for r, g & b.  (Within short distance)

set_rmin:      mov     curr_rmin,ax
               jmp     red_ok
set_rmax:      mov     curr_rmax,ax
               jmp     red_ok

set_gmin:      mov     curr_gmin,ax
               jmp     grn_ok
set_gmax:      mov     curr_gmax,ax
               jmp     grn_ok

set_bmin:      mov     curr_bmin,ax
               jmp     blu_ok
set_bmax:      mov     curr_bmax,ax
               jmp     blu_ok


; Place min/max values in addresses

do_minmax:     mov     ax,curr_ds
               mov     ds,ax

; Red          xor     ax,ax

               mov     ax,curr_rmin
               mov     bx,Word Ptr [bp + 22]    ; &rmin
               mov     Word Ptr [bx],ax         ; Save current min

               mov     ax,curr_rmax
               mov     bx,Word Ptr [bp + 24]    ; &rmin
               mov     Word Ptr [bx],ax         ; Save current max

; Green
               mov     ax,curr_gmin
               mov     bx,Word Ptr [bp + 26]    ; &gmin
               mov     Word Ptr [bx],ax         ; Save current min

               mov     ax,curr_gmax
               mov     bx,Word Ptr [bp + 28]    ; &gmax
               mov     Word Ptr [bx],ax         ; Save current max

; Blue
               mov     ax,curr_bmin
               mov     bx,Word Ptr [bp + 30]    ; &bmin
               mov     Word Ptr [bx],ax         ; Save current min

               mov     ax,curr_bmax
               mov     bx,Word Ptr [bp + 32]    ; &bmax
               mov     Word Ptr [bx],ax         ; Save current max


               pop     di
               pop     si
               pop     es
               pop     ds

               add     sp,24                     ; Restore 24 bytes local space
        pop     bp
        ret

_process_scan endp


;
;        count_unique (char far *, char far *);
;
;


_count_unique  proc    far

               push    bp
               mov     bp,sp
               push    ds
               push    es
               push    si
               push    di

               lds     si,DWord Ptr [bp + 6]
               les     di,DWord Ptr [bp + 10]

               xor     ax,ax                   ; Clear counter
               mov     cx,32768                ; Do 32k times
cnt_loop:      cmp     Word Ptr es:[di],0      ; If not 0, is a color
               jne     add_count 
               cmp     Byte Ptr [si],0         ; If 0, not a color
               je      next_hist
add_count:     inc     ax


next_hist:     inc     di
               inc     di
               inc     si

               loop    cnt_loop 

; (AX contains return value)

               pop     di
               pop     si
               pop     es
               pop     ds
        pop     bp
               ret

_count_unique  endp


;
               end
