typedef struct{
    __INT16_TYPE__ regfile[8];
    __INT16_TYPE__ memory[65536]; // 2^16
    __INT16_TYPE__ MDR;
    __UINT16_TYPE__ MAR;
    __UINT16_TYPE__ PC;
    __UINT16_TYPE__ IR;

    //Symbol table
} CPU;

CPU * createCPU();

void destroyCPU();

int loadBinary(char* filename);