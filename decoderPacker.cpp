#include "opcodes.h"
#include <assert.h>
#include <iostream>
#include <vector>

typedef	unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned int	DWORD;

#define BANK_SIZE	1024 * 4
#define BANK_COUNT	sizeof(DWORD)
#define MAX_STEPS	16

BYTE	MEM[BANK_SIZE * BANK_COUNT] = {0};

std::string toBinary(DWORD n, BYTE len) {
    std::string binary;
    for (unsigned i = (1 << len - 1); i > 0; i = i / 2) {
        binary += (n & i) ? "1" : "0";
    }
 
    return binary;
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

void packInstruction(BYTE ins, std::vector<DWORD> mi) {
	assert(mi.size() <= MAX_STEPS);
	printf("Pack instruction 0x%02x (exec time %d ticks)\n", ins, mi.size());
	for (BYTE i = 0; i < mi.size(); i++) {
		WORD addr = ins | (i << 8);
		printf("[0x%04x] Tick %02d -> %s\n", addr, i, toBinary(mi[i], 32).c_str());
		for (BYTE j = 0; j < BANK_COUNT; j++) {
			WORD offset = BANK_SIZE * j;
			MEM[addr + offset] = (mi[i] >> (8 * j)) & 0xFF;
		}
	}	
}

void packMemToFile(BYTE* memory) {
	for (int i = 0; i < BANK_COUNT; i++) {
		std::string filename = "decoder"+std::to_string(i)+".bin";
		FILE* out_file = fopen(filename.c_str(), "wb");
		fwrite(memory + (BANK_SIZE * i), sizeof(BYTE), BANK_SIZE, out_file);
		fclose(out_file);
	}
}

int main() {
/*
 * FETCH instruction sequence
 */
	packInstruction(I_FETCH, {
		L_IP_COUNT | L_IR_LOAD | L_DML_EN | L_UN(DML_EN(DML_MEM)) | L_EOI
	});

/*
 * MOV instruction sequence
 */
 	//MOV value from REGISTER to REGISTER
	packInstruction(I_MOV | DIR_SRC_REG | DIR_DST_REG, {
		L_IP_COUNT | L_EIR_LOAD | L_DML_EN | L_UN(DML_EN(DML_MEM)),
		L_DML_LOAD | L_DML_EN | L_DML_EIR,
		L_EIR_LOAD | L_IR_LOAD | L_EOI
	});

	//MOV value from REGISTER to ADDRESS in MEMORY
	packInstruction(I_MOV | DIR_SRC_REG | DIR_DST_MEM | SEG_DATA, {
		L_IP_COUNT | L_EIR_LOAD | L_DML_EN | L_UN(DML_EN(DML_MEM)),
		L_IP_COUNT | L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_DSEL)) | L_UN(DML_EN(DML_MEM)),
		L_DML_LOAD | L_DML_EN | L_DML_EIR,
		L_EIR_LOAD | L_IR_LOAD | L_EOI
	});

	//MOV value from ADDRESS in MEMORY to REGISTER
	packInstruction(I_MOV | DIR_SRC_MEM | DIR_DST_REG | SEG_DATA, {
		L_IP_COUNT | L_EIR_LOAD | L_DML_EN | L_UN(DML_EN(DML_MEM)),
		L_IP_COUNT | L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_DSEL)) | L_UN(DML_EN(DML_MEM)),
		L_DML_LOAD | L_DML_EN | L_DML_EIR,
		L_EIR_LOAD | L_IR_LOAD | L_EOI
	});

	//MOV value from CODE(direct load) to REGISTER
	packInstruction(I_MOV | DIR_SRC_MEM | DIR_DST_REG | SEG_CODE, {
		L_IP_COUNT | L_EIR_LOAD | L_DML_EN | L_UN(DML_EN(DML_MEM)),
		L_IP_COUNT | L_DML_LOAD | L_DML_EN | L_DML_EIR,
		L_EIR_LOAD | L_IR_LOAD | L_EOI
	});

	//MOV value from MEMORY to MEMORY
	packInstruction(I_MOV | DIR_SRC_MEM | DIR_DST_MEM | SEG_DATA, {
		L_IP_COUNT | L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_DSEL)) | L_UN(DML_EN(DML_MEM)),
		L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_TEMP)) | L_UN(DML_EN(DML_MEM)) | L_MUX_DATA,
		L_IP_COUNT | L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_DSEL)) | L_UN(DML_EN(DML_MEM)),
		L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_MEM)) | L_UN(DML_EN(DML_TEMP)) | L_MUX_DATA,
		L_EIR_LOAD | L_IR_LOAD | L_EOI
	});

/*
 * ADD instruction sequence
 */
 	//ADD value from CODE(direct load) to A REGISTER
	packInstruction(I_ADD | DIR_SRC_MEM | SEG_CODE, {
		L_IP_COUNT | L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_BREG)) | L_UN(DML_EN(DML_MEM)),
		L_DML_LOAD | L_UN(DML_LOAD(DML_AREG)) | L_ALU_EN,
		L_EIR_LOAD | L_IR_LOAD | L_EOI
	});

	//ADD value from MEMORY to A REGISTER
	packInstruction(I_ADD | DIR_SRC_MEM | SEG_DATA, {
		L_IP_COUNT | L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_DSEL)) | L_UN(DML_EN(DML_MEM)),
		L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_BREG)) | L_UN(DML_EN(DML_MEM)) | L_MUX_DATA,
		L_DML_LOAD | L_UN(DML_LOAD(DML_AREG)) | L_ALU_EN,
		L_EIR_LOAD | L_IR_LOAD | L_EOI
	});
/*
 * SUB instruction sequence
 */
 	//SUB value from CODE(direct load) to A REGISTER
	packInstruction(I_SUB | DIR_SRC_MEM | SEG_CODE, {
		L_IP_COUNT | L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_BREG)) | L_UN(DML_EN(DML_MEM)),
		L_DML_LOAD | L_UN(DML_LOAD(DML_AREG)) | L_ALU_EN | L_ALU_SUB,
		L_EIR_LOAD | L_IR_LOAD | L_EOI	
	});

	//SUB value from MEMORY to A REGISTER
	packInstruction(I_SUB | DIR_SRC_MEM | SEG_DATA, {
		L_IP_COUNT | L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_DSEL)) | L_UN(DML_EN(DML_MEM)),
		L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_BREG)) | L_UN(DML_EN(DML_MEM)) | L_MUX_DATA,
		L_DML_LOAD | L_UN(DML_LOAD(DML_AREG)) | L_ALU_EN | L_ALU_SUB,
		L_EIR_LOAD | L_IR_LOAD | L_EOI	
	});

/*
 * JMP instruction sequence
 */
 	//JMP to ADDRESS
	packInstruction(I_JMP | JMP(JMP_NO_COND), {
		L_JMP | JMP_UN(JMP_NO_COND) | L_DML_EN | L_UN(DML_EN(DML_MEM)),
		L_EIR_LOAD | L_IR_LOAD | L_EOI
	});

	//JMP to ADDRESS if CARRY FLAG is set
	packInstruction(I_JMP | JMP(JMP_CARRY), {
		L_JMP | JMP_UN(JMP_CARRY) | L_DML_EN | L_UN(DML_EN(DML_MEM)),
		L_CLR_FLAGS | L_EIR_LOAD | L_IR_LOAD | L_EOI
	});

	//JMP to ADDRESS if ZERO FLAG is set
	packInstruction(I_JMP | JMP(JMP_ZERO), {
		L_JMP | JMP_UN(JMP_ZERO) | L_DML_EN | L_UN(DML_EN(DML_MEM)),
		L_CLR_FLAGS | L_EIR_LOAD | L_IR_LOAD | L_EOI
	});

	//JMP to ADDRESS if NEGATIVE FLAG is set
	packInstruction(I_JMP | JMP(JMP_NEG), {
		L_JMP | JMP_UN(JMP_NEG) | L_DML_EN | L_UN(DML_EN(DML_MEM)),
		L_CLR_FLAGS | L_EIR_LOAD | L_IR_LOAD | L_EOI
	});

	//JMP to ADDRESS if OVERFLOW FLAG is set
	packInstruction(I_JMP | JMP(JMP_OVER), {
		L_JMP | JMP_UN(JMP_OVER) | L_DML_EN | L_UN(DML_EN(DML_MEM)),
		L_CLR_FLAGS | L_EIR_LOAD | L_IR_LOAD | L_EOI
	});
	
	//JMP to ADDRESS if CARRY FLAG is not set
	packInstruction(I_JMP | JMP(JMP_NO_CARRY), {
		L_JMP | JMP_UN(JMP_NO_CARRY) | L_DML_EN | L_UN(DML_EN(DML_MEM)),
		L_CLR_FLAGS | L_EIR_LOAD | L_IR_LOAD | L_EOI
	});

	//JMP to ADDRESS if ZERO FLAG is not set
	packInstruction(I_JMP | JMP(JMP_NO_ZERO), {
		L_JMP | JMP_UN(JMP_NO_ZERO) | L_DML_EN | L_UN(DML_EN(DML_MEM)),
		L_CLR_FLAGS | L_EIR_LOAD | L_IR_LOAD | L_EOI
	});

	//JMP to ADDRESS if NEGATIVE FLAG is not set
	packInstruction(I_JMP | JMP(JMP_NO_NEG), {
		L_JMP | JMP_UN(JMP_NO_NEG) | L_DML_EN | L_UN(DML_EN(DML_MEM)),
		L_CLR_FLAGS | L_EIR_LOAD | L_IR_LOAD | L_EOI
	});

	//JMP to ADDRESS if OVERFLOW FLAG is not set
	packInstruction(I_JMP | JMP(JMP_NO_OVER), {
		L_JMP | JMP_UN(JMP_NO_OVER) | L_DML_EN | L_UN(DML_EN(DML_MEM)),
		L_CLR_FLAGS | L_EIR_LOAD | L_IR_LOAD | L_EOI
	});
	
/*
 * CMP instruction sequence
 */
	packInstruction(I_CMP, {
		L_IP_COUNT | L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_BREG)) | L_UN(DML_EN(DML_MEM)),
		L_ALU_EN | L_ALU_SUB,
		L_EIR_LOAD | L_IR_LOAD | L_EOI
	});

/*
 * PUSH instruction sequence
 */
 	packInstruction(I_PUSH | DIR_SRC_REG, {
		L_IP_COUNT | L_EIR_LOAD | L_DML_EN | L_UN(DML_EN(DML_MEM)),
		L_DML_LOAD | L_DML_EN | L_DML_EIR,
		L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_MEM)) | L_UN(DML_EN(DML_TEMP)) | L_MUX_STACK,
		L_SP_COUNT | L_EIR_LOAD | L_IR_LOAD | L_EOI
	});
	
	packInstruction(I_PUSH | DIR_SRC_MEM | SEG_CODE, {
		L_IP_COUNT | L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_TEMP)) | L_UN(DML_EN(DML_MEM)),
		L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_MEM)) | L_UN(DML_EN(DML_TEMP)) | L_MUX_STACK,
		L_SP_COUNT | L_EIR_LOAD | L_IR_LOAD | L_EOI
	});

	packInstruction(I_PUSH | DIR_SRC_MEM | SEG_DATA, {
		L_IP_COUNT | L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_DSEL)) | L_UN(DML_EN(DML_MEM)),
		L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_TEMP)) | L_UN(DML_EN(DML_MEM)) | L_MUX_DATA,
		L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_MEM)) | L_UN(DML_EN(DML_TEMP)) | L_MUX_STACK,
		L_SP_COUNT | L_EIR_LOAD | L_IR_LOAD | L_EOI
	});

/*
 * POP instruction sequence
 */
 	packInstruction(I_POP | DIR_DST_REG, {
 		L_SP_COUNT | L_SP_DOWN | L_IP_COUNT | L_EIR_LOAD | L_DML_EN | L_UN(DML_EN(DML_MEM)),
 		L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_TEMP)) | L_UN(DML_EN(DML_MEM)) | L_MUX_STACK,
		L_DML_LOAD | L_DML_EN | L_DML_EIR,
		L_EIR_LOAD | L_IR_LOAD | L_EOI
	});

	packInstruction(I_POP | DIR_DST_MEM | SEG_DATA, {
		L_SP_COUNT | L_SP_DOWN,
		L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_TEMP)) | L_UN(DML_EN(DML_MEM)) | L_MUX_STACK,
		L_IP_COUNT | L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_DSEL)) | L_UN(DML_EN(DML_MEM)),
		L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_MEM)) | L_UN(DML_EN(DML_TEMP)) | L_MUX_DATA,
		L_EIR_LOAD | L_IR_LOAD | L_EOI
	});

/*
 * CALL instruction sequence
 */
	packInstruction(I_CALL, {		
		L_IP_COUNT | L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_BREG)) | L_UN(DML_EN(DML_MEM)),
		L_IP_COUNT | L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_TEMP)) | L_UN(DML_EN(DML_MEM)),
		
		L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_MEM)) | L_UN(DML_EN(DML_CSEG)) | L_MUX_STACK,
		L_SP_COUNT | L_DML_LOAD | L_UN(DML_LOAD(DML_MEM)) | L_IP_EN | L_MUX_STACK,
		
		L_SP_COUNT | L_IP_LOAD | L_DML_EN | L_UN(DML_EN(DML_TEMP)),
		L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_CSEG)) | L_UN(DML_EN(DML_BREG)) | L_MUX_STACK,
		
		L_EIR_LOAD | L_IR_LOAD | L_EOI
	});

/*
 * RET instruction sequence
 */
	packInstruction(I_RET, {
		L_SP_COUNT | L_SP_DOWN,
		L_IP_LOAD | L_DML_EN | L_UN(DML_EN(DML_MEM)) | L_MUX_STACK,
		
		L_SP_COUNT | L_SP_DOWN,
		L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_CSEG)) | L_UN(DML_EN(DML_MEM)) | L_MUX_STACK,
		
		L_EIR_LOAD | L_IR_LOAD | L_EOI
	});

/*
 * LJMP instruction sequence
 */
	packInstruction(I_LJMP, {
		L_IP_COUNT | L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_TEMP)) | L_UN(DML_EN(DML_MEM)),					//[0] Load new CSEG to TEMP register
		L_IP_LOAD | L_DML_EN | L_UN(DML_EN(DML_MEM)),															//[1] Load new IP from MEMORY
		L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_CSEG)) | L_UN(DML_EN(DML_TEMP)),								//[2] Load CSEG from TEMP register
		
		L_EIR_LOAD | L_IR_LOAD | L_EOI																			//[3] Reset register and call FETCH instruction	
	});
	
/*
 * OUT instruction sequence
 */
	packInstruction(I_OUTB, {
		L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_MEM)) | L_UN(DML_EN(DML_DSEG)) | L_MUX_STACK,					//[0] Save DSEG into stack
		L_SP_COUNT | L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_MEM)) | L_UN(DML_EN(DML_DSEL)) | L_MUX_STACK,	//[1] Save DSEL into stack

		L_IP_COUNT | L_SP_COUNT | L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_DSEG)) | L_UN(DML_EN(DML_MEM)),		//[2] Load new DSEG
		L_IP_COUNT | L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_DSEL)) | L_UN(DML_EN(DML_MEM)),					//[3] Load new DSEL

		L_IP_COUNT | L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_TEMP)) | L_UN(DML_EN(DML_MEM)),					//[4] Load value into TEMP register
		L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_MEM)) | L_UN(DML_EN(DML_TEMP)) | L_MUX_DATA,					//[5] Store value from TEMP into MEMORY

		L_SP_COUNT | L_SP_DOWN,																					//[6] Count down stack pointer
		L_SP_COUNT | L_SP_DOWN | L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_DSEL)) | L_UN(DML_EN(DML_MEM)) | L_MUX_STACK,	//[7] Load old DSEL
		L_DML_LOAD | L_DML_EN | L_UN(DML_LOAD(DML_DSEG)) | L_UN(DML_EN(DML_MEM)) | L_MUX_STACK,					//[8] Load old DSEG

		L_EIR_LOAD | L_IR_LOAD | L_EOI																			//[9] Reset registers and call FETCH instruction
	});
		
/*
 * HLT instruction sequencer code
 */
	packInstruction(I_HLT, {
		L_HLT, L_HLT, L_HLT, L_HLT, L_HLT, L_HLT, L_HLT, L_HLT,
		L_HLT, L_HLT, L_HLT, L_HLT, L_HLT, L_HLT, L_HLT, L_HLT
	});

	//memPrint(MEM, BANK_SIZE * BANK_COUNT);
	packMemToFile(MEM);
}
