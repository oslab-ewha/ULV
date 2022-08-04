.global ulv_setjmp_clone
.global ulv_copy_stack
.global ulv_clone_host

.type ulv_setjmp_clone,@function
.type ulv_copy_stack,@function
.type ulv_clone_host,@function

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
	mov $4, %rcx
	call get_stack_depth

	sub %rax, %rdi

	mov %rdi, %r10
	mov %rsi, -8(%r10)

	mov %rbp, %rdx
	mov %rsp, %rsi
	add $8, %rsi

	mov %rbp, %r8		/* save rbp */
	sub %rsi, %r8
	add %rdi, %r8
	mov %r8, -16(%r10)

	cld

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

ulv_clone_host:
	mov $56, %rax
	mov %rdx, (%rsi)
	syscall
	ret
