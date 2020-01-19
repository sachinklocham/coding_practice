#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include<pthread.h>
#include <sys/wait.h>

#define CLIENT_SOCK_FILE "/tmp/cmd_client"
#define SERVER_SOCK_FILE "/tmp/cmd_server"

void *client_thread(void *args){

	char *cmd_buff = (char*)args;

	int client_fd;
	struct sockaddr_un addr;
	int ret;
	char buff[128];
	struct sockaddr_un from;
	int status = 1;
	int len;

	if ((client_fd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
		perror("[ cmd_client ] socket");
		status = 0;
	}

	if (status) {
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, CLIENT_SOCK_FILE);
		unlink(CLIENT_SOCK_FILE);
		if (bind(client_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
			perror("[ cmd_client ] bind");
			status = 0;
		}
	}

	if (status) {
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, SERVER_SOCK_FILE);
		if (connect(client_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
			perror("[ cmd_client ] connect");
			status = 0;
		}
	}

	if (status) {
		strcpy (buff, cmd_buff);
		if (send(client_fd, buff, strlen(buff)+1, 0) == -1) {
			perror("[ cmd_client ] send");
			status = 0;
		}
		printf ("[ cmd_client ] sent command : %s\n",cmd_buff);
	}

	if (status) {
		if ((len = recv(client_fd, buff, 128, 0)) < 0) {
			perror("[ cmd_client ] recv");
			status = 0;
		}
		printf ("[ cmd_client ] Execute:%s\n", 	buff);
	}

	if (client_fd >= 0) {
		close(client_fd);
	}

	if(!status) { // if socket creation or connect fails to server
		printf ("[ cmd_client ] Fallback :popen\n");
		FILE *fp = popen(cmd_buff,"r");
		WEXITSTATUS(pclose(fp));
	}

	unlink (CLIENT_SOCK_FILE);
	pthread_exit(NULL);
}


int system_cmd(char *cmd_buff) {

	int i = 0;
	pthread_t tid;
	if( pthread_create(&tid, NULL, client_thread, cmd_buff) != 0 ){
		printf("[ cmd_client ] Failed to create thread\n");
	}
	sleep(1);
	pthread_detach(tid);
	//pthread_join(tid,NULL);
	return 0;
}
