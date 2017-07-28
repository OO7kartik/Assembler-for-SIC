# Assembler for SIC


This is a One Pass Assembler for SIC (Simplified Instruction Computer).

## What is SIC? ##

The SIC machine has basic addressing, storing most memory addresses hexadecimal integer format. Similar to most modern computing systems, the SIC architecture stores all data in binary and uses the two's complement to represent negative values at the machine level. Memory storage in SIC consists of 8-bit bytes, and all memory addresses in SIC are byte addresses. Any three consecutive bytes form a 24-bit 'word' value, addressed by the location of the lowest numbered byte in the word value. Numeric values are stored as word values, and character values use the 8-bit ASCII system. The SIC machine does not support floating-point hardware and have at most 32,768 bytes of memory. There is also a more complicated machine built on top of SIC called the Simplified Instruction Computer with Extra Equipment (SIC/XE). The XE expansion of SIC adds a 48-bit floating point data type, an additional memory addressing mode, and extra memory (1 megabyte instead of 32,768 bytes) to the original machine. All SIC assembly code is upwards compatible with SIC/XE.

SIC machines have several registers, each 24 bits long and having both a numeric and character representation:

A (0): Used for basic arithmetic operations; known as the accumulator register.
X (1): Stores and calculates addresses; known as the index register.
L (2): Used for jumping to specific memory addresses and storing return addresses; known as the linkage register.
PC (8): Contains the address of the next instruction to execute; known as the program counter register.
SW (9): Contains a variety of information, such as carry or overflow flags; known as the status word register.
In addition to the standard SIC registers, there are also four additional general-purpose registers specific to the SIC/XE machine:

B (3): Used for addressing; known as the base register.
S (4): No special use, general purpose register.
T (5): No special use, general purpose register.
F (6): Floating point accumulator register (This register is 48-bits instead of 24).
These five/nine registers allow the SIC or SIC/XE machine to perform most simple tasks in a customized assembly language. In the System Software book, this is used with a theoretical series of operation codes to aid in the understanding of assemblers and linker-loaders required for the execution of assembly language code.

## SIC Assembly Syntax ##

SIC uses a special assembly language with its own operation codes that hold the hex values needed to assemble and execute programs. A sample program is provided below to get an idea of what a SIC program might look like. In the code below, there are three columns. The first column represents a forwarded symbol that will store its location in memory. The second column denotes either a SIC instruction (opcode) or a constant value (BYTE or WORD). The third column takes the symbol value obtained by going through the first column and uses it to run the operation specified in the second column. This process creates an object code, and all the object codes are put into an object file to be run by the SIC machine.

      COPY   START  1000
      FIRST  STL    RETADR
      CLOOP  JSUB   RDREC
             LDA    LENGTH
             COMP   ZERO
             JEQ    ENDFIL
             JSUB   WRREC
             J      CLOOP
      ENDFIL LDA    EOF
             STA    BUFFER
             LDA    THREE
             STA    LENGTH
             JSUB   WRREC
             LDL    RETADR
             RSUB
      EOF    BYTE   C'EOF'
      THREE  WORD   3
      ZERO   WORD   0
      RETADR RESW   1
      LENGTH RESW   1
      BUFFER RESB   4096
      .
      .      SUBROUTINE TO READ RECORD INTO BUFFER
      .
      RDREC  LDX    ZERO
             LDA    ZERO
      RLOOP  TD     INPUT
             JEQ    RLOOP
             RD     INPUT
             COMP   ZERO
             JEQ    EXIT
             STCH   BUFFER,X
             TIX    MAXLEN
             JLT    RLOOP
      EXIT   STX    LENGTH
             RSUB
      INPUT  BYTE   X'F1'
      MAXLEN WORD   4096
      .
      .      SUBROUTINE TO WRITE RECORD FROM BUFFER
      .
      WRREC  LDX    ZERO
      WLOOP  TD     OUTPUT
             JEQ    WLOOP
             LDCH   BUFFER,X
             WD     OUTPUT
             TIX    LENGTH
             JLT    WLOOP
             RSUB
      OUTPUT BYTE   X'06'
             END    FIRST

If you were to assemble this program, you would get the object code depicted below. The beginning of each line consists of a record type and hex values for memory locations. For example, the top line is an 'H' record, the first 6 hex digits signify its relative starting location, and the last 6 hex digits represent the program's size. The lines throughout are similar, with each 'T' record consisting of 6 hex digits to signify that line's starting location, 2 hex digits to indicate the size (in bytes) of the line, and the object codes that were created during the assembly process.

      HCOPY 00100000107A
      T0010001E1410334820390010362810303010154820613C100300102A0C103900102D
      T00101E150C10364820610810334C0000454F46000003000000
      T0020391E041030001030E0205D30203FD8205D2810303020575490392C205E38203F
      T0020571C1010364C0000F1001000041030E02079302064509039DC20792C1036
      T002073073820644C000006
      E001000

NOTE: '^' is inserted in my implementation of object code for human readers.

## Instructions for User ##

This program uses C++/C++11.

To compile the source use:-  
g++ -std=c++11 assembler.cpp -o assembler
