#include <stdio.h>
#include <string.h>
#include <ctype.h>

int addLabel(char* labelName, int addr);

void cleanLine(char *buff);

char isInstr(char *input);

char beginWithList(char *input, const char *checklist[], int checklistLen);

int parseNum(char *input, int *parsedNum);

const char *INSTR[] = {"BR", "ADD", "LD", "ST", "JSR", "AND", "LDR", "STR", "RTI", "NOT", "LDI", "STI", "JMP", "RET", "LEA", "TRAP", "JSRR"};

const char *TRAPV[] = {"GETC", "OUT", "PUTS", "IN", "PUTSP", "HALT"};
const char *ASMDIR[] = {".ORIG", ".FILL", ".BLKW", ".STRINGZ", ".END"};

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

        int instructionID = isInstr(buff);

        // Append buffer to file if not blank
        if(buff[0] != 0xA && buff[0] != 0){
            printf("%d: %s", instructionID, buff);
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
    
    // Make sure program starts with .ORIG
    if(isInstr(buff) != 16) {
        printf("Program doesn't start with .ORIG directive.\n");
        return -1;
    } else {
        char startAddrStr[10];
        char dummy[1];
        int numRead = sscanf(buff, "%*s%s%1s", startAddrStr, dummy);

        if(numRead == 0) {
            printf(".ORIG has no start address");
        } else if(numRead > 2) {
            printf("Too many arguments to .ORIG");
        }

        //TODO implement getting start addr
        numRead = parseNum(startAddrStr, &lineNumber);
        if(numRead != 0) {
            printf("Bad .ORIG start address");
            return 1;
        }
    }

    // Go through each line and generate symbol table  
    
    while(!feof(fp)){
        // Get next line
        fgets(buff, 255, fp);

        char firstArg[100];
        sscanf(buff, "%s", firstArg);

        //TODO implement .STRINGZ and .BLKW fill

        // If line starts with label
        if(isInstr(buff) == -1) {
            //TODO fix problem with strcpy in addLabel() throwing seg fault
            //addLabel(firstArg, lineNumber);
        }

        lineNumber++;
    }

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
    if(numArgs == 2 || isInstr(line) != -1) {
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
char isInstr(char *input) {
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
    strcpy(labels[nextLabel], labelName);
    label_addrs[nextLabel] = addr;

    // Increment next index location
    nextLabel++;
    
    return 0;
}

/* @brief parseNum: Takes an LC3 formatted literal number and returns it as int
 * @param input Literal input as string
 * @param parsedNum pointer to output variable
 * @return 0 for sucess, 1 for fail
 */
int parseNum(char *input, int *parsedNum) {
    int numRead;
    char dummy;
    //Check if dec (#) or hex (X)
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

    return 0;
}