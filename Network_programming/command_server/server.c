#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

#define CLIENT_SOCK_FILE "/tmp/cmd_client"
#define SERVER_SOCK_FILE "/tmp/cmd_server"
#define MAXPENDING 25

#define SHELL_PATH              "/bin/sh"
#define SHELL_NAME              "sh"
#define SYSCMD_P_ERROR          (0xEF)

//#define NEED_COMMND_OUTPUT

#ifdef NEED_COMMND_OUTPUT

int cmd_fork_n_exec(const char *pBuff)
{
    int error = 0;
    pid_t child;
    int status;
    char cmd_output[1024];
    char *args[2];
    printf("[ cmd_server ]%s: cmd_fork_n_exec Entry PIPE\n",__FUNCTION__);
    child = fork();
    printf("[ cmd_server ]%s: cmd_fork_n_exec child pid [%d]\n",__FUNCTION__,child);
    if (child == 0) {
        /* In child process */
        printf("[ cmd_server ]%s: cmd_fork_n_exec Entry in child process\n",__FUNCTION__);
        args[0] = (char *)pBuff;
        args[1] = 0;
        memset(cmd_output,0,1024);
        FILE *fp = popen(args[0],"r");
        if(fp == NULL){
        	errno = SYSCMD_P_ERROR;
        }
        else{
        	fgets(cmd_output, sizeof(cmd_output), fp);//currently returning 1  line 
        	// while(fgets(cmd_output, sizeof(cmd_output), fp) != NULL) {//line by line
        	// 	printf("%s",cmd_output);//command output complete
        	// }
	        //write output to some buffer and send back to client
	        WEXITSTATUS(pclose(fp));

	        printf("[ cmd_server ]%s: cmd_fork_n_exec Exit from child process cmd_output :: %s \n",__FUNCTION__,cmd_output);//will not execute after execl
    	}
    }
    else if (child > 0) {
        /* Parent process waiting for child */
        printf("[ cmd_server ]%s: cmd_fork_n_exec Entry parent process\n",__FUNCTION__);

        printf("[ cmd_server ]%s: cmd_fork_n_exec Exit from parent process\n\n\n",__FUNCTION__);
    }
    else {
        /* Parent failed to fork */
        printf("[ cmd_server ]%s: local Fork failed %s\n",__FUNCTION__, strerror(errno));
    }

    return error;
}

#else

int cmd_fork_n_exec(const char *pBuff)
{
    int error = 0;
    pid_t child;
    int status;
    char *args[2];
    printf("[ cmd_server ]%s: cmd_fork_n_exec Entry execl\n",__FUNCTION__);
    child = vfork();
    printf("[ cmd_server ]%s: cmd_fork_n_exec pid [%d]\n",__FUNCTION__,child);
    if (child == 0) {
        /* In child process */
        printf("[ cmd_server ]%s: cmd_fork_n_exec Entry child \n",__FUNCTION__);
        args[0] = (char *)pBuff;
        args[1] = 0;

        if (execl(SHELL_PATH,SHELL_NAME,"-c",args[0],(char *) 0) < 0 ) {
            printf("[ cmd_server ]%s: Error in local execl (%s)\n",__FUNCTION__, strerror(errno));
            error = SYSCMD_P_ERROR;
            //exit(-1);
        }

        printf("[ cmd_server ]%s: cmd_fork_n_exec Exit child \n",__FUNCTION__);//will not execute after execl succ
    }
    else {
    	wait(&child);
    }

    return error;
}

#endif

void *server_thread(void *args){

	int client_fd = *((int *)args);
	char buff[128];
	int error = 0;
	int rc;
	while ( (rc=read(client_fd,buff,sizeof(buff))) > 0) {
	   	buff[rc] = 0x00;
		printf("[ cmd_server ]read %u bytes: %.*s\n", rc, rc, buff);
		error = cmd_fork_n_exec(buff); /* try vfork and exec */
		if (error == SYSCMD_P_ERROR){
			printf("\n[ cmd_server ] >> fallback to default system \n");
			error = system(buff);
		}/* fallback to default system */
		memset(buff,0,sizeof(buff));
		//copy command output back to client
		strcpy (buff, "ok");
		rc = write(client_fd, buff, sizeof(buff));
    }

    if (rc == -1) {
		perror("[ cmd_server ]read/write");
		exit(-1);
	}
	else if (rc == 0) {
		printf("\n[ cmd_server ] EOF closing client\n");
		close(client_fd);
	}
	pthread_exit(NULL);

}

int main() {
	int error = SYSCMD_P_ERROR;
	int server_fd, client_fd, rc;
	struct sockaddr_un addr;
	int ret;
	struct sockaddr_un from;
	int status = 1;
	int len;
	socklen_t fromlen = sizeof(from);

	if ((server_fd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
		perror("[ cmd_server ]socket");
		status = 0;
	}

	if (status) {
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, SERVER_SOCK_FILE);
		unlink(SERVER_SOCK_FILE);
		if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
			perror("[ cmd_server ]bind");
			status = 0;
		}
	}
	if (listen(server_fd, MAXPENDING) == -1) {
		perror("[ cmd_server ]listen error");
		exit(-1);
	}

    pthread_t tid;

	while (1) {

	    if ( (client_fd = accept(server_fd, NULL, NULL)) == -1) {
	      perror("[ cmd_server ]accept error");
	      continue;
	    }

		if( pthread_create(&tid, NULL, server_thread, &client_fd) != 0 )
           printf("[ cmd_server ]Failed to create thread\n");
       	else
       		printf("[ cmd_server ]success create thread\n");

        pthread_detach(tid);
	}
	if (server_fd >= 0) {
		close(server_fd);
	}
	return 0;

}
