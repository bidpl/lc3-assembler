#include <stdio.h>
#include <string.h>

const char *INST[] = {"ADD", "AND", "BR", "JMP", "JSR", "JSRR", "LD", "LDI", "LDR", "LEA", "NOT", "RET", "RTI", "ST", "STI", "STR", "TRAP"};

const char *TRAPV[] = {"GETC", "OUT", "PUTS", "IN", "PUTSP", "HALT"};

char beginWithList(char* input, char* checklist[], int checklistLen);

int main() {
    char input[255] = "TRAP R1,R2,R3";
    if(input[0] == '.') {
        return 3;
    }

    printf("%s, %d", input, beginWithList(input, TRAPV, sizeof(TRAPV)/sizeof(TRAPV[0])));

    return 0;
}

/*
 * @brief beginWithList: Takes a string and an array of lists. Checks if there is an element of array that starts the string (element of array is prefix of string).
 * @param input String to check
 * @param checklist Array of prefix strings
 * @return index of first prefix match, -1 if null checklist or no match
 */

char beginWithList(char *input, char *checklist[], int checklistLen) {
    // Check if checklist is null
    if(checklist == 0) {
        return -1;
    }

    for(int i = 0; i < checklistLen; i++){
        char charPos = 0;
        char matchInstr = 1;

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

        if(matchInstr) {
            return i;
        }
    }

    return -1;
}