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

    printf("\n\n===================================\nLC-3 Simulator by Binh-Minh Nguyen \nUIUC ECE220 SP22 Honors Project\nCompiled: %s %s\n===================================\n\n\n", __DATE__, __TIME__);

    // Initialize LC-3 struct
    CPU* lc3cpu = create_CPU();

    // Check if there are input file(s)
    if(argc >= 2) {
        // Try to load each input file
        int i;
        for(i = 1; i < argc; i++ ) {
            load_binary(lc3cpu, argv[i]);
        }
    }

    // Listen for commands loop
    while(1) {
        printf("LC3sim> ");
        gets(readBuff);

        int numRead = sscanf(readBuff, "%s %s %s", command, option1, option2);

        if(strcmp(command, "file") == 0 || strcmp(command, "f") == 0) {
            load_binary(lc3cpu, option1);
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
                printf("x%04X   ", memAddr + i);

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
                dumpMem(lc3cpu, memAddr);
            }
            
        } else if(strcmp(command, "printregs") == 0 || strcmp(command, "p") == 0) {
            // printf("Print registers and current instruction\n");
            printregs(lc3cpu);

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
        printf("Can't open file: %s\n", bin_filename);
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
    // printf("Starting at x%04x\n", readBuff);

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
        // printf("x%04x: x%04x\n", memAddr, readBuff);
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
    //     printf("%s: %04x\n", cpu->symbols[i], get_sym_addr(cpu, cpu->symbols[i]));
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
    printf("x%04X: x%04X\n", addr, lc3cpu->memory[addr]);
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

    printf("PC: x%04x   PSR: x%04X  (CC=%c)\n", lc3cpu->PC, PSR, cc);

    // Print R0-R7
    printf("R0: x%04X   R1: x%04X   R2: x%04X   R3: x%04X   \n", lc3cpu->regfile[0], lc3cpu->regfile[1], lc3cpu->regfile[2], lc3cpu->regfile[3]);
    printf("R4: x%04X   R4: x%04X   R6: x%04X   R7: x%04X   \n\n", lc3cpu->regfile[4], lc3cpu->regfile[5], lc3cpu->regfile[6], lc3cpu->regfile[7]);

    // TODO decode instruction
    printf("x%04X   ", lc3cpu->PC);

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
}