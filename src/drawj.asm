	section .text use32
	CPU 386
	bits 32

	;%define SCR_UNCHAINED

	SCR_W equ 320
	SCR_H equ 200
	SCR_BPP equ 8

	SCR_BITS equ 8

	%ifdef SCR_UNCHAINED
		SCR_BYTE_LENGTH equ (((SCR_W / 4) * SCR_BPP) / 8)
	%else
		SCR_BYTE_LENGTH equ ((SCR_W * SCR_BPP) / 8)
	%endif


	;EAX, EDX, EBX, ECX
	;EBP+8 12   16   20
	;ScreenPoint *p0, ScreenPoint *p1, uint8 color, uint8 *vram

	global _drawRectangleAsm

_drawRectangleAsm:
	push ebp
	mov ebp, esp
	push ebx
	push esi
	push edi

		mov ebx,[ebp+16]	; color
		mov edi,[ebp+20]	; vram

		mov edx,[ebp+12]
		mov ecx,[edx]		; x1
		sar ecx,SCR_BITS
		mov esi,[edx+4]		; y1
		sar esi,SCR_BITS
		mov eax,[ebp+8]
		mov edx,[eax]		; x0
		sar edx,SCR_BITS
		mov ebp,[eax+4]		; y0
		sar ebp,SCR_BITS

		mov bh,bl
		mov eax,ebx
		shl eax,16
		or eax,ebx		; uint32 color32 = (color << 24) | (color << 16) | (color << 8) | color;

		; if (y0 < 0) y0 = 0;
		test ebp,ebp
		jns noNegY0
			test esi,esi	; also check in that case if y1 <= 0
			jle aman
			xor ebp,ebp
		noNegY0:

		; if (y1 > SCR_H - 1) y1 = SCR_H - 1;
		cmp esi,SCR_H
		jc noClampY1
			mov esi,SCR_H-1
		noClampY1:

		; if (x0 < 0) x0 = 0;
		test edx,edx
		jns noNegX0
			test ecx,ecx	; also check in that case if x1 <= 0
			jle aman
			xor edx,edx
		noNegX0:

		; if (x1 > SCR_W - 1) x1 = SCR_W - 1;
		cmp ecx,SCR_W
		jc noClampX1
			mov ecx,SCR_W-1
		noClampX1:

		; edi = vram
		; eax = color
		; edx,ebp = x0,y0
		; ecx,esi = x1,y1

		%ifdef SCR_UNCHAINED
			shr edx,2
			shr ecx,2
		%endif

		sub esi,ebp			; store countY in ESI
		jng aman

		mov ebx,ebp
		%ifdef SCR_UNCHAINED
			; y * 80 = y * 64 + y * 16
			shl ebp,6
			shl ebx,4
		%else
			; y * 320 = y * 256 + y * 64
			shl ebp,8
			shl ebx,6
		%endif
		add ebp,ebx
		add edi,ebp		; uint8 *dstY = vram + VRAM_PIXEL_OFFSET(0,yScanlineMin);
		add edi,edx		; vram += x0

		sub ecx,edx		; ebx = x1 - x0
		jng aman

		; edi = vram + y*320+x
		; eax = color
		; edx = x0
		; ecx = lengthX
		; ebp = y0
		; esi = countY

		mov ebp,SCR_BYTE_LENGTH
		sub ebp,ecx
		mov ebx,ecx ; ebx for store lengthX

		cmp cx,5
		jnc longerRectRenderPath

		scanlineLoopY0:
			rep stosb

			add edi,ebp
			mov ecx,ebx	; get back lengthX

			dec esi
		jnz scanlineLoopY0
		jmp aman

longerRectRenderPath:

		and dl,3
		mov bh,4
		sub bh,dl

		movzx edx,bh
		sub ecx,edx
		mov bl,cl
		and bl,3

		shr ecx,2
		mov edx,ecx

		scanlineLoopY:
				mov cl,bh
				rep stosb

				mov ecx,edx
				rep stosd

				mov cl,bl
				rep stosb

				add edi,ebp
				dec esi
		jnz scanlineLoopY

aman:

	pop edi
    pop esi
    pop ebx
    pop ebp

	ret
