#include <stdio.h>
#include <string.h>
#include <ctype.h>

void cleanLine(char *buff);

char isInstr(char *input);

char beginWithList(char *input, const char *checklist[], int checklistLen);

const char *INSTR[] = {"BR", "ADD", "LD", "ST", "JSR", "AND", "LDR", "STR", "RTI", "NOT", "LDI", "STI", "JMP", "RET", "LEA", "TRAP", "JSRR"};

const char *TRAPV[] = {"GETC", "OUT", "PUTS", "IN", "PUTSP", "HALT"};
const char *ASMDIR[] = {".ORIG", ".FILL", ".BLKW", ".STRINGZ", ".END"};

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

        // Append buffer to file if not blank
        if(buff[0] != 0xA){
            printf("%d: %s", isInstr(buff), buff);
            fputs(buff, wfp);
        }  
    }

    fclose(fp);
    fclose(wfp);

    fp = fopen("factorial.asm2", "r");

    // Go through each line and check if it has a label
    int lineNumber = 0;
    char currentLine[255];
    while(!feof(fp)){
        fgets(buff, 255, fp);

        
    }

    return 0;
}

void cleanLine(char *line) {
    // char instruction[255];


    // Deletes comments, extra whitespace, and turns tabs into a single space
    int lineCounter = 0;
    short lastSpace = 1;

    for(int i = 0; i < strlen(line); i++) {
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

    //Add newline if not a lone label (at least 2 args, or doesn't begin with label)
    if(numArgs == 2 || isInstr(line) != -1) {
        line[lineCounter] = 0xA; // Add newline 
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
