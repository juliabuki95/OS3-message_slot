#include "message_slot.h"
#include <fcntl.h>      /* open */
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[]){
    int fd;
    if(argc!=4){
        perror("Bad Input");
        exit(1);
    }
    fd=open(argv[1],O_WRONLY);
    if(fd<0){
        perror("Couldnt open file");
        exit(1);
    }
    if(ioctl(fd,MSG_SLOT_CHANNEL,atoi(argv[2]))!=0){
        perror("Couldnt set id");
        exit(1);
    }
    if(write(fd,argv[3],strlen(argv[3]))!= strlen(argv[3])){
        perror("Couldnt write message");
        exit(1);
    }
    close(fd);
    return 0;
}

