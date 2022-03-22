#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *fp1 = fopen("factorial.obj2","r");
    FILE *fp2 = fopen("prog2.obj","r");

    int byteNum = 0;

    int byte1;
    int byte2;

    do{
        byte1 = fgetc(fp1);
        byte2 = fgetc(fp2);

        if(byte1 != byte2) {
            printf("\n\nDiff at %x: %x %x\n\n", byteNum, byte1, byte2);
        } else {
            printf("%02x ", byte1);
        }

        byteNum++;
    } while(byte1 != EOF && byte2 != EOF);

    printf("\nEnded at %x: %x %x\n", byteNum-1, byte1, byte2);

    // while(byte1 != EOF) {
    //     printf("%02x ", fgetc(fp1));
    // }

    // printf("\n");

    return 0;
}