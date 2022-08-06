.global ulv_setjmp_clone
.global ulv_clone_host

.type ulv_setjmp_clone,@function
.type ulv_clone_host,@function

ulv_setjmp_clone:
	mov %rbx,(%rdi)
	mov %r8,-8(%rsi)
	mov %r9,-16(%rsi)
	mov %rcx,-24(%rsi)
        mov %rsi,8(%rdi)	/* rbp */
	sub $24,%rsi
        mov %r12,16(%rdi)
        mov %r13,24(%rdi)
        mov %r14,32(%rdi)
        mov %r15,40(%rdi)
	mov %rsi,48(%rdi)
	mov %rdx,64(%rdi)
        mov (%rsp),%rdx         /* save return addr ptr for new rip */
        mov %rdx,56(%rdi)
        xor %rax,%rax           /* always return 0 */
        ret

ulv_clone_host:
	mov $56, %rax
	mov %rdx, (%rsi)
	syscall
	ret
