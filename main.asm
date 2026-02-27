	.file	"main.c"
	.text
	.globl	kernelEntry
	.section	.bss
	.align 4
	.type	kernelEntry, @object
	.size	kernelEntry, 4
kernelEntry:
	.zero	4
	.section	.rodata
.LC0:
	.string	"\n%x: "
.LC1:
	.string	"%x "
.LC2:
	.string	"\n"
	.text
	.globl	hexdump
	.type	hexdump, @function
hexdump:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	8(%ebp), %eax
	movl	%eax, -16(%ebp)
	movl	$0, -12(%ebp)
	jmp	.L2
.L4:
	movl	-12(%ebp), %eax
	andl	$15, %eax
	testl	%eax, %eax
	jne	.L3
	subl	$8, %esp
	pushl	-12(%ebp)
	pushl	$.LC0
	call	printf
	addl	$16, %esp
.L3:
	movl	-16(%ebp), %edx
	movl	-12(%ebp), %eax
	addl	%edx, %eax
	movzbl	(%eax), %eax
	movzbl	%al, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	$.LC1
	call	printf
	addl	$16, %esp
	addl	$1, -12(%ebp)
.L2:
	movl	12(%ebp), %eax
	cmpl	%eax, -12(%ebp)
	jb	.L4
	subl	$12, %esp
	pushl	$.LC2
	call	printf
	addl	$16, %esp
	nop
	leave
	ret
	.size	hexdump, .-hexdump
	.section	.rodata
.LC3:
	.string	"%x"
.LC4:
	.string	"Disk init error\r\n"
.LC5:
	.string	"FAT init error\r\n"
.LC6:
	.string	"BESOS Bootloader"
.LC7:
	.string	""
.LC8:
	.string	"Hello world"
.LC9:
	.string	"Press DEL for menu"
.LC10:
	.string	"pressed key\n"
.LC11:
	.string	"to kernel\n"
.LC12:
	.string	"/boot/kernel.elf"
	.align 4
.LC13:
	.string	"ELF read failed, booting halted!\n"
.LC14:
	.string	"Kernel not found\n"
.LC15:
	.string	"jump to 0x%p\n"
	.text
	.globl	start
	.type	start, @function
start:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$56, %esp
	movl	8(%ebp), %eax
	movw	%ax, -44(%ebp)
	call	fill_table
	movb	$0, -9(%ebp)
	subl	$8, %esp
	pushl	12(%ebp)
	pushl	$.LC3
	call	printf
	addl	$16, %esp
	subl	$8, %esp
	pushl	$64
	pushl	12(%ebp)
	call	hexdump
	addl	$16, %esp
	movl	$2097152, -16(%ebp)
	movzwl	-44(%ebp), %eax
	movzbl	%al, %eax
	subl	$8, %esp
	pushl	%eax
	leal	-24(%ebp), %eax
	pushl	%eax
	call	DISK_Initialize
	addl	$16, %esp
	xorl	$1, %eax
	testb	%al, %al
	je	.L6
	subl	$12, %esp
	pushl	$.LC4
	call	printf
	addl	$16, %esp
	jmp	.L7
.L6:
	subl	$4, %esp
	pushl	12(%ebp)
	leal	-24(%ebp), %eax
	pushl	%eax
	leal	-36(%ebp), %eax
	pushl	%eax
	call	MBR_DetectPartition
	addl	$16, %esp
	subl	$12, %esp
	leal	-36(%ebp), %eax
	pushl	%eax
	call	FAT_Initialize
	addl	$16, %esp
	xorl	$1, %eax
	testb	%al, %al
	je	.L8
	subl	$12, %esp
	pushl	$.LC5
	call	printf
	addl	$16, %esp
	jmp	.L7
.L8:
	movzwl	-44(%ebp), %eax
	movl	%eax, %edx
	movl	-16(%ebp), %eax
	movb	%dl, (%eax)
	movl	-16(%ebp), %eax
	movl	$0, 2629(%eax)
	movl	-16(%ebp), %eax
	addl	$2597, %eax
	subl	$8, %esp
	pushl	$.LC6
	pushl	%eax
	call	strcpy
	addl	$16, %esp
	movl	-16(%ebp), %eax
	addl	$2469, %eax
	subl	$8, %esp
	pushl	$.LC7
	pushl	%eax
	call	strcpy
	addl	$16, %esp
	subl	$12, %esp
	pushl	-16(%ebp)
	call	DetectMemory
	addl	$16, %esp
	subl	$12, %esp
	pushl	-16(%ebp)
	call	DetectVESA
	addl	$16, %esp
	subl	$12, %esp
	pushl	-16(%ebp)
	call	DetectPCI
	addl	$16, %esp
	subl	$12, %esp
	pushl	-16(%ebp)
	call	DetectEquipment
	addl	$16, %esp
	subl	$12, %esp
	pushl	$.LC8
	call	printf
	addl	$16, %esp
	call	Clear
	subl	$8, %esp
	pushl	$23
	pushl	$31
	call	SetCursor
	addl	$16, %esp
	subl	$12, %esp
	pushl	$.LC9
	call	printf
	addl	$16, %esp
	subl	$8, %esp
	pushl	$0
	pushl	$0
	call	SetCursor
	addl	$16, %esp
	movb	$83, menu_key
	call	X86_checkForKeys
	testb	%al, %al
	je	.L9
	subl	$12, %esp
	pushl	$.LC10
	call	printf
	addl	$16, %esp
	subl	$12, %esp
	pushl	-16(%ebp)
	call	menuEntry
	addl	$16, %esp
.L9:
	subl	$12, %esp
	pushl	$.LC11
	call	printf
	addl	$16, %esp
	subl	$4, %esp
	pushl	$kernelEntry
	pushl	$.LC12
	leal	-36(%ebp), %eax
	pushl	%eax
	call	ELF_Read
	addl	$16, %esp
	xorl	$1, %eax
	testb	%al, %al
	je	.L10
	subl	$12, %esp
	pushl	$.LC13
	call	printf
	addl	$16, %esp
	subl	$12, %esp
	pushl	$.LC14
	call	printf
	addl	$16, %esp
	jmp	.L7
.L10:
	movb	$1, -9(%ebp)
	cmpb	$0, -9(%ebp)
	je	.L12
	movl	kernelEntry, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	$.LC15
	call	printf
	addl	$16, %esp
/APP
/  118 "./src/Bootloader/stage2/main.c" 1
	cli
/  0 "" 2
/NO_APP
	movl	$pageDirectory, %eax
/APP
/  119 "./src/Bootloader/stage2/main.c" 1
	mov %eax, %cr3
/  0 "" 2
/  120 "./src/Bootloader/stage2/main.c" 1
	mov %cr0, %eax
/  0 "" 2
/  121 "./src/Bootloader/stage2/main.c" 1
	orl $0x80000000, %eax
/  0 "" 2
/  122 "./src/Bootloader/stage2/main.c" 1
	mov %eax, %cr0
/  0 "" 2
/  123 "./src/Bootloader/stage2/main.c" 1
	sti
/  0 "" 2
/NO_APP
	movl	-16(%ebp), %eax
/APP
/  124 "./src/Bootloader/stage2/main.c" 1
	movl %eax, %edi
/  0 "" 2
/NO_APP
	movl	kernelEntry, %eax
/APP
/  125 "./src/Bootloader/stage2/main.c" 1
	movl %eax, %eax
/  0 "" 2
/  126 "./src/Bootloader/stage2/main.c" 1
	jmp *%eax
/  0 "" 2
/NO_APP
	jmp	.L7
.L12:
	nop
.L7:
.L11:
	jmp	.L11
	.size	start, .-start
	.ident	"GCC: (GNU) 11.2.0"
