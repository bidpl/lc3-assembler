#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simulator.h"

#define BUFF_LEN 127
#define PATH_BUFF_LEN 255
#define LAST_MEM_ADDR 0xFDFF
#define DUMP_LEN 10

#define MAX_INSTR_LEN 35

const char *INSTR[] = {"BR", "ADD", "LD", "ST", "JSR", "AND", "LDR", "STR", "RTI", "NOT", "LDI", "STI", "JMP", "RET", "LEA", "TRAP", "JSRR"};

int main(int argc, char *argv[]) {
    char readBuff[BUFF_LEN];
    char command[BUFF_LEN];
    char option1[BUFF_LEN];
    char option2[BUFF_LEN];
    char lastFile[BUFF_LEN];
    int numRead;

    printf("\n\n===================================\nLC-3 Simulator by Binh-Minh Nguyen \nUIUC ECE220 SP22 Honors Project\nCompiled: %s %s\n===================================\n\n\n", __DATE__, __TIME__);

    // Initialize LC-3 struct
    CPU* lc3cpu = create_CPU();

    // Check if there are input file(s)
    if(argc >= 2) {
        // Try to load each input file
        int i;
        for(i = 1; i < argc; i++ ) {
            load_binary(lc3cpu, argv[i]);
            strcpy(lastFile, argv[i]);
        }
    }

    // Listen for commands loop
    while(1) {
        printf("LC3sim> ");
        gets(readBuff);

        // Reuse prev command if empty input (only overwrite if not empty)
        if(readBuff[0] != '\0') {
            numRead = sscanf(readBuff, "%s %s %s", command, option1, option2);
        } else {
            printf("reuse last command\n");
        }

        if(strcmp(command, "file") == 0 || strcmp(command, "f") == 0) {
            if( load_binary(lc3cpu, option1) ) {continue;}
            strcpy(lastFile, option1);

            printf("Loaded file: %s\n\n", option1);
            continue;

        } else if(strcmp(command, "break") == 0 || strcmp(command, "b") == 0) {
            // printf("Breakpoint management");
            if(numRead == 2) {
                if(strcmp(option1, "list") == 0) {
                    for(int i = 0; i < lc3cpu->bIndex; i++) {
                        printf("%02d: %04X   ", i, lc3cpu->breakpoints[i]);
                        print_instr(lc3cpu, lc3cpu->breakpoints[i]);
                    }
                } else if(strcmp(option1, "clear") == 0) {
                    for(int i = 0; i < lc3cpu->bIndex; i++) {
                        lc3cpu->breakpoints[i] = 0;
                    }

                    lc3cpu->bIndex = 0;
                } else {
                    __UINT16_TYPE__ memAddr = get_sym_addr(lc3cpu, option1);

                    if(memAddr == 0) {
                        if(sscanf(option1, "%*c %hx", &memAddr) != 1) {
                            printf("Invalid label/memory address\n");
                            continue;
                        }
                    }

                    lc3cpu->breakpoints[lc3cpu->bIndex] = memAddr;
                    lc3cpu->bIndex++;
                    // Print confirmation
                }
                
            } else {printf("Invalid input for breakpoint");}   
            continue;   
        } else if(strcmp(command, "continue") == 0 || strcmp(command, "c") == 0) {
            __UINT16_TYPE__ bPoint;
            do {
                runCycle(lc3cpu);
                // Debug only
                // printregs(lc3cpu);
                
                // Keep on running while MCR is active and not a breakpt
            } while( !(bPoint = checkBreakPt(lc3cpu)) && (lc3cpu->memory[0xFFFE] & 0x8000));

            // If Halted
            if(!(lc3cpu->memory[0xFFFE] & 0x8000)) {
                printf("--- Machine Halted --- \n");
                printregs(lc3cpu);
                continue;
            }

            printf("Execution hit breakpoint: x%04hX\n", bPoint);
            printregs(lc3cpu);
            continue;
        } else if(strcmp(command, "step") == 0 || strcmp(command, "s") == 0){
            runCycle(lc3cpu);
            printregs(lc3cpu);
            continue;
        } else if(strcmp(command, "list") == 0 || strcmp(command, "l") == 0) {
            // printf("List instructions @ PC (defualt), address, or label\n");
            __UINT16_TYPE__ memAddr;

            if(numRead == 1) {
                memAddr = lc3cpu->PC;
            } else {
                // TODO make this not case sensitive
                memAddr = get_sym_addr(lc3cpu, option1);
                
                if(memAddr == 0) {
                    if(sscanf(option1, "%*c %hx", &memAddr) != 1) {
                        printf("Invalid label/memory address\n");
                        continue;
                    }
                }            
            }

            for(int i = 0; i < DUMP_LEN; i++) {
                printf("x%04X:   x%04hX   ", memAddr + i, lc3cpu->memory[memAddr+i]);

                print_instr(lc3cpu, memAddr + i);
            }
             
            printf("\n");  
        } else if(strcmp(command, "dump") == 0 || strcmp(command, "d") == 0) {
            // printf("Dump memory @ PC (defualt), address, or label");

            __UINT16_TYPE__ memAddr;

            if(numRead == 1) {
                memAddr = lc3cpu->PC;
            } else {
                // TODO make this not case sensitive
                memAddr = get_sym_addr(lc3cpu, option1);
                
                if(memAddr == 0) {
                    if(sscanf(option1, "%*c %hx", &memAddr) != 1) {
                        printf("Invalid label/memory address\n");
                        continue;
                    }
                }            
            }
                
            for(int i = 0; i < DUMP_LEN; i++) {
                dumpMem(lc3cpu, memAddr + i);
            }    
            printf("\n");

        } else if(strcmp(command, "translate") == 0 || strcmp(command, "t") == 0) {
            // printf("Show value of label + print contents");
            if(numRead != 2) {
                printf("Invalid arguments (invalid # of args)\n");
                continue;
            }

            __UINT16_TYPE__ memAddr = get_sym_addr(lc3cpu, option1);

            if(memAddr == 0) {
                printf("Could not find label: %s\n", option1);
            }else {
                printf("%s: x%04X\n", option1, memAddr);
                printf("x%04X  ", lc3cpu->memory[memAddr]);
                print_instr(lc3cpu, memAddr);
            }
            
        } else if(strcmp(command, "printregs") == 0 || strcmp(command, "p") == 0) {
            // printf("Print registers and current instruction\n");
            printregs(lc3cpu);

        } else if (strcmp(command, "memory") == 0 || strcmp(command, "m") == 0) {
            // printf("Set value in memory location\n");
            __UINT16_TYPE__ memAddr = get_sym_addr(lc3cpu, option1);
            
            if(memAddr == 0) {
                if(sscanf(option1, "%*c %hx", &memAddr) != 1) {
                    printf("Invalid label/memory address\n");
                    continue;
                }
            }

            __UINT16_TYPE__ value = get_sym_addr(lc3cpu, option2);
            if(value == 0) {
                if(option2[0] == '#') {
                    sscanf(option2+1, "%hd", &value);
                } else if(option2[0] == 'x' || option2[0] == 'X') {
                    sscanf(option2+1, "%hx", &value);
                } else {
                    printf("Invalid value format\n");
                    continue;
                }
            }

            lc3cpu->memory[memAddr] = value;

        } else if(strcmp(command, "register") == 0 || strcmp(command, "reg") == 0) {
            // printf("Set value in register");
            __UINT16_TYPE__ regNum;

            if(option1[0] != 'R' || sscanf(option1+1, "%hd", &regNum) != 1 || regNum >= 8) {
                printf("Invalid register");
                continue;
            }

            __UINT16_TYPE__ value = get_sym_addr(lc3cpu, option2);
            if(value == 0) {
                if(option2[0] == '#') {
                    sscanf(option2+1, "%hd", &value);
                } else if(option2[0] == 'x' || option2[0] == 'X') {
                    sscanf(option2+1, "%hx", &value);
                } else {
                    printf("Invalid value format\n");
                    continue;
                }
            }
            
            lc3cpu->regfile[regNum] = value;
            continue;
        } else if(strcmp(command, "quit") == 0 || strcmp(command, "q") == 0) {
            printf("Are you sure you want to quit? [y]: ");
            scanf("%s", command);

            if(strcmp(command, "y") == 0) {
                printf("Quitting...\n");
                break;
            } else {
                printf("Quit canceled\n");
                continue;
            }
        } else if(strcmp(command, "q!") == 0) {
            break;
        } else if(strcmp(command, "reset") == 0) {
            printf("Reseting simulation... ");
            
            destroy_CPU(lc3cpu);
            lc3cpu = create_CPU();
            load_binary(lc3cpu, lastFile);

            printf(" Loaded %s\n", lastFile);
            continue;
        } else {
            printf("Did not recognize command: %s\n\n", command);
            continue;
        }

    }

    destroy_CPU(lc3cpu);

    return 0;
}


/**
 * @brief: create_CPU: Creates an instance of CPU struct and returns its pointer
 * @return: CPU* to new instance of CPU
 */
CPU * create_CPU(){
    CPU* newCPU = (CPU*) malloc(sizeof(CPU));

    // Initialize values to 0
    for(int i = 0; i < 8; i++) {
        newCPU->regfile[i] = 0;
    }
    newCPU->numSymbols = 0;
    newCPU->bIndex = 0;

    // Set MCR[15] to active
    newCPU->memory[0xFFFE] = 0x8000;

    return newCPU;
}


/** 
 * @brief destroy_CPU: Frees CPU struct from memory
 * 
 * @param CPU* cpu: pointer to CPU struct to be freed
 * @return nothing
 */
void destroy_CPU(CPU* cpu) {
    int i;
    for(i = 0; i < cpu->numSymbols; i++) {
        free(cpu->symbols[i]);
    }

    free(cpu);
}


/**
 * @brief load_binary: Tries to open a binary file and copy its contents to CPU's ram and sets PC.
 * Also tries to import a symbol table + cleaned code for preview.
 * 
 * @param char* filename: path to binary file
 * @return 0 on success, 1 if no symbol table2 if invalid binary filename, 3 if issue with reading binary, 4 if memory overflow
 */
int load_binary(CPU* cpu, char* filename){
    __UINT16_TYPE__ memAddr;

    char bin_filename[PATH_BUFF_LEN];
    char sym_filename[PATH_BUFF_LEN];

    __UINT16_TYPE__ readBuff;
    char buff[BUFF_LEN];

    parse_filename(filename, bin_filename, sym_filename);

    FILE* bin_fp = fopen(bin_filename, "rb");
    if(bin_fp == NULL) {
        printf("Can't open file: %s\n\n", bin_filename);
        return 2;
    }

    // Load instructions here

    // Read in 2 bytes = 16 bit word from memory (this will be PC start address)
    // Also check for sucessful read
    if(fread(&readBuff, 2, 1, bin_fp) != 1) {
        printf("Issue reading binary file.\n");
        return 3;
    }

    // Write PC start address to PC
    // Flip endianness of reading
    readBuff = le_to_be(readBuff);

    cpu->PC = readBuff;
    memAddr = readBuff;
    // printf("Starting at x%04X\n", readBuff);

    // Keep on reading file and appending to memory;
    while(!feof(bin_fp)) {
        // Read next word in file + read check
        if(fread(&readBuff, 2, 1, bin_fp) > 1) {
            printf("Issue reading binary file. %d\n", feof(bin_fp));
            return 3;
        }

        // feof isn't set until an invalid read
        if(feof(bin_fp)) {
            break;
        }

        // Check for valid memory address
        if(memAddr > LAST_MEM_ADDR) {
            printf("File overflowed into I/O map section of memory (xFE00 - xFFFF) \n");
            return 3;
        }

        // Flip endianness of reading
        readBuff = le_to_be(readBuff);
        
        // Write word to memory
        // printf("x%04X: x%04X\n", memAddr, readBuff);
        cpu->memory[memAddr] = readBuff;
        memAddr++;
    }

    fclose(bin_fp);


    FILE* sym_fp = fopen(sym_filename, "r");
    if(sym_fp == NULL) {
        printf("Warning, no symbol table loaded \n");

        return 1;
    }

    // TODO Load sym table here
    // Flush symbol table header
    fgets(buff, BUFF_LEN, sym_fp); // Symbol table
    fgets(buff, BUFF_LEN, sym_fp); // Scope level 0:
    fgets(buff, BUFF_LEN, sym_fp); //	Symbol Name       Page Address
    fgets(buff, BUFF_LEN, sym_fp); //	----------------  ------------

    // Read and import symbol table
    while(fgets(buff, BUFF_LEN, sym_fp)) {
        char symName[20];
        __UINT16_TYPE__ addr;


        // Read in line + parse
        if(sscanf(buff, "%*s %s %hx", symName, &addr) != 2) {
            if(strcmp(buff, "\n") != 0) {
                printf("Bad symbol table line: %s\n", buff);
            }
        }

        // Add to symbol table
        cpu->symbols[cpu->numSymbols] = (char *) malloc(strlen(symName) + 1);
        strcpy(cpu->symbols[cpu->numSymbols], symName);
        cpu->symAddr[cpu->numSymbols] = addr;
        cpu->numSymbols++;
    }

    // Debug print symbol table
    // for(int i = 0; i < cpu->numSymbols; i++) {
    //     printf("%s: %04X\n", cpu->symbols[i], get_sym_addr(cpu, cpu->symbols[i]));
    // }
    
    fclose(sym_fp);

    return 0;
}


/**
 * @brief parse_filename: Takes a filename, and generates obj and sym filenames. 
 * Checks if there's a file extension, * If .obj or no extension. generate .sym file name, if invalid extension, fill neither;
 * Won't work with extensionless paths in hidden files (last period in path has to be for extension)
 * 
 * @param char* filename: String containing path/filename
 * @output char* bin_filename: String to be filled with binary filename
 * @output char* sym_filename: String to be filled with symbol table filename
 * @return 0 on success, 1 if invalid extension or NULL filename
 * 
 */
int parse_filename(char* filename, char* bin_filename, char* sym_filename) {
    // NULL check
    if(filename == NULL) {
        return 1;
    }
    
    // Pointer to last dot in filename (beginning of file extension)
    char* lastDot = strchr(filename, '.');

    // If no file extention
    if(lastDot == NULL) {
        strcpy(bin_filename, filename);
        strcat(bin_filename, ".obj");

        strcpy(sym_filename, filename);
        strcat(sym_filename, ".sym");

        return 0;
    }

    // .obj extension
    if(strcmp(lastDot, ".obj") == 0) {
        strcpy(bin_filename, filename);

        strcpy(sym_filename, filename);
        strcpy(strchr(sym_filename, '.'), ".sym");

        return 0;
    }

    // // .sym extension
    // if(strcmp(lastDot, ".sym") == 0) {
    //     bin_filename = NULL;

    //     strcpy(sym_filename, filename);

    //     return 0;
    // }

    return 1;
}


/**
 * @brief le_to_be: converts 2 byte litle endian to 2 byte big endian cause windows != lc3
 * 
 * @param uint16_t input: self explanatory
 * @return uint16_t flipped: flipped endian version of input
 */
__UINT16_TYPE__ le_to_be(__UINT16_TYPE__ input) {
    return ((input << 8) | (input >> 8)) & 0xFFFF;
}


/**
 * @brief get_sym_addr: takes symbol and returns corresponding address
 * 
 * @param CPU* lc3cpu
 * @param char* symName
 * @return uint16_t address (0 if symbol doesn't exist)
 */

__UINT16_TYPE__ get_sym_addr(CPU* lc3cpu, char* symName) {
    int i;
    for(i = 0; i < lc3cpu->numSymbols; i++) {
        if(strcmp(symName, lc3cpu->symbols[i]) == 0) {
            return lc3cpu->symAddr[i];
        }
    }

    return 0;
}

/**
 * @brief get_addr_sym: takes address and returns pointer to address name
 * 
 * @param CPU* lc3cpu
 * @param uint16_t addr
 * @return char* sym (NULL if address doesn't have symbol)
 */

char* get_addr_sym(CPU* lc3cpu, __UINT64_TYPE__ addr) {
    int i;
    for(i = 0; i < lc3cpu->numSymbols; i++) {
        if(addr == lc3cpu->symAddr[i]) {
            return lc3cpu->symbols[i];
        }
    }

    return 0;
}

/**
 * @brief dumpMem: takes addr and prints contents of address
 * 
 * @param CPU* lc3cpu
 * @param uint16_t addr
 * @return nothing
 */

void dumpMem(CPU* lc3cpu, __UINT16_TYPE__ addr) {
    printf("x%04hX: x%04hX\n", addr, lc3cpu->memory[addr]);
}


/**
 * @brief printregs: prints registers (R0-R7, PSR) and current instruction
 * 
 * @param CPU* lc3cpu
 * @return nothing
 */

void printregs(CPU* lc3cpu) {
    
    // Print PC + PSR
    __UINT16_TYPE__ PSR = lc3cpu->memory[0xFFFC];
    char cc;

    if(PSR & 0b100) {
        cc = 'n';
    } else if(PSR & 0b010) {
        cc = 'z';
    } else {
        cc = 'p';
    }

    printf("PC: x%04X   PSR: x%04X  (CC=%c)\n", lc3cpu->PC, PSR, cc);

    // Print R0-R7
    printf("R0: x%04hX   R1: x%04hX   R2: x%04hX   R3: x%04hX   \n", lc3cpu->regfile[0], lc3cpu->regfile[1], lc3cpu->regfile[2], lc3cpu->regfile[3]);
    printf("R4: x%04hX   R5: x%04hX   R6: x%04hX   R7: x%04hX   \n\n", lc3cpu->regfile[4], lc3cpu->regfile[5], lc3cpu->regfile[6], lc3cpu->regfile[7]);

    // TODO decode instruction
    printf("x%04hX   ", lc3cpu->PC);

    print_instr(lc3cpu, lc3cpu->PC);

    printf("\n");
}


/**
 * @brief print_instr: prints instruction at a memory address in human readable format
 * 
 * @param CPU* lc3cpu
 * @param uint16_t memAddr
 * @return nothing
 * 
 */

void print_instr(CPU* lc3cpu, __UINT16_TYPE__ memAddr) {
    __UINT16_TYPE__ instruction = lc3cpu->memory[memAddr];
    int opcode = instruction >> 12;

    if(instruction == 0) {
        printf("NOP\n");
        return;
    }

    // If BR
    if(opcode == 0) {
        __INT16_TYPE__ PCoffset9 = instruction & 0x1FF;
        // SEXT PCoffset9
        if(instruction & 0x100) {
            PCoffset9 |= 0xFE00;
        }

        char ccBits[4] = {0, 0, 0, 0};

        // n bit
        if(instruction & 0x0800) {
            strcat(ccBits, "n");
        }

        // z bit
        if(instruction & 0x0400) {
            strcat(ccBits, "z");
        }

        // p bit
        if(instruction & 0x0200) {
            strcat(ccBits, "p");
        }

        // Check if there is an associated symbol
        char* symbol = get_addr_sym(lc3cpu, memAddr + 1 + PCoffset9);
        if(symbol != NULL) {
            printf("BR%s %s\n", ccBits, symbol);
        } else {
            printf("BR%s #%d\n", ccBits, PCoffset9);
        }
        
        return;
    }

    // If ADD or AND
    if(opcode == 0b0001 || opcode == 0b0101) {
        // if instruction[5]
        if(instruction & 0b100000) {
            __INT16_TYPE__ imm5 = instruction & 0b11111;
            
            // SEXT imm5
            if(instruction & 0b10000) {
                imm5 |= 0xFFE0;
            }
            
            
            printf("%s R%1d, R%1d, #%d", INSTR[opcode], (instruction >> 9) & 0b111, (instruction >> 6) & 0b111, imm5);
        } else {
            printf("%s R%1d, R%1d, R%1d", INSTR[opcode], (instruction >> 9) & 0b111, (instruction >> 6) & 0b111, instruction & 0b111);
        }

        printf("\n");
        return;
    }

    // JMP
    if(opcode == 0b1100) {
        // if BaseR == R7
        if( ((instruction >> 6) & 0b111) == 7 ) {
            printf("RET\n");
        } else {
            printf("JMP R%d\n", (instruction >> 6) & 0b111);
        }

        return;
    }

    // JSR / JSRR
    if(opcode == 0b0100) {
        // If JSR
        if(instruction & 0x0800) {
            __INT16_TYPE__ PCoffset11 = instruction & 0x7FF;
            // SEXT PCoffset11
            if(instruction & 0x400) {
                PCoffset11 |= 0xF800;
            }

            // Check if there is an associated symbol
            char* symbol = get_addr_sym(lc3cpu, memAddr + 1 + PCoffset11);
            if(symbol != NULL) {
                printf("%s %s\n", INSTR[opcode], symbol);
            } else {
                printf("%s #%d", INSTR[opcode], PCoffset11);
            }
        } else {
            printf("JSRR R%d\n", (instruction >> 6) & 0b111);
        }
    }

    // LD, LDI, LEA, ST, STI
    if(opcode == 0b0010 || opcode == 0b1010 || opcode == 0b1110 || opcode == 0b0011 || opcode == 0b1011) {
        __INT16_TYPE__ PCoffset9 = instruction & 0x1FF;
        // SEXT PCoffset9
        if(instruction & 0x100) {
            PCoffset9 |= 0xFE00;
        }

        // Check if there is an associated symbol
        char* symbol = get_addr_sym(lc3cpu, memAddr + 1 + PCoffset9);
        if(symbol != NULL) {
            printf("%s R%1d, %s", INSTR[opcode], (instruction >> 9) & 0b111,  symbol);
        } else {
            printf("%s R%1d, #%d", INSTR[opcode], (instruction >> 9) & 0b111, PCoffset9);
        }

        printf("\n");
        return;
    }

    // LDR/STR
    if(opcode == 0b0110 || opcode == 0b0111) {
        __INT16_TYPE__ offset6 = instruction & 0b111111;
        // SEXT offset6
        if(instruction & 0x20) {
            offset6 |= 0xFFC0;
        }

        printf("%s R%1d, R%1d, #%d\n", INSTR[opcode], (instruction >> 9) & 0b111, (instruction >> 6) & 0b111, offset6);
        return;
    }

    // NOT
    if(opcode == 0b1001) {
        printf("NOT R%1d, R%1d\n", (instruction >> 9) & 0b111, (instruction >> 6) & 0b111);
        return;
    }

    // TRAP
    if(opcode == 0b1111) {
        __UINT8_TYPE__ trapV = instruction & 0xFF;

        switch(trapV) {
            case 0x20:
                printf("GETC\n");
                break;
            case 0x21:
                printf("OUT\n");
                break;
            case 0x22:
                printf("PUTS\n");
                break;
            case 0x23:
                printf("IN\n");
                break;
            case 0x24:
                printf("HALT\n");
                break;
            default:
                printf("TRAP x%02x\n", trapV);
                break;
        }

        return;
    }

    // RTI
    if(opcode == 0b1000) {
        printf("RTI\n");
        return;
    }
}


/**
 * @brief runCycle: Runs one cycle (fetch, decode, execute) on an CPU struct
 * 
 * @param lc3cpu: pointer to CPU struct
 * @return nothing
 */

void runCycle(CPU* lc3cpu) {

    if(lc3cpu == NULL) {
        printf("Null CPU pointer\n");
        return;
    }

    // Fetch
    lc3cpu->MAR = lc3cpu->PC;
    lc3cpu->PC++;
    lc3cpu->MDR = lc3cpu->memory[lc3cpu->MAR];
    lc3cpu->IR = lc3cpu->MDR;

    // Decode/Execute

    __INT16_TYPE__ PCoffset9 = lc3cpu->IR & 0x1FF;
    // SEXT PCoffset9
    if(PCoffset9 & 0x100) {
        PCoffset9 |= 0xFFFF << 9; // Fill upper bits with ones
    }

    __INT16_TYPE__ PCoffset11 = lc3cpu->IR & 0x7FF;
    // SEXT PCoffset11
    if(PCoffset11 & 0x400) {
        PCoffset11 |= 0xFFFF << 11; // Fill upper bits with ones
    }

    __INT16_TYPE__ offset6 = lc3cpu->IR & 0x3F;
    // SEXT offset6
    if(offset6 & 0x20) {
        offset6 |= 0xFFFF << 6;
    }

    __UINT8_TYPE__ DR = (lc3cpu->IR >> 9) & 0b111;

    // Read opcode and decide execution from there
    switch(lc3cpu->IR >> 12) {
        // ADD
        case 1:
            // if IR[5], imm5 add, else register add
            if(lc3cpu->IR & 0b100000) {
                __INT8_TYPE__ imm5 = lc3cpu->IR & 0b11111;

                // SEXT imm5
                if(imm5 & 0b10000) {
                    imm5 |= 0xFF << 5;
                }
                lc3cpu->regfile[DR] = lc3cpu->regfile[(lc3cpu->IR >> 6) & 0b111] + imm5;
            } else {
                lc3cpu->regfile[DR] = lc3cpu->regfile[(lc3cpu->IR >> 6) & 0b111] + lc3cpu->regfile[lc3cpu->IR & 0b111];
            }

            // Set CC
            setCC(lc3cpu);
            break;
        // AND
        case 5:
            // if IR[5], imm5 AND, else register AND
            if(lc3cpu->IR & 0b100000) {
                __INT16_TYPE__ imm5 = lc3cpu->IR & 0b11111;

                // SEXT imm5
                if(imm5 & 0b10000) {
                    imm5 |= 0xFF << 5;
                }
                lc3cpu->regfile[DR] = lc3cpu->regfile[(lc3cpu->IR >> 6) & 0b111] & imm5;
            } else {
                lc3cpu->regfile[DR] = lc3cpu->regfile[(lc3cpu->IR >> 6) & 0b111] & lc3cpu->regfile[lc3cpu->IR & 0b111];
            }

            // Set CC
            setCC(lc3cpu);
            break;
        //BR
        case 0:
            // AND CC condition with CC bits
            if(DR & lc3cpu->memory[0xFFFC] & 0b111) {
                lc3cpu->PC += PCoffset9;
            }
            break;
        // JMP
        case 12:
            lc3cpu->PC = lc3cpu->regfile[(lc3cpu->IR >> 6) & 0b111];
            break;
        // JSR
        case 4:
            
            lc3cpu->regfile[7] = lc3cpu->PC;
            lc3cpu->PC += PCoffset11;
            break;
        // TRAP
        case 15:{
            // TODO implement this, not doing yet since no OS
            __UINT8_TYPE__ trapV = lc3cpu->IR & 0xFF;
            if(trapV == 0x20) {
                char readBuff[BUFF_LEN];

                gets(readBuff);

                unsigned char tempChar;

                sscanf(readBuff, "%c", &tempChar); // Read character

                lc3cpu->regfile[0] = tempChar;

                // No overflow protection
                
            } else if (trapV == 0x21) {
                printf("%c", lc3cpu->regfile[0]);
            } else if(trapV == 0x22) {
                char string[100];
                
                int i = 0;
                while((string[i] = lc3cpu->memory[lc3cpu->regfile[0] + i])) {
                    i++;
                }

                printf("%s", string);
            } else if(trapV == 0x25) {
                // Clear clock bit to stop clock
                lc3cpu->memory[0xFFFE] &= 0x7FFF;
            }
            break;}
        // LD
        case 2:
            lc3cpu->MAR = lc3cpu->PC + PCoffset9;
            lc3cpu->MDR = lc3cpu->memory[lc3cpu->MAR];
            lc3cpu->regfile[DR] = lc3cpu->MDR;

            // Set CC
            setCC(lc3cpu);
            break;
        // LDI
        case 10:
            // SEXT PCoffset9
            if(PCoffset9 & 0x100) {
                PCoffset9 |= 0xFFFF << 9; // Fill upper bits with ones
            }

            lc3cpu->MAR = lc3cpu->PC + PCoffset9;
            lc3cpu->MDR = lc3cpu->memory[lc3cpu->MAR];
            lc3cpu->MAR = lc3cpu->MDR;
            lc3cpu->MDR = lc3cpu->memory[lc3cpu->MAR];
            lc3cpu->regfile[DR] = lc3cpu->MDR;

            // Set CC
            setCC(lc3cpu);
            break;
        // LDR
        case 6:
            lc3cpu->MAR = lc3cpu->regfile[((lc3cpu->IR >> 6) & 0b111) + offset6];
            lc3cpu->MDR = lc3cpu->memory[lc3cpu->MAR];
            lc3cpu->regfile[DR] = lc3cpu->MDR;

            // Set CC
            setCC(lc3cpu);
            break;
        // LEA
        case 14:
            lc3cpu->regfile[DR] = lc3cpu->PC + PCoffset9;
            setCC(lc3cpu);
            break;
        // NOT
        case 9:
            lc3cpu->regfile[DR] = ~ (lc3cpu->regfile[(lc3cpu->IR >> 6) & 0b111]);
            setCC(lc3cpu);
            break;
        // ST
        case 3:
            lc3cpu->MAR = lc3cpu->PC + PCoffset9;
            lc3cpu->MDR = lc3cpu->regfile[DR];
            lc3cpu->memory[lc3cpu->MAR] = lc3cpu->MDR;
            break;
        // STI
        case 11:
            lc3cpu->MAR = lc3cpu->PC + PCoffset9;
            lc3cpu->MDR = lc3cpu->memory[lc3cpu->MAR];
            lc3cpu->MAR = lc3cpu->MDR;
            lc3cpu->MDR = lc3cpu->regfile[DR];
            lc3cpu->memory[lc3cpu->MAR] = lc3cpu->MDR;
            break;
        // STR
        case 7:
            lc3cpu->MAR = lc3cpu->regfile[((lc3cpu->IR >> 6) & 0b111) + offset6];
            lc3cpu->MDR = lc3cpu->regfile[(lc3cpu->IR >> 9) & 0b111];
            lc3cpu->memory[lc3cpu->MAR] = lc3cpu->MDR;
            break;
    }
}


/**
 * @brief setCC: sets CC bits based on DR
 * 
 * @param lc3cpu: pointer to CPU struct
 * @return int current cc (-1 if N, 0 if Z, 1 if P)
 */

int setCC(CPU* lc3cpu) {
    __INT16_TYPE__* PSR = &(lc3cpu->memory[0xFFFC]);
    
    *PSR &= 0xFFFF << 3; // Clear CC bits

    // Set appropriate CC bit based on DR (IR[11:8])
    if(lc3cpu->regfile[(lc3cpu->IR >> 9) & 0b111] < 0) {
        *PSR |= 0b100;
        return -1;
    } else if(lc3cpu->regfile[(lc3cpu->IR >> 9) & 0b111] == 0) {
        *PSR |= 0b010;
        return 0;
    } else {
        *PSR |= 0b001;
        return 1;
    }
}

/**
 * @brief checkBreakPt: checks if CPU struct hit breakpoint
 * 
 * @param lc3cpu 
 * @return uint8_t address of breakpoint or 0 if not a breakpoint 
 */

__UINT8_TYPE__ checkBreakPt(CPU* lc3cpu) {
    for(int i = 0; i < lc3cpu->bIndex; i++) {
        if(lc3cpu->breakpoints[i] == lc3cpu->PC) {
            return lc3cpu->breakpoints[i];
        }
    }

    return 0;
}