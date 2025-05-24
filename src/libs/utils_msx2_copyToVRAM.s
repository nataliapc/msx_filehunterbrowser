; void msx2_copyToVRAM(uint16_t source, uint32_t vram, uint16_t size) __sdcccall(0);

	.globl setVDP_Write

_msx2_copyToVRAM::
		push ix
		ld   ix,#4
		add  ix,sp

		ld   l,0(ix)
		ld   h,1(ix)
		ld   e,2(ix)
		ld   d,3(ix)
		ld   a,4(ix)
		ld   c,6(ix)
		ld   b,7(ix)
		pop  ix

msx2_copyToVRAM::			; source:HL vram:A+DE size:BC
		push bc
		call setVDP_Write	; A+DE
		pop  bc

		ld   d, b			; Num of blocks of 256 bytes
		ld   b, c			; Rest
		ld   c, #0x98

		xor	 a				; There is rest?
		cp	 b
		jr	 z, .c2v_loop1	; ...if no, skip the rest bytes send

	.c2v_loop0:
		otir				; Send rest bytes
		cp   d				; There is blocks of 256 bytes?
		ret  z				; ...return if not

	.c2v_loop1:
		otir				; Send block of 256 bytes
		dec  d				; Decrement blocks counter
		jr   nz, .c2v_loop1	; ...more?
		ret
