#include <stdio.h>
#include <string.h>

void cleanLine(char *buff);

char isInstr(char *input);

const char checklist[17][5] = {"ADD", "AND", "BR", "JMP", "JSR", "JSRR", "LD", "LDI", "LDR", "LEA", "NOT", "RET", "RTI", "ST", "STI", "STR", "TRAP"};

const char TRAPV[6][6] = {"GETC", "OUT", "PUTS", "IN", "PUTSP", "HALT"};

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
            printf("%s", buff);
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
        } if((line[i] == ' ' && lastSpace) || (line[i] == 0x9 /*Tab character*/) ) {
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

            line[lineCounter] = line[i];
            lineCounter++;
        }
    }

    //If last character in line is space, overwrite it later
    if(lineCounter >= 1 && (line[lineCounter-1] == ' ' || line[lineCounter-1] == 0xA)){
        lineCounter--;
    }

    line[lineCounter] = 0xA; // Add newline 
    line[lineCounter+1] = 0; // Add null terminating character

    // *line = instruction;
}

// Input: *char input
// Return:
// 0 if label
// 1 if instruction
// 2 if trap assembler instruction
// 3 if assembler directive
char isInstr(char *input) {
    if(input[0] == '.') {
        return 3;
    }

    for(int i = 0; i < sizeof(checklist); i++){

    }

    return 0;
}
