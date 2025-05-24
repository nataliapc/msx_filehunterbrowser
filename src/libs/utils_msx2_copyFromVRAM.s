; void msx2_copyFromVRAM(uint32_t vram, uint16_t source, uint16_t size) __sdcccall(0);

	.globl setVDP_Read

_msx2_copyFromVRAM::
		push ix
		ld   ix,#4
		add  ix,sp

		ld   e,0(ix)		; A+DE = VRAM address
		ld   d,1(ix)
		ld   a,2(ix)
		ld   l,4(ix)		; HL = source RAM address
		ld   h,5(ix)
		ld   c,6(ix)		; BC = size
		ld   b,7(ix)
		pop  ix

msx2_copyFromVRAM::			; source:HL vram:A+DE size:BC
		push bc
		call setVDP_Read	; Read from A+DE
		pop  bc

		ld   d, b			; Num of blocks of 256 bytes
		ld   b, c			; Rest
		ld   c, #0x98

		xor	 a				; There is rest?
		cp	 b
		jr	 z, .c2v_loop1	; ...if no, skip the rest bytes to read

	.c2v_loop0:
		inir				; Read rest bytes
		cp   d				; There is blocks of 256 bytes?
		ret  z				; ...return if not

	.c2v_loop1:
		inir				; Read block of 256 bytes
		dec  d				; Decrement blocks counter
		jr   nz, .c2v_loop1	; ...more?
		ret
