typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned int	DWORD;

/*
 * Sequencer logic outputs
 */
#define L_HLT			(0b1 << 0)
#define L_IP_COUNT		(0b1 << 1)
#define L_IP_LOAD		(0b1 << 2)
#define L_IP_EN			(0b1 << 3)
#define L_ALU_SUB		(0b1 << 4)
#define L_ALU_EN		(0b1 << 5)
#define L_IR_LOAD		(0b1 << 6)
#define L_IR_EN			(0b1 << 7)

#define L_EIR_LOAD		(0b1 << 8)
#define L_EIR_EN		(0b1 << 9)
#define L_DML_LOAD		(0b1 << 10)
#define L_DML_EN		(0b1 << 11)
#define L_DML_EIR		(0b1 << 12)
//						(0b1 << 13)
#define L_JMP			(0b1 << 14)
#define L_CLR_FLAGS		(0b1 << 15)

#define L_SP_LOAD		(0b1 << 16)
#define L_SP_EN			(0b1 << 17)
#define L_SP_COUNT		(0b1 << 18)
#define L_SP_DOWN		(0b1 << 19)
//						(0b1 << 20)
#define L_MUX0			(0b1 << 21)
#define L_MUX1			(0b1 << 22)
#define L_EOI			(0b1 << 23)

#define L_UN(D)			((D) << 24)

#define L_MUX_CODE		~(L_MUX0 | L_MUX1)
#define L_MUX_DATA		(L_MUX0)
#define L_MUX_STACK		(L_MUX1)

/*
 * Data Movement Logic
 */
#define DML_EN(D)		(D)
#define DML_LOAD(D)		((D) << 4)

#define DML_CODE		0b0000
#define DML_DATA		0b1000

#define DML_MEM			0b0000
#define DML_AREG		0b0001
#define DML_BREG		0b0010
#define DML_CSEG		0b0011
#define DML_DSEG		0b0100
#define DML_DSEL		0b0101
#define DML_SSEG		0b0110
#define DML_TEMP		0b0111


/*
 * General purpose definitions
 */

#define SRC(D)			(DML_EN(D))
#define DST(D)			(DML_LOAD(D))

#define MEMORY			(DML_MEM)
#define A_REG			(DML_AREG)
#define B_REG			(DML_BREG)
#define CODE_SEG		(DML_CSEG)
#define DATA_SEG		(DML_DSEG)
#define DATA_SEL		(DML_DSEL)
#define STACK_SEG		(DML_SSEG)
#define TEMP_REG		(DML_TEMP)

#define DIR_Msk				(0b00110000)
#define DIR_REG				(0b0)
#define DIR_MEM				(0b1)
#define DIR_DST_REG			(0b00000000)
#define DIR_DST_MEM			(0b00010000)
#define DIR_SRC_REG			(0b00000000)
#define DIR_SRC_MEM			(0b00100000)
#define DIR_SRC_GET(D)		(((D) >> 5) & 0b1)
#define DIR_DST_GET(D)		(((D) >> 4) & 0b1)
#define SEG_Msk				(0b10000000)
#define	SEG_CODE			(0b00000000)
#define SEG_DATA			(0b10000000)

#define SRC_VAL(D)		((BYTE)(D) | (DIR_SRC_MEM | SEG_CODE) << 4)
#define SRC_ADDR(D)		((BYTE)(D) | (DIR_SRC_MEM | SEG_DATA) << 4)
#define SRC_REG(D)		((BYTE)(D) | (DIR_SRC_REG) << 4)

#define DST_ADDR(D)		((BYTE)(D) | (DIR_DST_MEM | SEG_DATA) << 4)
#define DST_REG(D)		((BYTE)(D) | (DIR_DST_REG) << 4)
 
/*
 * JMP instruction definitions
 */

#define JMP(D)			((D) << 4)
#define JMP_UN(D)		(DWORD)(L_UN((D) << 4))
#define JMP_NO_COND		(0b0000)
#define JMP_CARRY		(0b1000)
#define JMP_ZERO		(0b1001)
#define JMP_NEG			(0b1010)
#define JMP_OVER		(0b1011)
#define	JMP_NO_CARRY	(0b1100)
#define JMP_NO_ZERO		(0b1101)
#define	JMP_NO_NEG		(0b1110)
#define JMP_NO_OVER		(0b1111)


/*
 * Processor instructions
 */
#define	I_FETCH			0x00	//Fetch next instruction
#define I_MOV			0x01	//Move data
#define	I_ADD			0x02	//Add number
#define I_SUB			0x03	//Subtract number
#define I_JMP			0x04	//Short jump & conditional jump
#define I_CMP			0x05	//Compare number
#define I_PUSH			0x06	//Push into stack
#define I_POP			0x07	//Pop from stack
#define I_CALL			0x08	//Call function
#define I_RET			0x09	//Return from function
#define I_LJMP			0x0A	//Long jump
#define I_OUTB			0x0B	//Writes byte in port
#define I_HLT			0xFF	//Halt processor
