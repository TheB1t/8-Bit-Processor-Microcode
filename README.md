# 8-Bit processor microcode
## Architecture schematic
![alt text] (/pictures/schematic.png)

## Architecture features
- 8-Bit data bus
- 16-bit address bus with SEG:SEL addressing
    - Code segment 
        - CSEG - Code segment register
        - IP - Instruction pointer
    - Data segment
        - DSEG - Data segment register
        - DSEL - Data selection register
- 32-bit sequencer
- Incomplete implementation of I/O ports (output only) (OUTB instruction)
- Full implementation of data movement between internal modules (MOV instruction)
- 2 General-purpose registers (A and B registers)
- Stack support (PUSH and POP instructions)
    - SSEG - Stack segment register
    - SP - Stack pointer

## Supported instructions
- MOV [val or src] [dst] - Moves data between memory and registers (Data segment sets manually)
    - MEM to MEM
    - MEM to REG
    - REG to MEM
    - REG to REG
- ADD [val] - Adds an argument to a value in the A register
- SUB [val] - Subtracts the argument to a value in the A register
- JMP [addr] - Jumps to the specified 8-bit address within the segment (short jump)
- J* [addr] - Conditional jumps
    - JC - Jump is carry flag set
    - JZ - Jump is zero flag set
    - JN - Jump is negative flag set
    - JO - Jump is overflow flag set
    - JNC - Jump is carry flag reset
    - JNZ - Jump is zero flag reset
    - JNN - Jump is negative flag reset
    - JNO - Jump is overflow flag reset
- CMP [val] - Compares the argument with the value of the A register
- PUSH [val or src] - Puts the value on the stack
- POP [dst] - Retrieves a value from the stack
- CALL [addr] - Calling a function address
- RET - Return from function
- LJMP [addr] - Jumps to the specified 16-bit address (long jump)
- OUTB [addr] [value] - Set byte in port
- HLT - Halt executing
