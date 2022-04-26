typedef struct{
    __INT16_TYPE__ regfile[8];
    __INT16_TYPE__ memory[65536]; // 2^16
    __INT16_TYPE__ MDR;
    __UINT16_TYPE__ MAR;
    __UINT16_TYPE__ PC;
    __UINT16_TYPE__ IR;

    //Symbol table
} CPU;

CPU * create_CPU();

void destroy_CPU();

int load_binary(CPU* cpu, char* filename);

int parse_filename(char* filename, char* bin_filename, char* sym_filename);
__UINT16_TYPE__ le_to_be(__UINT16_TYPE__ input);