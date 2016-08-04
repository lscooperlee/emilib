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

#include <syslog.h>
#include <string.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include "emi.h"
#include "msg_table.h"
#include "shmem.h"
#include "emisocket.h"
#include "emi_semaphore.h"
#include "debug.h"
#include "emi_config.h"


#define coreprt(a) emiprt(a)

#define DAEMONIZE	0x00000001
#define SINGLIZE	0x00000002


extern int emi_core(struct emi_config *config);
extern void emi_release(void);

static int lock_fd=-1;

void sig_release(pid_t pid){
	emi_release();
	if(lock_fd>=0)
		close(lock_fd);
	exit(0);
}

void print_usage(void){
	printf("usage:emi_core [-d]\n");
	printf("	-d	run as a daemon\n");
}


int main(int argc,char **argv){
	struct sigaction sa;

	int option=0,opt;

	struct emi_config *config;

	while(*++argv!=NULL&&**argv=='-'){
		while((opt=*++*argv)!='\0'){
			switch(opt){
				case 'd':
					option|=DAEMONIZE;
					break;
				case 'h':
					print_usage();
					return 0;
				default:
					option=0;
			}
		}
	}

	config=get_config();
	if(config==NULL)
		config=emi_config;

	umask(0);

	if(option&DAEMONIZE){
		pid_t pid;
		if((pid=fork())<0){
			coreprt("process fork error\n");
			return -1;
		}else if(pid!=0){
			return 0;
		}
	}

	setsid();

	sa.sa_handler=sig_release;
	sa.sa_flags=0;
	sigfillset(&sa.sa_mask);
	if(sigaction(SIGTERM,&sa,NULL)<0){
		coreprt("sigaction error\n");
	}

	sa.sa_handler=sig_release;
	sa.sa_flags=0;
	sigfillset(&sa.sa_mask);
	if(sigaction(SIGINT,&sa,NULL)<0){
		coreprt("sigaction error\n");
	}

/*
 * in order to solve SIGPIPE issue , when write to a socket that has already been closed by remote program.
 *
 * Program received signal SIGPIPE, Broken pipe.
 */
	signal(SIGPIPE,SIG_IGN);
//	sa.sa_handler = SIG_IGN;
//	sigaction( SIGPIPE, &sa, 0 )

	if(chdir("/")<0){
		coreprt("can not changed dir to /\n");
	}

	return emi_core(config);
}
