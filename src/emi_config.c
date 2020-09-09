#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "emi_msg.h"
#include "emi_config.h"
#include "emi_sock.h"
#include "emi_if.h"


/*global emi_config using default value,
 */
/**
 *some parameters may not suit for user configuration using config files. For example,emi_port should use a unique value especially for remote connection.

 */
static struct emi_config __emi_config={
    .emi_port=USR_EMI_PORT,
};



struct emi_config *emi_config=&__emi_config;

void set_default_config(struct emi_config *config){
    emi_config->emi_port=config->emi_port;
}

struct emi_config *get_config(void){
    int fd;
    unsigned int i,j;
    char *p;
    char stack[256];
    struct stat sb;
    char *addr;

    const char *terms[]={
                        "EMI_PORT",
                        };
    const char *name[]={"emi.conf","$HOME/.emi.conf","$HOME/emi.conf","/etc/emi.conf"};


    for(i=0; i<sizeof(name)/sizeof(char *); i++){
        if((fd=open(name[i],O_RDONLY))>0)
            break;
    }

    if(fd<0||(fstat(fd,&sb)<0)){
        return NULL;
    }

    if((addr = (char *)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0))==NULL){
        return NULL;
    }

    struct emi_config *config=(struct emi_config *)malloc(sizeof(struct emi_config));
    if(config==NULL){
        munmap(addr,sb.st_size);
        close(fd);
        return NULL;
    }

    for(p=addr;p-addr<sb.st_size;p++){
        if(*p=='#'||*p=='\n'){
            for(; *p!='\n'; p++);
        }
        for(i=0; i<sizeof(terms)/sizeof(char *); i++){
            if(!strncmp(terms[i],p,strlen(terms[i]))){
                p=p+strlen(terms[i]);
                memset(stack,0,sizeof(stack));
                for(j=0;*p!='#'&&*p!='\n';p=(*p=='='?p+1:p),stack[j++]=*p++);
                *((int *)(config)+i)=atoi(stack);
            }
        }
    }
    munmap(addr,sb.st_size);
    close(fd);

    return config;
}


eu32 get_pid_max(void){
    int fd,i;
    char buf[8]={0};
    if((fd=open("/proc/sys/kernel/pid_max",O_RDONLY))<0){
        goto error;
    }
    if(read(fd,buf,sizeof(buf))<0){
        close(fd);
        goto error;
    }
    close(fd);
    i=atoi(buf);
    return i;

error:
    perror("dangerous!it seems your system does not mount the proc filesystem yet,so can not get your pid_max number.emi_core would use default ,but this may be different with the value in your system,as a result,may cause incorrect transmission");
    return 32768;
}

