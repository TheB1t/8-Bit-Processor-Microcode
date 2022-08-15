#include "opcodes.h"
#include <iostream>
#include <vector>

#define MEM_SIZE		256 * 2

BYTE	PROGRAM[MEM_SIZE] = {0};

#define PUSH(A, D)	PROGRAM[A++] = D
#define PUT_LABEL(A, D)	BYTE LABEL_##D = A;
#define GET_LABEL(D)	LABEL_##D

void F_FETCH(WORD &a) {
	printf("[0x%04X][FETCH] Manual fetch next instruction\n", a);
	PUSH(a, I_FETCH);
}

void F_MOV(WORD &a, WORD src, WORD dst) {
	WORD dir = ((src | dst) >> 4) & (DIR_Msk | SEG_Msk);
	printf("[0x%04X][MOV] (%s)0x%02X -> (%s)0x%02X\n", a, DIR_SRC_GET(dir) == DIR_MEM ? (dir & SEG_DATA ? "MEM" : "VAL") : "REG", src & 0xFF, DIR_DST_GET(dir) == DIR_MEM ? "MEM" : "REG", dst & 0xFF);
	PUSH(a, I_MOV | dir);

	if (DIR_SRC_GET(dir) != DIR_DST_GET(dir)) {
		BYTE srcEIR = (DIR_SRC_GET(dir) == DIR_MEM ? SRC(MEMORY) : SRC(src & 0xFF));
		BYTE dstEIR = (DIR_DST_GET(dir) == DIR_MEM ? DST(MEMORY) : DST(dst & 0xFF));
		PUSH(a, srcEIR | dstEIR | (dir & SEG_Msk) >> 4);
	}
	
	if (DIR_SRC_GET(dir) == DIR_MEM)
		PUSH(a, src & 0xFF);
		
	if (DIR_DST_GET(dir) == DIR_MEM)
		PUSH(a, dst & 0xFF);
}

void F_ADD(WORD &a, WORD data) {
	WORD dir = (data >> 4) & (DIR_Msk | SEG_Msk);
	printf("[0x%04X][ADD] AREG + (%s)0x%02X\n", a, DIR_SRC_GET(dir) == DIR_MEM ? (dir & SEG_DATA ? "MEM" : "VAL") : "REG",data & 0xFF);
	PUSH(a, I_ADD | dir);
	PUSH(a, data);
}

void F_SUB(WORD &a, WORD data) {
	WORD dir = (data >> 4) & (DIR_Msk | SEG_Msk);
	printf("[0x%04X][SUB] AREG - (%s)0x%02X\n", a, DIR_SRC_GET(dir) == DIR_MEM ? (dir & SEG_DATA ? "MEM" : "VAL") : "REG",data & 0xFF);
	PUSH(a, I_SUB | dir);
	PUSH(a, data);
}

void F_JMP(WORD &a, BYTE addr) {
	printf("[0x%04X][JMP] Jump to 0x%02X\n", a, addr);
	PUSH(a, I_JMP | JMP(JMP_NO_COND));
	PUSH(a, addr);
}

void F_JC(WORD &a, BYTE addr) {
	printf("[0x%04X][JC] Jump to 0x%02X if carry flag is set\n", a, addr);
	PUSH(a, I_JMP | JMP(JMP_CARRY));
	PUSH(a, addr);	
}

void F_JZ(WORD &a, BYTE addr) {
	printf("[0x%04X][JZ] Jump to 0x%02X if zero flag is set\n", a, addr);
	PUSH(a, I_JMP | JMP(JMP_ZERO));
	PUSH(a, addr);	
}

void F_JN(WORD &a, BYTE addr) {
	printf("[0x%04X][JN] Jump to 0x%02X if negative flag is set\n", a, addr);
	PUSH(a, I_JMP | JMP(JMP_NEG));
	PUSH(a, addr);	
}

void F_JO(WORD &a, BYTE addr) {
	printf("[0x%04X][JO] Jump to 0x%02X if overflow flag is set\n", a, addr);
	PUSH(a, I_JMP | JMP(JMP_OVER));
	PUSH(a, addr);	
}

void F_JNC(WORD &a, BYTE addr) {
	printf("[0x%04X][JNC] Jump to 0x%02X if carry flag is clear\n", a, addr);
	PUSH(a, I_JMP | JMP(JMP_NO_CARRY));
	PUSH(a, addr);	
}

void F_JNZ(WORD &a, BYTE addr) {
	printf("[0x%04X][JNZ] Jump to 0x%02X if zero flag is clear\n", a, addr);
	PUSH(a, I_JMP | JMP(JMP_NO_ZERO));
	PUSH(a, addr);	
}

void F_JNN(WORD &a, BYTE addr) {
	printf("[0x%04X][JNN] Jump to 0x%02X if carry flag is clear\n", a, addr);
	PUSH(a, I_JMP | JMP(JMP_NO_NEG));
	PUSH(a, addr);	
}

void F_JNO(WORD &a, BYTE addr) {
	printf("[0x%04X][JNO] Jump to 0x%02X if zero flag is clear\n", a, addr);
	PUSH(a, I_JMP | JMP(JMP_NO_OVER));
	PUSH(a, addr);	
}

void F_CMP(WORD &a, BYTE data) {
	printf("[0x%04X][CMP] Compare 0x%02X with AREG\n", a, data);	
	PUSH(a, I_CMP);
	PUSH(a, data);
}

void F_PUSH(WORD &a, WORD data) {
	WORD dir = (data >> 4) & (DIR_Msk | SEG_Msk);
	printf("[0x%04X][PUSH] Push (%s)0x%02X to stack\n", a, DIR_SRC_GET(dir) == DIR_MEM ? (dir & SEG_DATA ? "MEM" : "VAL") : "REG", data & 0xFF);	
	PUSH(a, I_PUSH | dir);
		
	if (DIR_SRC_GET(dir) == DIR_REG)
		PUSH(a, SRC(data & 0xFF) | DST(TEMP_REG));
	else
		PUSH(a, data);
}

void F_POP(WORD &a, WORD data) {
	WORD dir = (data >> 4) & (DIR_Msk | SEG_Msk);
	printf("[0x%04X][POP] Pop from stack to (%s)0x%02X\n", a, DIR_DST_GET(dir) == DIR_MEM ? "MEM" : "REG", data & 0xFF);	
	PUSH(a, I_POP | dir);
		
	if (DIR_DST_GET(dir) == DIR_REG)
		PUSH(a, SRC(TEMP_REG) | DST(data & 0xFF));
	else
		PUSH(a, data);
}

void F_CALL(WORD &a, WORD addr) {
	printf("[0x%04X][CALL] Call function from address 0x%04X\n", a, addr);
	PUSH(a, I_CALL);
	PUSH(a, (addr >> 8) & 0xFF);
	PUSH(a, (addr >> 0) & 0xFF);
}

void F_RET(WORD &a) {
	printf("[0x%04X][RET] Return from function\n", a);
	PUSH(a, I_RET);
}

void F_LJMP(WORD &a, WORD addr) {
	printf("[0x%04X][LJMP] Long jump to 0x%04X\n", a, addr);
	PUSH(a, I_LJMP);
	PUSH(a, (addr >> 8) & 0xFF);
	PUSH(a, (addr >> 0) & 0xFF);
}

void F_OUTB(WORD &a, WORD addr, BYTE data) {
	printf("[0x%04X][OUT] Set 0x%02X on address 0x%04X \n", a, data, addr);
	PUSH(a, I_OUTB);
	PUSH(a, (addr >> 8) & 0xFF);
	PUSH(a, (addr >> 0) & 0xFF);
	PUSH(a, data);
}

void F_HLT(WORD &a) {
	printf("[0x%04X][HLT] Program halted!\n", a);
	PUSH(a, I_HLT);
}

void packMemToFile(BYTE* memory) {
	FILE* out_file = fopen("program.bin", "wb");
	fwrite(memory, sizeof(BYTE), MEM_SIZE, out_file);
	fclose(out_file);
}

void memPrint(uint8_t* mem, uint32_t size) {
	#define OUT_W 16

	for (uint32_t i = 0; i < size / OUT_W; i++) {
		printf("%08X | ", OUT_W * i);
		for (uint32_t j = 0; j < OUT_W; j++) {
			printf("%02X ", mem[(OUT_W * i) + j]);
		}
		printf("| ");
		for (uint32_t j = 0; j < OUT_W; j++) {
			if (mem[(OUT_W * i) + j]  > 31)
				printf("%.1c", mem[(OUT_W * i) + j]);
			else
				printf(".");
		}
		printf("\n");
	}
}

int main() {
	WORD p0 = 0x0000;
	WORD p1 = 0x0100;
	
	F_MOV(p0, SRC_VAL(0x02), DST_REG(STACK_SEG));
	F_OUTB(p0, 0x2000, 0xDE);
	F_OUTB(p0, 0x2001, 0xAD);
	F_OUTB(p0, 0x2002, 0xBE);
	F_OUTB(p0, 0x2003, 0xEF);
	F_CALL(p0, p1);
	
	F_RET(p1);
	F_LJMP(p0, p1);
	F_HLT(p1);
	
	memPrint(PROGRAM, MEM_SIZE);
	packMemToFile(PROGRAM);
}
