.text
.global ulv_syscall
.type   ulv_syscall,@function
.global __clone
.hidden __clone
.type   __clone,@function
__clone:
	push %rbp
	mov %rsp, %rbp

	sub $40, %rsp		/* for ulv_syscall args */
	mov %rdx,-40(%rbp)
	mov %r8,-24(%rbp)
	mov %r9,-16(%rbp)
	mov 48(%rsp),%rdx
	mov %rdx,-8(%rbp)

	and $-16,%rsi
	mov %rsi,-32(%rbp)

	lea -40(%rbp),%rsi
	push %r12
	push %r13
	mov %rcx, %r13
	mov %rdi,%r12
	mov $56,%rdi

	call ulv_syscall

	test %eax,%eax
	jnz 1f
	xor %ebp,%ebp
	mov %r13, %rdi
	call *%r12
	mov %eax,%edi
	xor %eax,%eax
	mov $60,%al
	syscall
	hlt
1:
	pop %r13
	pop %r12
	add $40, %rsp
	pop %rbp
	ret
