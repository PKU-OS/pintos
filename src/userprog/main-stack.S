## Code for setting up the main stack.

	.globl setup_main_stack_asm
	.func setup_main_stack_asm
setup_main_stack_asm:
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx
	pushl %esi
	pushl %edi

	# command line
	movl 8(%ebp), %ecx
	# output address
	movl 12(%ebp), %eax


	decl %ecx
1:	incl %ecx
	movb (%ecx), %bl
	cmp $0, %bl
	jne 1b


	movl $0, %edi
	movl $0, %esi
1:	cmp %ecx, 8(%ebp)
	je 9f
	decl %ecx
	movb (%ecx), %bl
	cmpb $32, %bl
	je 1b

	incl %ecx
	incl %esi
	incl %edi

	decl %eax
	movb $0, (%eax)

2:	cmp %ecx, 8(%ebp)
	je 9f
	decl %ecx
	movb (%ecx), %bl

	cmpb $32, %bl
	je 1b

	decl %eax
	movb %bl, (%eax)
	jmp 2b

9:	movl %eax, %ecx
	andl $0xFFFFFFFC, %eax


	leal 1(%esi), %edi
	shll $2, %edi
	subl %edi, %eax

	movl $0, (%eax, %esi, 4)

	movl $0, %edi
1:	cmpl 12(%ebp), %ecx
	je 9f

	movl %ecx, (%eax, %edi, 4)
	incl %edi

2:	movb (%ecx), %bl
	incl %ecx
	cmpb $0, %bl
	je 1b
	jmp 2b

9:	movl %eax, %ecx


	subl $12, %eax
	movl %esi, 4(%eax)
	movl %ecx, 8(%eax)

	popl %edi
	popl %esi
	popl %ebx
	popl %ebp
	ret

	.endfunc
