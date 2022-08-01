.global ulv_longjmp
.global ulv_setjmp
.global ulv_setjmp_clone
.global ulv_copy_stack

.type ulv_longjmp,@function
.type ulv_setjmp,@function
.type ulv_setjmp_clone,@function
.type ulv_copy_stack,@function

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
        xor %rax,%rax           /* always return 0 */
        ret

ulv_setjmp_clone:
	mov %rbx,(%rdi)
	mov (%rsi),%r10
	add $8,%rsi
        mov %r10,8(%rdi)
        mov %r12,16(%rdi)
        mov %r13,24(%rdi)
        mov %r14,32(%rdi)
        mov %r15,40(%rdi)
	mov %rsi,48(%rdi)
        mov (%rsp),%rdx         /* save return addr ptr for new rip */
        mov %rdx,56(%rdi)
        xor %rax,%rax           /* always return 0 */
        ret

ulv_copy_stack:
	mov $6, %rcx
	call get_stack_depth

	sub %rax, %rdi

	mov %rdi, %r10
	mov %rsi, -8(%r10)

	mov %rbp, %rdx
	mov %rsp, %rsi
	add $8, %rsi

	mov %rbp, %r8
	sub %rsi, %r8
	add %rdi, %r8
	mov %r8, -16(%r10)

	cld

	call copy_stack_frame
	call copy_stack_frame
	call copy_stack_frame
	call copy_stack_frame
	call copy_stack_frame
	call copy_stack_frame

	sub $16, %r10
	mov %r10, %rax
	ret

copy_stack_frame:
	mov %rdx, %rax
	sub %rsi, %rax
	mov %rax, %rcx
	rep movsb
	mov (%rsi), %rdx
	mov %rdx, %rcx
	sub %rsi, %rcx
	mov %rdi, %r11
	add %rcx, %r11
	mov %r11, (%rdi)
	mov 8(%rsi), %r11
	mov %r11, 8(%rdi)
	add $16, %rsi
	add $16, %rdi
	ret

get_stack_depth:
	mov %rbp, %rax
again:
	mov (%rax), %rax
	dec %rcx
	cmp $0, %rcx
	jz done
	jmp again
done:	
	sub %rsp, %rax
	sub $8, %rax
	ret
