.global ulv_longjmp
.global ulv_setjmp

.type ulv_longjmp,@function
.type ulv_setjmp,@function

ulv_longjmp:
        mov %rsi,%rax           /* val will be longjmp return */
        test %rax,%rax
        jnz 1f
        inc %rax                /* if val==0, val=1 per longjmp semantics */
1:
        mov (%rdi),%rbx         /* rdi is the jmp_buf, restore regs from it */
        mov 8(%rdi),%rbp
        mov 16(%rdi),%r12
        mov 24(%rdi),%r13
        mov 32(%rdi),%r14
        mov 40(%rdi),%r15
        mov 48(%rdi),%rdx       /* this ends up being the stack pointer */
        mov %rdx,%rsp
        mov 56(%rdi),%rdx       /* this is the instruction pointer */
	mov 64(%rdi),%r11
	wrfsbase %r11
        jmp *%rdx               /* goto saved address without altering rsp */

ulv_setjmp:
        mov %rbx,(%rdi)         /* rdi is jmp_buf, move registers onto it */
        mov %rbp,8(%rdi)
        mov %r12,16(%rdi)
        mov %r13,24(%rdi)
        mov %r14,32(%rdi)
        mov %r15,40(%rdi)
        lea 8(%rsp),%rdx        /* this is our rsp WITHOUT current ret addr */
        mov %rdx,48(%rdi)
        mov (%rsp),%rdx         /* save return addr ptr for new rip */
        mov %rdx,56(%rdi)
	rdfsbase %r11
        mov %r11,64(%rdi)
        xor %rax,%rax           /* always return 0 */
        ret
