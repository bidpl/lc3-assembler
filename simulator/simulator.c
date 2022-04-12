#include <stdio.h>
#include <stdlib.h>
#include "simulator.h"

int main() {
    
}

/**
 * @brief: createCPU: Creates an instance of CPU struct and returns its pointer
 * @return: CPU* to new instance of CPU
 */
CPU * createCPU(){
    CPU* newCPU = malloc(sizeof(CPU));
}

/** 
 * @brief destroyCPU: Frees CPU struct from memory
 * 
 * @param CPU* cpu: pointer to CPU struct to be freed
 * @return nothing
 */
void destroyCPU(CPU* cpu) {
    free(cpu);
}

/**
 * @brief loadBinary: Tries to open a binary file and copy its contents to CPU's ram and sets PC.
 * Also tries to import a symbol table + cleaned code for preview.
 * 
 * @param char* filename: path to binary file
 * @return 0 on success, 1 if no symbol table, 2 if invalid binary filename
 */
int loadBinary(char* filename){

}