/*
 *  slc3.c
 *
 *  Date Due: June 1, 2018
 *  Authors:  Mike Josten, Tyler Shupack, Samantha "Immortal Darkfear 1337" Anderson
 *  Final Project
 *  version: 5/30a
 */

#include "slc3.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <ctype.h>

unsigned short memory[MEMORY_SIZE];
unsigned short mem_start;
bool isRun = false;
bool isHalted = false;
int outputLineCounter = 0;
int outputColCounter = 0;
unsigned short int breakpoints[MEMORY_SIZE];


/**
 * Simulates trap table lookup.
 * @param vector the area in memory that is simulated to be looked up to
 *        execute the requested TRAP routine.
 * @param cpu the cpu object that contains data.
 */
void trap(unsigned short vector, CPU_p *cpu, WINDOW *theWindow) {
    switch (vector) {
        case TRAP_VECTOR_X20: ;// GETC
            noecho(); //turn echo off
            char *input = (char*) malloc(sizeof(char));
            cursorAtInput(theWindow, input);
            cpu->reg[0] = *input;
                //printf("\nDOING TRAP X20\n");
            free(input);
            
            echo(); //turn echo back on.
            break;
            case TRAP_VECTOR_X21: ;// OUT
            /* put R0 value into char variable, then send to "cursor" function */
            char *output = (char*) malloc(sizeof(char) * 2);
            output[0] = cpu->reg[0];
            output[1] = '\0'; // null-terminator
            cursorAtOutput(theWindow, output);
            //printf("\nDOING TRAP X21\n");
            free(output);
            break;
        case TRAP_VECTOR_X22: ;//PUTS trap command
            //printf("\nDOING TRAP X22\n");
            char *outputString = (char*) malloc(sizeof(char) * 40);
            short memCounter = cpu->reg[0];
            short outCounter = 0;
            /* store characters in string until character is null pointer,
            * starting with character stored in memory at reg[0] and incrementing until 
            * null pointer is reached. */
            while (memory[memCounter] != 0) {
                outputString[outCounter] = memory[memCounter];
                memCounter++;
                outCounter++;
            }
            outputString[outCounter] = '\0';
            cursorAtOutput(theWindow, outputString);
	    free(outputString);
            break;
        case TRAP_VECTOR_X25: // HALT
            cursorAtPrompt(theWindow, "==========HALT==========");
            cpu->pc = 0; // reset to zero as per Prof Mobus.
            isRun = false;
	    isHalted = true;
            break;
        default: 
            cursorAtPrompt(theWindow, "Error: Unknown Trap vector");
            break;
    }
}

/**
 * The controller component of the LC-3 simulator.
 * @param cpu the cpu object to contain data.
 */
int controller(CPU_p *cpu, WINDOW *theWindow, bool step) {

    // check to make sure both pointers are not NULL
    // do any initializations here
    unsigned int opcode, dr, sr1, sr2, bit5, bit11, state, nzp;    // fields for the IR
    short offset, immed;
    unsigned short vector8, vector16;
    bool isCycleComplete = false;

    state = FETCH;
    while (isRun) {
        isCycleComplete = false;
        int i;
        //check for breakpoints
        for (i = 0; i < MEMORY_SIZE; i++){
                if (cpu->pc + mem_start == breakpoints[i]) {
                    step = true;
                }
            }
        switch (state) {
            
            case FETCH: // microstates 18, 33, 35 in the book.
                //printf("Now in FETCH---------------\n");
                cpu->mar = cpu->pc;           // Step 1: MAR is loaded with the contends of the PC,
                cpu->pc++;                    //         and also increment PC. Only done in the FETCH phase.
                cpu->mdr = memory[cpu->mar];  // Step 2: Interrogate memory, resulting in the instruction placed into the MDR.
                cpu->ir  = cpu->mdr;          // Step 3: Load the IR with the contents of the MDR.
                state    = DECODE;
                break;

            case DECODE: // microstate 32
                //printf("Now in DECODE---------------\n");
                opcode = (cpu->ir & MASK_OPCODE) >> BITSHIFT_OPCODE; // Input is the four-bit opcode IR[15:12]. The output line asserted is the one corresponding to the opcode at the input.
                //opcode = opcode  >> BITSHIFT_OPCODE;
                state = EVAL_ADDR;
                break;

            case EVAL_ADDR:
                //printf("Now in EVAL_ADDR---------------\n");
                // This phase computes the address of the memory location that is needed to process the instruction.
                // NOTE: Study each opcode to determine what all happens this phase for that opcode.
                // Look at the LD instruction to see microstate 2 example.
                switch (opcode) {
                    // different opcodes require different handling
                    // compute effective address, e.g. add sext(immed7) to
                    // register
                    case OP_LD:
			dr = (cpu->ir & MASK_DR) >> BITSHIFT_DR;
			offset = cpu->ir & MASK_PCOFFSET9;
			offset = SEXT(offset, BIT_PCOFFSET9);
                        cpu->mar = cpu->pc + offset; // microstate 2.
                        cpu->mdr = memory[cpu->mar]; // microstate 25.
                        break;
		    case OP_LDI:
			dr = (cpu->ir & MASK_DR) >> BITSHIFT_DR;
			offset = cpu->ir & MASK_PCOFFSET9;
			offset = SEXT(offset, BIT_PCOFFSET9);
			cpu->mar = cpu->pc + offset;
			cpu->mdr = memory[cpu->mar];
			//need to subtract by x3000 becuase user space starts at 0
			cpu->mdr -= ADDRESS_MIN;
			cpu->mar = cpu->mdr;
			cpu->mdr = memory[cpu->mar];
			break;
                    case OP_LDR:
                        dr       = (cpu->ir & MASK_DR)  >> BITSHIFT_DR;
                        sr1      = (cpu->ir & MASK_SR1) >> BITSHIFT_SR1;
                        offset   =  cpu->ir & MASK_PCOFFSET6;
			offset = SEXT(offset, BIT_PCOFFSET6);
                        cpu->mar =  cpu->reg[sr1] + offset;
                        cpu->mdr =  memory[cpu->mar];
                        break;
                    case OP_ST:
                        dr       = (cpu->ir & MASK_DR) >> BITSHIFT_DR;         // This is actually a source register, but still use dr.
                        offset   =  cpu->ir & MASK_PCOFFSET9;
			offset = SEXT(offset, BIT_PCOFFSET9);
                        cpu->mar =  cpu->pc + offset; // microstate 2.
                        break;
			case OP_STI:
			dr = (cpu->ir & MASK_DR) >> BITSHIFT_DR; //source register
			offset = cpu->ir & MASK_PCOFFSET9;
			offset = SEXT(offset, BIT_PCOFFSET9);
			cpu->mar = cpu->pc + offset;
			cpu->mdr = memory[cpu->mar];
			//needs to be subtracted by x3000 becuase memory user space starts at 0.
			cpu->mdr -= ADDRESS_MIN;
			cpu->mar = cpu->mdr;
			break;
                    case OP_STR:
                        dr       = (cpu->ir  & MASK_DR)  >> BITSHIFT_DR;  //actually source register
                        sr1      = (cpu->ir  & MASK_SR1) >> BITSHIFT_SR1;  //base register
                        offset   =  cpu->ir  & MASK_PCOFFSET6;
			offset = SEXT(offset, BIT_PCOFFSET6);
                        cpu->mar =  cpu->reg[sr1] + offset;
                        break;
                    case OP_LEA:
                        dr       = (cpu->ir & MASK_DR) >> BITSHIFT_DR;
                        offset   =  cpu->ir & MASK_PCOFFSET9;
			offset = SEXT(offset, BIT_PCOFFSET9);
                        break;
                    case OP_JSR:
                        offset = cpu->ir & MASK_PCOFFSET11;
			offset = SEXT(offset, BIT_PCOFFSET11);
                    break;
                }
                state = FETCH_OP;
                break;
                
            case FETCH_OP:
                //printf("Now in FETCH_OP---------------\n");
                switch (opcode) {
                    // get operands out of registers into A, B of ALU
                    // or get memory for load instr.
                    case OP_ADD:
                    case OP_AND:
                        dr   = (cpu->ir & MASK_DR)   >> BITSHIFT_DR;
                        sr1  = (cpu->ir & MASK_SR1)  >> BITSHIFT_SR1;
                        bit5 = (cpu->ir & MASK_BIT5) >> BITSHIFT_BIT5;
                        if (bit5 == 0) {
                            sr2 = cpu->ir & MASK_SR2; // no shift needed.
                        } else if (bit5 == 1) {
                            immed = cpu->ir & MASK_IMMED5; // no shift needed.
			    immed = SEXT(immed, BIT_IMMED);
                        }
                        // The book page 106 says current microprocessors can be done simultaneously during fetch, but this simulator is old skool.
                        break;
                    case OP_NOT:
                        dr  = (cpu->ir & MASK_DR) >> BITSHIFT_DR;
                        sr1 = (cpu->ir & MASK_SR1) >> BITSHIFT_SR1;
                        break;
                    case OP_TRAP:
                        vector8 = cpu->ir & MASK_TRAPVECT8; // No shift needed.
                        break;
                    case OP_ST: // Same as LD.
		    case OP_STI:
                    case OP_STR:
                        // Book page 124.
                        cpu->mdr = cpu->reg[dr];
                        break;
                    case OP_JMP:
                        sr1 = (cpu->ir & MASK_SR1) >> BITSHIFT_SR1;
                        break;
                    case OP_BR:
                        nzp = (cpu->ir & MASK_NZP) >> BITSHIFT_CC;
                        offset = cpu->ir & MASK_PCOFFSET9;
                        break;
                    case OP_JSR:
                        bit11 = (cpu->ir & MASK_BIT11) >> BITSHIFT_BIT11;
                        cpu->reg[7] = cpu->pc;
                        if (bit11 == 0) { //JSRR
                            sr1 = (cpu->ir & MASK_SR1) >> BITSHIFT_SR1;
                            cpu->pc = cpu->reg[sr1];
                        } else { //JSR
                            cpu->pc += offset;
                        }
                        break;
		    case OP_STACK: //push / pop
			dr = (cpu->ir & MASK_DR) >> BITSHIFT_DR;
			bit5 = (cpu->ir & MASK_BIT5) >> BITSHIFT_BIT5;
			break;
                }
                state = EXECUTE;
                break;

            case EXECUTE: // Note that ST does not have an execute microstate.
                //printf("Now in EXECUTE---------------\n");
                switch (opcode) {
                    case OP_ADD:
                        if (bit5 == 0) {
                            cpu->mdr = cpu->reg[sr2] + cpu->reg[sr1];
                        } else if (bit5 == 1) {
                            cpu->mdr = cpu->reg[sr1] + immed;
                        }
                        cpu->cc = getCC(cpu->mdr); 
                        break;
                    case OP_AND:
                        if (bit5 == 0) {
                            cpu->mdr = cpu->reg[sr2] & cpu->reg[sr1];
                        } else if (bit5 == 1) {
                            cpu->mdr = cpu->reg[sr1] & immed;
                        }
                        cpu->cc = getCC(cpu->mdr); 
                        break;
                    case OP_NOT:
                        cpu->mdr = ~cpu->reg[sr1]; // Interpret as a negative if the leading bit is a 1.
                        cpu->cc = getCC(cpu->mdr); 
                        break;
                    case OP_TRAP:
                        cpu->reg[7] = cpu->pc; // Store the PC in R7 before loading PC with the starting address of the service routine.
                        trap(vector8, cpu, theWindow);
                        break;
                    case OP_JMP:
                        cpu->pc = cpu->reg[sr1];
                        break;
                    case OP_BR: ;
			offset = SEXT(offset, BIT_PCOFFSET9);
                        if (branchEnabled(nzp, cpu)) {
                            cpu->pc += (offset);
			    
                        }
                        break;
                }
                state = STORE;
                break;
                
            case STORE: // Look at ST. Microstate 16 is the store to memory
                //printf("Now in STORE---------------\n");
                switch (opcode) {
                    // write back to register or store MDR into memory
                    case OP_ADD:
                    case OP_AND: // Same as ADD
                    case OP_NOT: // Same as AND and AND.
                        cpu->reg[dr] = cpu->mdr;
                        break;
                    case OP_LD:
		            case OP_LDI:
                    case OP_LDR:
                        cpu->reg[dr] = cpu->mdr; // Load into the register.
			cpu->cc = getCC(cpu->reg[dr]);
                        break;
                    case OP_ST:
		            case OP_STI:
                    case OP_STR:
                        memory[cpu->mar] = cpu->mdr;     // Store into memory.
                        break;
                    case OP_LEA:
                        cpu->reg[dr] = cpu->pc + offset;
                        cpu->cc = getCC(cpu->reg[dr]);
                        break;
		            case OP_STACK:
			            if (bit5 == 0) { //push
			    //decrement stack pointer then store the source register
			            cpu->reg[6]--;
			            memory[cpu->reg[6]] = cpu->reg[dr];
			            } else { //pop
			            //load the destination register with the stack pointer then increment SP
			                cpu->reg[dr] = memory[cpu->reg[6]];
			                cpu->reg[6]++;
			            }
			        break;
                    }
                // do any clean up here in prep for the next complete cycle
                isCycleComplete = true;
                state = FETCH;
                break;
        } // end switch (state)

        //if (!isRun) {
        //    cpu->pc = 0;
        //}

        if (step && isCycleComplete) {
            isRun = false;
        }

    } // end while()
    return 0;
} // end controller()

/**
 * Sets the condition code resulting by the resulting computer value.
 * @param value the value that was recently computed.
 * @return the condition code that represents the 3bit NZP as binary.
 */
bool branchEnabled(unsigned short nzp, CPU_p *cpu) {
    bool result = false;
    switch(nzp) {
    case CONDITION_NZP:
	result = true;
	break;
    case CONDITION_NP:
	if (cpu->cc == CONDITION_N || cpu->cc == CONDITION_P)
	    result = true;
	break;
    case CONDITION_NZ:
	if (cpu->cc == CONDITION_N || cpu->cc == CONDITION_Z)
	    result = true;
	break;
    case CONDITION_ZP:
	if (cpu->cc == CONDITION_Z || cpu->cc == CONDITION_P)
	    result = true;
	break;
    case CONDITION_N:
	if (cpu->cc == CONDITION_N)
	    result = true;
	break;
    case CONDITION_Z:
	if (cpu->cc == CONDITION_Z)
	    result = true;
	break;
    case CONDITION_P:
	if (cpu->cc == CONDITION_P)
	    result = true;
	 break;
    }    
    return result;
}

/**
* This function will determine the condition code based on the value passed
*/
unsigned short getCC(unsigned short value) {
    short signedValue = value;
    unsigned short code;
    if (signedValue < 0)
        code = CONDITION_N;
    else if (signedValue == 0)
            code = CONDITION_Z;
    else
        code = CONDITION_P;
    return code;
}


/**
 * This returns the same value except it is converted to a signed short instead.
 */
short toSign(unsigned short value) {
    short signedValue = value;
    return signedValue;
}

/**
* This function will take the loaction of the high order bit of the immediate value
* and sign extend it so that if the high order bit is a 1, then it will be converted to
* negative value.
* 
* @param value is the number to be sign extended.
* @param instance determines what the high order bit is of the value.
*/
short SEXT(unsigned short theValue, int highOrderBit) {
    short value = (short) theValue;
    switch(highOrderBit) {
    case BIT_IMMED:
	if (((value & BIT_IMMED) >> BITSHIFT_NEGATIVE_IMMEDIATE) == 1) 
            value = value | MASK_NEGATIVE_IMMEDIATE;
	break;
    case BIT_PCOFFSET11:
	if (((value & BIT_PCOFFSET11) >> BITSHIFT_NEGATIVE_PCOFFSET11) == 1)
	    value = value | MASK_NEGATIVE_PCOFFSET11;
	break;
    case BIT_PCOFFSET9:
	if (((value & BIT_PCOFFSET9) >> BITSHIFT_NEGATIVE_PCOFFSET9) == 1)
	    value = value | MASK_NEGATIVE_PCOFFSET9;
	break;
    case BIT_PCOFFSET6:
	if (((value & BIT_PCOFFSET6) >> BITSHIFT_NEGATIVE_PCOFFSET6) == 1)
	    value = value | MASK_NEGATIVE_PCOFFSET6;
	break;
    }
    return value;
}

/**
 * Simulating a ZEXT operation.
 */
unsigned short ZEXT(unsigned short value) {
    // Simulated ZEXT.
    return value;
}


/**
 * Print out fields to the console for the CPU_p object.
 * @param cpu the cpu object containing the data.
 */
void displayCPU(CPU_p *cpu, int memStart) {
    int c;
    int hexExit;
    initscr();
    cbreak();
    clear();
    WINDOW *main_win = newwin(32, 80, 0, 0);
    box(main_win, 0, 0);
    refresh();

    while(1) {
        bool rePromptUser = true;
        bool rePromptHex = true;
        int menuSelection = 0;
        int newStart = 0;
        char inStart[4];
        char printEnd[4];
        int lastPrint = 0;
        FILE *fptr;
        char saveFileName[20];
        int fileExists;
        int ii = 0;
        char memLocChange[4];
        char memConChange[4];
        short newMemLoc = 0;
        char *fileName = malloc(FILENAME_SIZE * sizeof(char)); //char fileName[FILENAME_SIZE];
        mvwprintw(main_win, 1, 20,  "Welcome to the LC-3 Simulator Simulator");
        mvwprintw(main_win, 2, 1,  "Registers");
        mvwprintw(main_win, 2, 31, "Memory");

    
        int i = 0;
        int j = 0;
        //registers all on one loop
        for(i = 0; i < 8; i++) {
            mvwprintw(main_win, 3+i, 1, "R%u: x%04X", i, cpu->reg[i]);   // Registers.
        }
        
        //memory all on one loop
        for (i = 0; i < 16; i++) {
            
            for (j = 0; j < MEMORY_SIZE; j++){
                if (i+memStart == breakpoints[j]) {
                     mvwprintw(main_win, 3+i, 28, "* x%04X: x%04X       ", i+memStart, memory[i + (memStart - ADDRESS_MIN)]);
                     break;
                }
                mvwprintw(main_win, 3+i, 28, "x%04X: x%04X     ", i+memStart, memory[i + (memStart - ADDRESS_MIN)]); // Memory.
       
            }
        }

        mvwprintw(main_win, 14, 1, "PC:  x%04X    IR: x%04X", cpu->pc+ADDRESS_MIN, cpu->ir, i+memStart, memory[i+(memStart-ADDRESS_MIN)]);
        i++;
        mvwprintw(main_win, 15, 1, "A:   x%04X     B: x%04X", cpu->A, cpu->B, i+memStart, memory[i+(memStart - ADDRESS_MIN)]);
        i++;
        mvwprintw(main_win, 16, 1, "MAR: x%04X   MDR: x%04X", cpu->mar+ADDRESS_MIN, cpu->ir, i+memStart, memory[i+(memStart-ADDRESS_MIN)]);
        i++;
        mvwprintw(main_win, 17, 1, "CC:  N:%d Z:%d P:%d",
                (cpu->cc >> BITSHIFT_CC_BIT3) & MASK_CC_N,
                (cpu->cc >> BITSHIFT_CC_BIT2) & MASK_CC_Z,
                cpu->cc  & MASK_CC_P,
                i+ADDRESS_MIN,
                memory[i+(memStart-ADDRESS_MIN)]);

        i++;
        mvwprintw(main_win, 19, 1, "Select: 1) Load 2) Save 3) Step 4) Run 5) DispMem 6) Edit 8) BreakPt 9) Exit");
        cursorAtPrompt(main_win, "");
        if (cpu->pc == 0 && !isHalted) {
            // Only do a single time, else what you want to display gets obliterated.
            mvwprintw(main_win, 22, 1, "                                           ");
            mvwprintw(main_win, 23, 1, ">                                          ");
	    outputColCounter = 0;
        }
        cursorAtPrompt(main_win, ""); // twice necessary to prevent overwrite.

            rePromptUser = false;
            CPU_p cpuTemp;
            noecho();
              
            c = wgetch(main_win); // This is what stops to prompt the user for an Option input.
            echo();
            
            box(main_win, 0, 0);
            refresh();
            switch(c){
                case '1':
                    cpuTemp = initialize();
                    clearOutput(main_win);
                    cpu = &cpuTemp;
                    clearPrompt(main_win);
                    cursorAtPrompt(main_win, "Specify file name: ");
                    wgetstr(main_win, fileName);
                    loadProgramInstructions(openFileText(fileName, main_win), main_win);
                    free(fileName);
                    box(main_win, 0, 0);
                    refresh();
                    break;
                case '2':
                	clearPrompt(main_win);
                	cursorAtPrompt(main_win, "Save file name: ");
                	wgetstr(main_win, saveFileName);
                	fptr = fopen(saveFileName, "r");
                	if (fptr == NULL) {
                		fileExists = 0;
                	} else {
                		fileExists = 1;
                		fclose(fptr);
                	}
                	if (fileExists == 1) {
                		fptr = fopen(saveFileName, "r+b");
                	} else {
                		fptr = fopen(saveFileName, "w+b");
                	}
                	if (fptr != NULL) {
                		fprintf(fptr, "%04x\n", mem_start);
                		clearPrompt(main_win);
                		cursorAtPrompt(main_win, "Last memory location you want printed: ");
                		wgetstr(main_win, printEnd);
                		if (hexCheck(printEnd)) {
                            lastPrint = strtol(printEnd, NULL, MAX_BIN_BITS);
                        } else {
                        	clearPrompt(main_win);
                            cursorAtPrompt(main_win, "You must enter a 4-digit hex value. Try again. ");
                        }
                		while (mem_start + ii <= lastPrint) {
                			fprintf(fptr, "%04x\n", memory[ii + (mem_start - ADDRESS_MIN)]);
                			ii++;
                		}
                		fclose(fptr);
                	} else {
                		clearPrompt(main_win);
                		cursorAtPrompt(main_win, "Error creating file.");
                	}
                	break;
                case '3':
                // do nothing.  Just let the PC run the next instruction.
                    isRun = true;
                    controller(cpu, main_win, true); 
                    wrefresh(main_win);
                    refresh();
                    break;
                case '4':
		    isHalted = false;
                    isRun = true;
                    controller(cpu, main_win, false);
                    wrefresh(main_win);
                    refresh();
                    break;
                case '5':
                    while (rePromptHex) {
                    	clearPrompt(main_win);
                        cursorAtPrompt(main_win, "New Starting Address: ");
                        wgetstr(main_win, inStart);
                        box(main_win, 0, 0);
                        refresh();
                        if (inStart[0] == 'q' || inStart[0] == 'Q') {
                            cursorAtPrompt(main_win, "");
                            rePromptUser = true;
                            break;
                        }
                        if (hexCheck(inStart)) {
                            newStart = strtol(inStart, NULL, MAX_BIN_BITS);
                            displayCPU(cpu, newStart);
                        } else {
                        	clearPrompt(main_win);
                            cursorAtPrompt(main_win, "You must enter a 4-digit hex value. Try again. ");
                            rePromptHex = true;
                        }
                    }
                    //printf("CASE5\n"); // Update the window for the memory registers.
                    break;
                case '6':
                	clearPrompt(main_win);
                	cursorAtPrompt(main_win, "Memory Location To Be Changed: ");
                	wgetstr(main_win, memLocChange);
                	if(hexCheck(memLocChange)) {
                		newMemLoc = strtol(memLocChange, NULL, MAX_BIN_BITS);
                		clearPrompt(main_win);
                		cursorAtPrompt(main_win, "New Value To Be Stored: ");
                		wgetstr(main_win, memConChange);
                		if (hexCheck(memConChange)) {
                			memory[newMemLoc - ADDRESS_MIN] = strtol(memConChange, NULL, MAX_BIN_BITS);
                			displayCPU(cpu, newMemLoc - 7);
                		} else {
                			clearPrompt(main_win);
                			cursorAtPrompt(main_win, "Did not input a valid hex value.");
                		}
                	} else {
                		clearPrompt(main_win);
                		cursorAtPrompt(main_win, "Did not input a valid hex value.");
                	}
                	break;
                case '8':
                clearPrompt(main_win);
                cursorAtPrompt(main_win, "Breakpoint location: ");
                wgetstr(main_win, inStart);
                //check if we're removing a value
                if (hexCheck(inStart)) {
                    int found = 0;
                    int i;
                    unsigned short int bp = strtol(inStart, NULL, MAX_BIN_BITS);
                    for (i = 0; i < MEMORY_SIZE; i++){
                        if (breakpoints[i] == bp){
                            breakpoints[i] = 0;
                            found = 1;
                        }

                    }
                    if (found == 0) {
                        //if not, add the new value in
                        for (i = 0; i < MEMORY_SIZE; i++){
                            if (breakpoints[i] == 0){
                                breakpoints[i] = bp;
                                break;
                            }
                        }
                    }
                }
                else {
                	clearPrompt(main_win);
                    cursorAtPrompt(main_win, "Did not input a valid hex value.");
                }
                	break;
                case '9':
                    //printf("CASE9\n");
                    endwin();
                    exit(0);
                    break;
                default:
                	clearPrompt(main_win);
                    cursorAtPrompt(main_win, "---Invalid selection ");
                    rePromptUser = true;
                    break;
            }
            wrefresh(main_win);
    }
}

void cursorAtPrompt(WINDOW *theWindow, char *theText) {
    if (isRun) {
         // First wipe out what ever is there.
        mvwprintw(theWindow, 20, 1, "                                               ");
    }
    mvwprintw(theWindow, 21, 1, "------------------------------------------------------------------------------");
    mvwprintw(theWindow, 20, 1, theText); //The last place the cursor will sit.
    refresh();
}

void clearPrompt(WINDOW *theWindow) {
	mvwprintw(theWindow, 20, 1, "                                                            ");
}

void cursorAtInput(WINDOW *theWindow, char *theText) {
    flushinp();
    int input = mvwgetch(theWindow, 22, 3);
    theText[0] = input;
    refresh();
}

void cursorAtOutput(WINDOW *theWindow, char *theText) {
    int i;
    char *text = (char*) malloc(sizeof(char) * 2);
    text[1] = '\0';
    for (i = 0; i < strlen(theText); i++) {
	text[0] = theText[i];
	mvwprintw(theWindow, OUTPUT_LINE_NUMBER + outputLineCounter, OUTPUT_COL_NUMBER + outputColCounter, text);
	outputColCounter++;
	if (theText[i] == 10) {
	    outputLineCounter++;
	    outputColCounter = 0;
	}
    }
    //mvwprintw(theWindow, OUTPUT_LINE_NUMBER + outputLineCounter, 8, theText);
    //outputLineCounter++;
    refresh();
}

void clearOutput(WINDOW *theWindow) {
    int i;
    mvwprintw(theWindow, OUTPUT_LINE_NUMBER, 1, ">                                         ");
    for (i = 1; i <= OUTPUT_AREA_DEPTH; i++) {
        mvwprintw(theWindow, OUTPUT_LINE_NUMBER + i, 2, "                                   ");
    }
    refresh();
    outputLineCounter = 0;
}

void cursorAtCustom(WINDOW *theWindow, int theRow, int theColumn, char *theText) {
    mvwprintw(theWindow, theRow, theColumn, theText);
    refresh();
}

/**
 * A function to check the validity of a hex number.
 * Returns 1 if true, 0 if false.
 */
int hexCheck(char num[]) {
    int counter = 0;
    int valid = 0;
    int i;

    for (i = 0; i < 4; i++) {
        if (isxdigit(num[i])) {
            counter++;
        }
    }
    if (counter == 4) {
        return 1;
    } else {
        return 0;
    }
}
/**
 * Sets all elements to zero.
 */
void zeroOut(unsigned short *array, int quantity) {
    int i;
    for (i = 0; i <= quantity; i++) {
        array[i] = 0;
    }
}

/**
 * Returns 1 if array element exists, 0 otherwise 
 */
int arrayContains(unsigned short *array, unsigned short target, int size) {
    int i;
    
    for (i = 0; i < size; i++)
    {
        if (array[i] == target) return 1;
    }
    
    return 0;
}

/**
 * Initializes a CPU_p object and its fields.
 * Removes the junk from these memory locations.
 */
CPU_p initialize() {
    CPU_p cpu = { 0    // PC
                , CONDITION_Z    // cc
                , { 0, 0, 0, 0, 0, 0, 0, 0 }
                , 0    // ir
                , 0    // mar
                , 0
                , 0
                , 0
                , 0};  // mdr

    zeroOut(memory, 100);

    // Intentionally hard coding these values into two memory registers.
    //cpu.reg[0] = 3;
    //cpu.reg[7] = 4;
    //cpu->reg[3] = 0xB0B0;   // Intentional simulated data.
    //memory[4]  = 0xA0A0;   // Intentional simulated data.
    //cpu->reg[0] = 0xD0E0;   // Intentional simulated data.
    return cpu;
}

/**
 * Opens a text file in read only mode.
 * @param theFileName the file name to open, in this present working directory.
 * @return the pointer to the file now opened.
 */
FILE* openFileText(char *theFileName, WINDOW *theWindow) {
    //printf("You said %s", theFileName); // debugging, remove me.
    FILE *dataFile;
    dataFile = fopen(theFileName, "r");
    //if ((dataFile = fopen(theFileName, "r")) == NULL) {
    if (dataFile == NULL) {
    //  printf("\n---ERROR: File %s could not be opened.\n\n", theFileName);
        char temp;
        if(theWindow == NULL) {
            printf("Error, File not found.  Press <ENTER> to continue.");
            fflush(stdin);
            temp = getchar(); // BUG this does not work as expected in option #1.
            printf("\n");
        } else {
            cursorAtPrompt(theWindow, "Error, File not found.");

        }
    } else {
        //printf("\nSUCCESS: File Found: %s\n\n", theFileName); // debugging
    }
    return dataFile;
}

/**
 * Loads the the instructions from the text file into memory[].
 * Expects one instruction per line, in hex number form, and a new line character.
 * @param inputFile the file to read.
 */
void loadProgramInstructions(FILE *inputFile, WINDOW *theWindow) {
    if (inputFile != NULL){
        char instruction [5]; // includes room for the carriage return character.
        int length = sizeof(instruction);
        int i = 0;
        unsigned short startingAddress = 0;

        /* The first line in the inputfile is actually the starting address of
           where to begin loading the instructions into memory.  This is
           a result of the .ORIG instruction in assembly. */
        if(!feof(inputFile)) {
            fgets(instruction, length, inputFile);
            startingAddress = strtol(instruction, NULL, MAX_BIN_BITS);
            fgets(instruction, length, inputFile); // processes the carriage return character.
        }

        // In this simulator, we start at ADDRESS_MIN which is the zero'th element in memory[].
        if (startingAddress >= ADDRESS_MIN) {
            i = (startingAddress - ADDRESS_MIN);
            mem_start = startingAddress;
            while(!feof(inputFile)) {
                fgets(instruction, length, inputFile);
                memory[i] = strtol(instruction, NULL, MAX_BIN_BITS);
                //printf("\n %04X", memory[i]); // debugging, confirms the memory[] does have the data.
                fgets(instruction, length, inputFile); // processes the carriage return character.
                i++;
                }
        } else {
            if(theWindow == NULL) {
                printf("Error, address must be between %x and %x\n"
                    , ADDRESS_MIN, (ADDRESS_MIN + MEMORY_SIZE));
            } else {
                cursorAtPrompt(theWindow, "Error, address "); // TODO specify min and max address.
            }
        }
        fclose(inputFile);
    }
}

/**
 * Driver for the program.
 */
int main(int argc, char* argv[]) {
    char *fileName = argv[1]; //char *fileName = "./HW3.hex";
    CPU_p cpu = initialize();
    zeroOut(breakpoints, MEMORY_SIZE);
    if(fileName != NULL) {
        loadProgramInstructions(openFileText(fileName, NULL), NULL);
    }
    displayCPU(&cpu, ADDRESS_MIN); // send the address of the object.
}
