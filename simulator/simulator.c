#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simulator.h"

#define BUFF_LEN 127
#define PATH_BUFF_LEN 255
#define LAST_MEM_ADDR 0xFDFF

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
            
        } else if(strcmp(command, "quit") == 0 || strcmp(command, "q") == 0) {
            printf("Are you sure you want to quit? y to confirm: ");
            scanf("%s", command);

            if(strcmp(command, "y") == 0) {
                printf("Quitting...\n");
                break;
            } else {
                printf("Quit canceled\n");
                continue;
            }
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

    return newCPU;
}


/** 
 * @brief destroy_CPU: Frees CPU struct from memory
 * 
 * @param CPU* cpu: pointer to CPU struct to be freed
 * @return nothing
 */
void destroy_CPU(CPU* cpu) {
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
    printf("Starting at x%04x\n", readBuff);

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
        printf("x%04x: x%04x\n", memAddr, readBuff);
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

