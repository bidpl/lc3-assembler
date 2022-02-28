#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    char fileName[255];
    char *ext;

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
    
    strcat(fileName, "2");
    printf("%s\n", fileName);
}