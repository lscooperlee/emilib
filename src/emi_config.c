#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "emi.h"
#include "emi_config.h"
#include "debug.h"
#include "emisocket.h"
#include "emiif.h"


/*global emi_config using default value,
 */
/**
 *some parameters may not suit for user configuration using config files.for example,emi_port should use a unique value especially for remote connection.another example is emi_data_size_per_msg,this parameter defines the max data size in each massage,if two sides use different parameter,it may cause data transmit error.so be causion with these parameters.

the obsolete parameter emi_max_data is defined for max data ,because new design append massage data space for each massage, so it will be not used again.

 */
static struct emi_config __emi_config={
	.emi_port=USR_EMI_PORT,
	.emi_data_size_per_msg=EMI_DATA_SIZE_PER_MSG,
	.emi_key=EMI_KEY,
};



struct emi_config *emi_config=&__emi_config;

#ifdef DEBUG
void debug_config(struct emi_config *config){
	printf("config->emi_port=%d\n",config->emi_port);
	printf("config->emi_data_size_per_msg=%d\n",config->emi_data_size_per_msg);
	printf("config->emi_key=%x\n",config->emi_key);
}
#endif

void set_default_config(struct emi_config *config){
	emi_config->emi_port=config->emi_port;
	emi_config->emi_data_size_per_msg=config->emi_data_size_per_msg;
	emi_config->emi_key=config->emi_key;
	return;
}

struct emi_config *get_config(void){
	int fd;
	int i,j;
	char *p;
	char stack[256];
	struct stat sb;
	char *addr;

	char *terms[]={"EMI_PORT",
					"EMI_DATA_SIZE_PER_MSG",
					"EMI_KEY"};
	char *name[]={"emi.conf","$HOME/.emi.conf","$HOME/emi.conf","/etc/emi.conf"};


	for(i=0;i<sizeof(name)/sizeof(char *);i++){
		if((fd=open(name[i],O_RDONLY))>0)
			break;
	}

	if(fd<0||(fstat(fd,&sb)<0)){
		return NULL;
	}

	if((addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0))==NULL){
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
			for(;*p!='\n';p++);
		}
		for(i=0;i<sizeof(terms)/sizeof(char *);i++){
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

struct emi_config *guess_config(){
	struct sk_dpr *sd;
	int ret;
	struct emi_config *config=NULL;
	struct emi_addr src_addr;

	if((sd=emi_open(AF_INET))==NULL){
		dbg("emi_open error\n");
		return NULL;
	}
	
	config=emi_config;
	emi_fill_addr(&src_addr,"127.0.0.1",config->emi_port);
	if((ret=emi_connect(sd,&src_addr,1))==0){
		goto OK;
	}

	emi_close(sd);
	return NULL;

OK:
	emi_close(sd);
	return config;
}
