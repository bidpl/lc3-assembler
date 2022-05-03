#define MAX_SYMBOLS 255

typedef struct{
    __INT16_TYPE__ regfile[8];
    __INT16_TYPE__ memory[65536]; // 2^16
    __UINT16_TYPE__ MDR;
    __UINT16_TYPE__ MAR;
    __UINT16_TYPE__ PC;
    __UINT16_TYPE__ IR;

    //Symbol table
    int numSymbols;

    char* symbols[MAX_SYMBOLS];
    __UINT16_TYPE__ symAddr[MAX_SYMBOLS];
    __UINT16_TYPE__ breakpoints[MAX_SYMBOLS];
    int bIndex;
} CPU;

CPU * create_CPU();

void destroy_CPU();

int load_binary(CPU* cpu, char* filename);

int parse_filename(char* filename, char* bin_filename, char* sym_filename);
__UINT16_TYPE__ le_to_be(__UINT16_TYPE__ input);
__UINT16_TYPE__ get_sym_addr(CPU* lc3cpu, char* symName);
char* get_addr_sym(CPU* lc3cpu, __UINT64_TYPE__ addr);
void dumpMem(CPU* lc3cpu, __UINT16_TYPE__ addr);
void printregs(CPU* lc3cpu);
void print_instr(CPU* lc3cpu, __UINT16_TYPE__ memAddr);
void runCycle(CPU* lc3cpu);
int setCC(CPU* lc3cpu);
__UINT8_TYPE__ checkBreakPt(CPU* lc3cpu);