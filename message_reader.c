#include "message_slot.h"
#include <fcntl.h>      /* open */
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int main(int argc, char *argv[]){
    int fd;
    int len;
    char buffer[MESSAGE_LEN];
    if(argc!=3){
        perror("Bad Input");
        exit(1);
    }
    fd=open(argv[1],O_RDONLY);
    if(fd<0){
        perror("Couldnt open file");
        exit(1);
    }
    if(ioctl(fd,MSG_SLOT_CHANNEL,atoi(argv[2]))!=0){
        perror("Couldnt set id");
        exit(1);
    }
    len=read(fd,buffer,MESSAGE_LEN);

    if(len<0){
        perror("Couldnt read message");
        exit(1);
    }
    close(fd);
    if(write(STDOUT_FILENO,buffer,len)<0){
        perror("Couldnt print message");
        exit(1);
    }

    return(0);
}