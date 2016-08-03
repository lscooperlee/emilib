/*
EMI:	embedded message interface
Copyright (C) 2009  Cooper <davidontech@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see http://www.gnu.org/licenses/.
*/



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <emiif.h>


int func1(struct emi_msg *tmp){
	printf("1:msg is %d,while cmd is %d\n",tmp->msg,tmp->cmd);
	sleep(2);
//	emi_msg_decode_aes(tmp,"hello");
	debug_msg(tmp);
}
int func2(struct emi_msg *tmp){
	printf("2:msg is %d,while cmd is %d\n",tmp->msg,tmp->cmd);
}

int func3(struct emi_msg *tmp){
	printf("3:msg is %d,while cmd is %d\n",tmp->msg,tmp->cmd);
	sleep(2);
	tmp->size=3;
	memcpy(tmp->data,"oll",tmp->size);
	return 0;
}

#define FUNC1MSG	0x00000001
#define FUNC2MSG	0x00000002
#define FUNC3MSG	0x00000003

#define CMD1		0x00000003
#define CMD2		0x00000004
#define CMD3		0x00000005


int main(int argc,char **argv){

	emi_init();

	char opt;
	int option=-1;
	int ret;
	int size=0;
	unsigned int msg;
	while(*++argv!=NULL&&**argv=='-'){
		while((opt=*++*argv)!='\0'){
			switch(opt){
				case '1':
					option=0;
					break;
				case '2':
					option=1;
					break;
				case '3':
					option=2;
					break;
				default:
					option=-1;
			}
		}
	}

	if(option==-1){
		printf("usage: emi_test -1/-2/-3\n");
		return 0;
	}
	if(option==0){
		struct emi_msg *msg1;

		if((ret=emi_msg_register(FUNC1MSG,func1))<0){
			printf("emi_msg_register error\n");
			return -1;
		}

		if((msg1=emi_obtain_msg("127.0.0.1","abcdefg",strlen("abcdefg"),1,FUNC1MSG,EMI_MSG_FLAG_BLOCK))==NULL){
			printf("emi_obtain_msg error\n");
			return -1;
		}
		
		unsigned int input=0;
				getchar();
//				emi_msg_encode_aes(msg1,"hello");
				if((ret=emi_msg_send(msg1))<0){
					printf("emi_send_msg error\n");
					return -1;
				}
				debug_msg(msg1);
				pause();
				printf("recieved\n");
				
	}
	if(option==1){
		struct emi_msg *msg2;

	msg=FUNC2MSG;
	if((ret=emi_msg_register(msg,func1))<0){
		printf("emi_msg_register error\n");
		return -1;
	}

	msg=FUNC1MSG;
	if((ret=emi_msg_register(msg,func2))<0){
		printf("emi_msg_register error\n");
		return -1;
	}

		if((msg2=emi_msg_alloc(size))==NULL){
			printf("emi_msg_alloc error\n");
			return -1;
		}
		
		if((ret=emi_fill_msg(msg2,"127.0.0.1",NULL,CMD2,FUNC2MSG,EMI_MSG_TYPE_CMD|EMI_MSG_FLAG_NONBLOCK))<0){
			printf("emi_fill_msg error\n");
			return -1;
		}
		unsigned int input;
		for(;;){
			getchar();
			sleep(1);
//			for(j=0;j<input-1;j++){
				if((ret=emi_msg_send(msg2))<0){
					printf("emi_send_msg error\n");
					return -1;
//				}
			}
		}
	}
	if(option==2){
		struct emi_msg *msg;

		if((ret=emi_msg_register_blockreturn(FUNC3MSG,func3))<0){
			printf("emi_msg_register error\n");
			return -1;
		}

		if((msg=emi_msg_alloc(size))==NULL){
			printf("emi_msg_alloc error\n");
			return -1;
		}
		
		if((ret=emi_fill_msg(msg,"127.0.0.1","hello",CMD3,FUNC3MSG,EMI_MSG_TYPE_CMD|EMI_MSG_FLAG_BLOCK_RETURN))<0){
			printf("emi_fill_msg error\n");
			return -1;
		}

		unsigned int input;
		for(;;){
			printf("press any key:\n");
			getchar();
			if((ret=emi_msg_send(msg))<0){
				printf("emi_send_msg error\n");
				return -1;
			}
			printf("%s\n",msg->data);
			sleep(1);
		}
	}
}
