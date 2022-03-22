#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

int addLabel(char* labelName, int addr);

int getLabel(char* labelName, int *addr);

void cleanLine(char *buff);

char getOpcode(char *input);

char getOpcodeFromInstr(char input[3][30], int *operandsIndex);

char beginWithList(char *input, const char *checklist[], int checklistLen);

int parseNum(char *input, int *parsedNum);

int parseInstr(char *instruction, int *output, int currentAddr);

int parseOperands(int *output, char *operands, int numOperands, int currentAddr, int immLen);

int parseRegister(char *inputString, int *registerNum);

const char *INSTR[] = {"BR", "ADD", "LD", "ST", "JSR", "AND", "LDR", "STR", "RTI", "NOT", "LDI", "STI", "JMP", "RET", "LEA", "TRAP", "JSRR"};

const char *TRAPV[] = {"GETC", "OUT", "PUTS", "IN", "PUTSP", "HALT"};
const char *ASMDIR[] = {".ORIG", ".FILL", ".BLKW", ".STRINGZ", ".END"};

//Symbol table implementation (BSed a map by using 2 arrays, we love O(n) access times). Hopefully good enough for these small usecases.
char *labels[255];
int label_addrs[255];
int nextLabel = 0;

int main(int argc, char *argv[]) {
    FILE *fp;
    FILE *wfp;
    char buff[255];

/*    char fileName[255];
    char outputName[255];
    char *ext;

// Get inputfile from command line input
    // Check for 1 arg (should only have input file as arg)
    if(argc == 1) {
        printf("Provide an lc3 file.\n");
        return 1;
    }

    if(argc > 2) {
        printf("Please provide only 1 lc3 file.\n");
        return 1;
    }
    
    // Copy 1st argument (should be input file) to fileName
    strcpy(fileName, argv[1]);
    
    //Check that its a .asm file
    ext = strrchr(fileName, '.');

    if(ext == NULL) {
        //If no file extension, append .asm
        strcat(fileName, ".asm");
    }else if(strcmp(ext, ".asm")) {
        printf("Need .asm file, given %s\n", fileName);
        return 1;
    }
    
    strcpy(outputName, fileName);
    strcat(outputName, "2");

    fp = fopen(fileName, "r");
    wfp = fopen(outputName, "w+");

*/
////

    fp = fopen("factorial.asm", "r");
    wfp = fopen("factorial.asm2", "w+");

    // Go through each line and copy instructions to .asm2 file
    while(!feof(fp)){
        fgets(buff, 255, fp);

        cleanLine(buff);

        int instructionID = getOpcode(buff);

        // Append buffer to file if not blank
        if(buff[0] != 0xA && buff[0] != 0){
            printf("%2d: %s", instructionID, buff);
            fputs(buff, wfp);
        }

        // Stop at .END directive
        if(instructionID == 20) {
            break;
        }
    }

    fclose(fp);
    fclose(wfp);

// Start Symbol Table generation
    fp = fopen("factorial.asm2", "r");

    int lineNumber = 0;

    // Read first line
    fgets(buff, 255, fp);
    
    // Make sure first line is .ORIG directive
    if(getOpcode(buff) != 16) {
        printf("Program doesn't start with .ORIG directive.\n");
        return -1;
    } else {
        // Read start address
        char startAddrStr[10];
        char dummy[1];
        int numRead = sscanf(buff, "%*s%s%1s", startAddrStr, dummy);

        // Check if .ORIG has exactly 1 input
        if(numRead == 0) {
            printf(".ORIG has no start address");
        } else if(numRead > 2) {
            printf("Too many arguments to .ORIG");
        }

        // Convert input address to int value, store into lineNumber
        numRead = parseNum(startAddrStr, &lineNumber);
        // Check for sucessful conversion
        if(numRead != 0) {
            printf("Bad .ORIG start address");
            return 1;
        }
    }

    // Go through each line and generate symbol table  
    
    while(!feof(fp)){
        // Get next line
        fgets(buff, 255, fp);

        //TODO implement .STRINGZ and .BLKW fill

        // If line starts with label
        if(getOpcode(buff) == -1) {
            //TODO fix problem with strcpy in addLabel() throwing seg fault
            char temp[25];
            sscanf(buff, "%s", temp);
            addLabel(temp, lineNumber);
        }

        lineNumber++;
    }

   fclose(fp);

    //Write symbol table
    //.sym header
    wfp = fopen("factorial.sym", "w+");
    fputs("// Symbol table\n", wfp);
    fputs("// Scope level 0:\n", wfp);
    fputs("//\tSymbol Name       Page Address\n", wfp);
    fputs("//\t----------------  ------------\n", wfp);
    
    for(int i = 0; i < nextLabel; i++) {
        //Write symbol to .sym file
        fprintf(wfp, "//\t%-16s  %x\n", labels[i], label_addrs[i]);
    }
    //Addition newline at end to match lc3as's .sym format
    fprintf(wfp, "\n");

    //Close .sym file
    fclose(wfp);

    //Reopen formatted asm file and make .obj file
    fp = fopen("factorial.asm2", "r");
    rewind(fp);
    wfp = fopen("factorial.obj", "w+");

    // Get first line (.ORIG), we've already made sure it's a .ORIG statement above
    fgets(buff, 255, fp);

    char startAddrStr[10];
    sscanf(buff, "%*s%s", startAddrStr);
    parseNum(startAddrStr, &lineNumber);

    fprintf(wfp, "Testing the output obj file, File start at x%x\n", lineNumber);

    while(!feof(fp) && getOpcode(buff) != 0x14){
        int instrBuff;

        // Get next line
        fgets(buff, 255, fp);

        int parseRet = parseInstr(buff, &instrBuff, lineNumber);

        if(parseRet == 0) {
            fprintf(wfp, "%x\n", instrBuff);
        } else if(parseRet == -1) {
            // Encountered .END so stop
            printf("Reached .END at mem addr %X\n", lineNumber);
            break;
        } else if(parseRet == -2) {
            // Encountered a .BLKW

            // Allocate that many blank words
            for(int i = 0; i < instrBuff; i++) {
                //TODO change this to 0 when printing bin
                fprintf(wfp, "x0000\n");
            }

            // Increment address counter (-1 since there is a lineNumber++ after the if)
            lineNumber += instrBuff - 1;
        }else {
            //TODO make better error
            printf("Some instruction didn't parse\n");
            fclose(fp);
            fclose(wfp);
            return 1;
        }

        lineNumber++;
    }

    fclose(fp);
    fclose(wfp);

    return 0;
}

void cleanLine(char *line) {
    // char instruction[255];


    // Deletes comments, extra whitespace, and turns tabs into a single space
    int lineCounter = 0;
    short lastSpace = 1;

    // Loop through every character in the line but the last character (newline)
    for(int i = 0; i < strlen(line) - 1; i++) {
        if(line[i] == ';') {
            break;
        } if((line[i] == ' ' && lastSpace) || (line[i] == 0x9 /*Tab character*/)) {
            continue;
        } if(line[i] == 0x9 /*Tab character*/) {
            line[lineCounter] = ' ';
            lineCounter++;
        } else {
            if(line[i] == ' ' || line[i] ==',') {
                lastSpace = 1;
            } else {
                lastSpace = 0;
            }

            line[lineCounter] = toupper(line[i]);
            lineCounter++;
        }
    }

    //If last character in line is space, overwrite it later
    if(lineCounter >= 1 && (line[lineCounter-1] == ' ' || line[lineCounter-1] == 0xA)){
        lineCounter--;
    }

    char buff1[255];
    char buff2[255];
    int numArgs = sscanf(line, "%s %s", buff1, buff2);

    //Add newline if not a lone label (at least 2 args, or doesn't begin with label), space if lone label
    if(numArgs == 2 || getOpcode(line) != -1) {
        line[lineCounter] = '\n'; // Add newline
    } else if(numArgs == 1) {
        line[lineCounter] = ' '; // Add space
    } else if(numArgs == -1) {
        line[lineCounter] = 0;
    }

    line[lineCounter+1] = 0; // Add null terminating character

    // *line = instruction;
}


// Input: *char input
// Return:
// -1 if label
// 0x00-0x0f if instruction (return opcode)
// 0x20-0x25 if trap assembler instruction (return the trap vector)
// 0x10-0x14 if assembler directive (.ORIG = x10, .FILL = x11, .BLKW = x12, .STRINGZ = x13, .END = x14)
char getOpcode(char *input) {
    char tempChar = beginWithList(input, INSTR, sizeof(INSTR)/sizeof(INSTR[0]));

    if(tempChar != -1) {
        return tempChar;
    }

    tempChar = beginWithList(input, TRAPV, sizeof(TRAPV)/sizeof(TRAPV[0]));

    if(tempChar != -1) {
        return tempChar + 0x20;
    }

    tempChar = beginWithList(input, ASMDIR, sizeof(ASMDIR)/sizeof(ASMDIR[0]));

    if(tempChar != -1) {
        return tempChar + 0x10;
    }

    return -1;
}


/* @brief getOpcodeFromInstr: Takes an array of strings (instruction) and returns the opcode. Also sets operands pointer to string containing the operands 
 * @param input[3] array of strings containing instructiong: Label Opcode Operands
 * @param operands To be set to operands string
 * @return opcode (see getOpcode for table) except -1 means invalid instruction
 */
char getOpcodeFromInstr(char input[3][30], int *operandsIndex) {
    // Check if instruction starts with label
    int lastChar = strlen(input[0]);

    // getOpcode requires terminating space
    input[0][lastChar] = ' ';
    input[0][lastChar + 1] = 0;
    int opcode = getOpcode(input[0]);
    // Remove the extra space
    input[0][lastChar] = 0;

    // If line starts with a label
    if(opcode == -1) {
        // Get opcode based on second argument (should be the instruction name)
        // Same ending space BS as above
        lastChar = strlen(input[1]);
        input[1][lastChar] = ' ';
        input[1][lastChar + 1] = 0;
        opcode = getOpcode(input[1]);
        input[1][lastChar] = 0;

        if(opcode == -1) {
            // If invalid opcode
            return -1;
        } else {
            *operandsIndex = 2;
            return opcode;
        }
    } else {
        // If line doesn't start with label, it should start with opcode
        //If there is NO label, the operands should be the 2nd argument
        *operandsIndex = 1;
        return opcode;
    }
}

/* @brief beginWithList: Takes a string and an array of lists. Checks if there is an element of array that starts the string (element of array is prefix of string).
 * @param input String to check
 * @param checklist Array of prefix strings
 * @return index of first prefix match, -1 if null checklist or no match
 */

char beginWithList(char *input, const char *checklist[], int checklistLen) {
    // Check if checklist is null
    if(checklist == 0) {
        return -1;
    }

    for(int i = 0; i < checklistLen; i++){
        int charPos = 0;
        int matchInstr = 1;

        // For every character in the instruction name
        while(checklist[i][charPos] != 0) {
            // Check if corresponding character in input doesn't match or is null
            if(input[charPos] == 0 || input[charPos] != checklist[i][charPos]) {
                matchInstr = 0;
                break;
            } else {
                charPos++;
            }
        }

        //Make sure that there is space or end of line after matching string
        // Hardcode edge case for BR, very bad practice (ruins this function for other uses)
        if(matchInstr && strcmp(checklist[i], "BR") == 0){
            matchInstr = 0;

            for(int j = 0; j < 4; j++) {
                if(input[charPos + j] == ' ' || input[charPos + j] == 0xA){
                    matchInstr = 1;
                    break;
                } else if(!(input[charPos + j] == 'N' || input[charPos + j] == 'Z' || input[charPos + j] == 'P')) {
                    break;
                }
            }

        } else if( !(input[charPos] == ' ' || input[charPos] == 0xA) ){
            matchInstr = 0;
        }

        if(matchInstr) {
            return i;
        }
    }

    return -1;
}

/* @brief addLabel: Takes a label (String) and lc3 memory address (int). Adds it to symbol table
 * @param labelName name of label
 * @param addr LC3 memory address to be associated with label
 * @return 0 if sucess, 1 if label was already taken
 */

int addLabel(char* labelName, int addr) {
    // TODO check if label already defined
    if(0) {
        return 1;
    }

    // Add label and addr to symbol table
    labels[nextLabel] = (char *)malloc(strlen(labelName) * sizeof(char)); // Nervous laugher I hope i don't mem leak here
    strcpy(labels[nextLabel], labelName);

    label_addrs[nextLabel] = addr;

    // Increment next index location
    nextLabel++;
    
    return 0;
}

/* @brief getLabel: Takes a label (String) and stores corresponding memory address in addr
 * @param labelName name of label
 * @param addr LC3 memory address associated with label
 * @return 0 if sucess, 1 if label not defined
 */
int getLabel(char* labelName, int *addr) {
    for(int i = 0; i < nextLabel; i++) {
        if(strcmp(labelName, labels[i]) == 0) {
            *addr = label_addrs[i];
            
            return 0;
        }
    }
    
    // If none of the keys match, label isn't defined
    return 1;
}

/* @brief parseNum: Takes an LC3 formatted literal number and returns it as int
 * @param input Literal input as string
 * @param parsedNum pointer to output variable
 * @return 0 for sucess, 1 for fail, 2 for fail for being too large value (LC3 is 16-bit)
 */
int parseNum(char *input, int *parsedNum) {
    int numRead;
    char dummy;
    //Check if dec (#) or hex (X), read input based on that
    if(input[0] == '#'){
        numRead = sscanf(input, "%*c%d%1s", parsedNum, &dummy);
    } else if(input[0] == 'X') {
        numRead = sscanf(input, "%*c%X%1s", parsedNum, &dummy); 
    } else {
        //Does not start with valid base marker
        return 1;
    }

    if(numRead != 1) {
        //Either no number or too many inputs
        //e.g. "x" or "x400 123"
        return 1;
    }

    // TODO implement too big num check
    if(0 /*check if parsedNum doens't fit in 16 bits*/) {
        return 2;
    }

    return 0;
}

/* @brief parseInstr: Takes an LC3 assembly instruction formated: "<label (optional)> <instr> <operands>"
 * @param instruction: assembly instruciton to be formatted
 * @param output: pointer to where formatted binary instruction should be outputted
 * @return 0 for sucess
 *         1 for bad instruction name
 *         2 for invalid operands
 *         3 if incorrect # of args in instruction
 *         4 if second .ORIG
 *        -1 for .END
 *        -2 for .BLKW (output will be set to .BLKW's arg)
 *        -3 for .STRINGZ
 */

int parseInstr(char *instruction, int *output, int currentAddr) {
    int operandsIndex;
    char *operands;
    // Parse the instruction into it's 2/3 components (either instr + operands or label + instr + operands)
    char arguments[3][30];
    char post[2];

    int numRead = sscanf(instruction, "%s%s%s%1s", arguments[0], arguments[1], arguments[2], post);

    // Get opcode ID and set operands pointer
    int opcode = getOpcodeFromInstr(arguments, &operandsIndex);

    // If bad name/can't find instruction name
    if(opcode == -1) {
        return 1;
    }

    // Check for correct number of inputs scanned in
    if(operandsIndex == 2) {
        // If operands is the 3rd argument, there is a label
        // This means we expect 2 (instructions w/o operands like RET) or 3 items scanned in

        if(numRead > 3 || numRead < 2) {
            return 3;
        }
    } else {
        // Otherwise there is no label
        // This means we expect 1 (instructions w/o operands like RET) or 2 items scanned in

        // Technically numRead < 1 will never be hit, it will be caught in getOpcodeFromInstr
        if(numRead > 2 || numRead < 1) {
            return 3;
        }
    }

    // Set operands to correct arg
    operands = arguments[operandsIndex];

    // Done with inital checks, now starting instruction specific checks/parsing
    //Parse Trap pseudo-instructions
    if(opcode >= 0x20 && opcode <= 0x25) {
        *output = 0xF000; // Sets the 1111 opcode for TRAP, TODO see if C is sext or zext
        *output += opcode; // Fill the trapvect8, Trap pseudo-op "opcode" is encoded as its offset
        
        return 0;
    }
    // Parse Directives
    int illegalInput;
    switch(opcode) {
        // x10 = .ORIG
        case 0x10:
            return 4;
        // x11 = .FILL
        case 0x11:
            //TODO let .FILL take a label
            illegalInput = parseNum(operands, output);

            if(illegalInput){
                return 2;
            }

            *output = *output;

            return 0;
        // x12 = .BLKW
        case 0x12:
            illegalInput = parseNum(operands, output);

            if(illegalInput){
                return 2;
            }

            return -2;
        // x13 = .STRINGZ
        // x14 = .END
        case 0x14:
            return -1;
    }

    // Parse Instructions
    if(opcode >= 0 && opcode <= 15) {
        *output = 0;
        //Temp code TODO implement
        if(opcode == 0) {
            //BR instruction

            
            parseOperands(output, operands, 1, currentAddr, 9);
            //TODO set NZP
        
        } else if(opcode == 1) {
            //TODO check for parseOperands return val
            parseOperands(output, operands, 3, currentAddr, 5);
        } else if(opcode == 2) {
            parseOperands(output, operands, 2, currentAddr, 9);
        }
        
        // Keep only lower 12 bits, upper ones should not be used yet
        *output &= 0xFFF;
        // Write the opcode (bits 16-13)
        *output |= opcode << 12;

        return 0;
    }

    return 0;
}

/* @brief parseOperands: Takes an LC3 assembly operand formated: "<operand0>,<operand1>,<operand2>,...", number of operands (ex 1 for BR, 3 for ADD), current mem address (for imm range checking), and immLen for (imm range checking)
 * @param operands: assembly operands to be formatted
 * @param numOperands: expected number of operands
 * @return 0 for sucess
 *         1 for imm out of bounds
 *         2 for illegal operand (invalid register, imm when expected register, etc)
 *         3 if incorrect # of operands
 *         4 for invalid inputs (should never happen)
 *        -1 if 1 operand and it's a register (usually 1 operand expects imm value)
 */

int parseOperands(int *output, char *operands, int numOperands, int currentAddr, int immLen) {
    // To store the individual operands, calling them val(ue)s
    char vals[3][20];
    char post[2];

    // Read in the vals/operands
    int numScanned = sscanf(operands, "%[^,]%*c%[^,]%*c%[^,]%*c%c", vals[0], vals[1], vals[2], post);
    
    // Check for the correct number of operands
    if(numScanned != numOperands) {
        return 3;
    }

    // TODO continue this function

    if(numOperands == 1) {
        // One operand, expect imm/label or 1 register
        int regNum;
        int retVal = parseRegister(vals[0], &regNum);

        // If it's a register
        if(retVal == 0) {
            //for 1 operand register opcodes (JSRR, JMP), 000 <baseR 3 bit> 000000 (in total 12 bits)
            // Clear out 12 LSB
            *output &= 0xF000; 
            // Set them to the register
            *output |= regNum << 6;
            return -1;
        } else if(retVal == -1) {
            // If invalid register number
            return 2;
        } else {
            // If label/imm val

            // Check if number
            int offset;
            int isLabel = parseNum(vals[0], &offset);

            // If it's a label, switch it out
            if(isLabel) {
                // Get address of label
                retVal = getLabel(vals[0], &offset);

                // If not a defined label, return illegal operand
                if(retVal == 1) {
                    return 2;
                }

                // Turn address into offset
                offset -= (currentAddr + 1);
            }

            // Check for imm size TODO check that I have correct checker
            if(offset < -1 << (immLen - 1) || offset > 1 << (immLen - 1) -1) {
                return 1;
            }

            // Clear out lower bits for imm val (using imm len)
            *output &= -1 << immLen;

            // Mask lower bits of offset (in case negative) and or it to output
            // Mask is to creat immLen number of 1s
            *output |= offset & ((1 << immLen) - 1);

            return 0;
        }
    } else if(numOperands == 2) {

    } else if(numOperands == 3) {

    } else {
        // Should only have to parse up to 3 
        return 4;
    }

    return 0;
}


/* @brief parseOperands: Takes null-terminated string and checks if it contains a register name. If so, store it into registerNum 
 * @param inputString: String containing register name eg. "R4" Needs to have no leading/trailing whitespace
 * @param registerNum: Where to store register number
 * @return 0 for sucess
 *         1 for not a register
 *         -1 for invalid register number
 */
int parseRegister(char *inputString, int *registerNum) {
    // If string doesn't start with 'R', not valid register name
    if(inputString[0] != 'R') {
        return 1;
    }

    // Check that every character after is a number
    // Make sure the string isn't super long (might be missing null termination)
    if(strlen(inputString) > 20) {
        return 1;
    }

    // Check that the characters are numbers
    for(int i = 1; i < strlen(inputString); i++) {
        if(!isdigit(inputString[i])) {
            return -1;
        }
    }

    // Read register number
    *registerNum = atoi(&inputString[1]);

    // Make sure it's a valid number
    if(*registerNum < 0 || *registerNum > 7) {
        return -1;
    }

    return 0;
}