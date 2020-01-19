#include <stdio.h>

extern int system_cmd(char *cmd_buff);

int main(){
	char cmd[128] = {0};
	system_cmd("ifconfig");
	system_cmd("ls");
	system_cmd("mkdir -p tmp");
	system_cmd("ls");
	system_cmd("rm -rf tmp");
	return 0;
}
