%include "asmdefs.inc"
%include "ia_32e.inc"

%if (__ARCH32E >= __ARCH32E_M7)

segment .text align=ARCH_ALIGN_FACTOR

;
; carry, r[:size] = a[:size] + w
; uint64_t mpn_inc_vectorized(uint64_t *r, const uint64_t *a, unsigned int size, uint64_t w)
;
align ARCH_ALIGN_FACTOR
IPPASM mpn_inc_vectorized,PUBLIC
%assign LOCAL_FRAME 0
        USES_GPR rsi,rdi
        USES_XMM
        COMP_ABI 4

; rdi = r
; rsi = a
; rdx = size
; rcx = w

   movsxd   rdx, edx    ; length

   mov      r8, qword [rsi]     ; r[0] = r[0]+increment
   add      r8, rcx
   mov      qword [rdi], r8

   lea      rsi, [rsi+rdx*sizeof(qword)]
   lea      rdi, [rdi+rdx*sizeof(qword)]
   lea      rcx, [rdx*sizeof(qword)]

   sbb      rax, rax                ; save cf
   neg      rcx                     ; rcx = negative length (bytes)
   add      rcx, sizeof(qword)
   jrcxz    .exit
   add      rax, rax                ; restore cf
   jnc      .copy

align ARCH_ALIGN_FACTOR
.inc_loop:
   mov      r8, qword [rsi+rcx]
   adc      r8, 0
   mov      qword [rdi+rcx], r8
   lea      rcx, [rcx+sizeof(qword)]
   jrcxz    .exit_loop
   jnc      .exit_loop
   jmp      .inc_loop
.exit_loop:
   sbb      rax, rax                ; save cf

.copy:
   cmp      rsi, rdi
   jz       .exit
   jrcxz    .exit
.copy_loop:
   mov      r8, qword [rsi+rcx]
   mov      qword [rdi+rcx], r8
   add      rcx, sizeof(qword)
   jnz      .copy_loop

.exit:
   neg      rax
   REST_XMM
   REST_GPR
   ret
ENDFUNC mpn_inc_vectorized


;
; borrow, r[:size] = a[:size] - w
; uint64_t mpn_dec_vectorized(uint64_t *r, const uint64_t *a, unsigned int size, uint64_t w)
;

align ARCH_ALIGN_FACTOR
IPPASM mpn_dec_vectorized,PUBLIC
%assign LOCAL_FRAME 0
        USES_GPR rsi,rdi
        USES_XMM
        COMP_ABI 4

; rdi = r
; rsi = a
; rdx = size
; rcx = w

   movsxd   rdx, edx    ; length

   mov      r8, qword [rsi]     ; r[0] = r[0]+increment
   sub      r8, rcx
   mov      qword [rdi], r8

   lea      rsi, [rsi+rdx*sizeof(qword)]
   lea      rdi, [rdi+rdx*sizeof(qword)]
   lea      rcx, [rdx*sizeof(qword)]

   sbb      rax, rax                ; save cf
   neg      rcx                     ; rcx = negative length (bytes)
   add      rcx, sizeof(qword)
   jrcxz    .exit
   add      rax, rax                ; restore cf
   jnc      .copy

align ARCH_ALIGN_FACTOR
.inc_loop:
   mov      r8, qword [rsi+rcx]
   sbb      r8, 0
   mov      qword [rdi+rcx], r8
   lea      rcx, [rcx+sizeof(qword)]
   jrcxz    .exit_loop
   jnc      .exit_loop
   jmp      .inc_loop
.exit_loop:
   sbb      rax, rax                ; save cf

.copy:
   cmp      rsi, rdi
   jz       .exit
   jrcxz    .exit
.copy_loop:
   mov      r8, qword [rsi+rcx]
   mov      qword [rdi+rcx], r8
   add      rcx, sizeof(qword)
   jnz      .copy_loop

.exit:
   neg      rax
   REST_XMM
   REST_GPR
   ret
ENDFUNC mpn_dec_vectorized

%endif
