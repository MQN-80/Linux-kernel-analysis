#include <unistd.h>
#include <stdio.h>
#include<stdlib.h>


int main(int argc, char *argv[]){
    int i;
    int j = 0;
    int k;
    int l,m = 0;
    char block[32];
    char buf[32];
    char *p = buf;
    char *lineSplit[32];
    for(i = 1; i < argc; i++){
        lineSplit[j++] = argv[i];
    }
    while( (k = read(0, block, sizeof(block))) > 0){
        for(l = 0; l < k; l++){
            if(block[l] == '\n'){
                buf[m] = 0;
                m = 0;
                lineSplit[j++] = p;
                p = buf;
                lineSplit[j] = 0;
                j = argc - 1;
                if(fork() == 0){
                    execl(argv[1], lineSplit);
                }                
                wait();
            }else if(block[l] == ' ') {
                buf[m++] = 0;
                lineSplit[j++] = p;
                p = &buf[m];
            }else {
                buf[m++] = block[l];
            }
        }
    }
    exit(0);
}
