#ifndef _PARSELINE_H
#define _PARSELINE_H

#define CMD_MAX 512
#define STAGE_MAX 10
#define ARGC_MAX 11

#define MAX_CMND_ARGS 512
#define MAX_PIPE_LEN 10
#define MAX_CMND_NUM 10

#define RD_END 0
#define WR_END 1

struct stage
{
   int argc;
   char cmd[CMD_MAX];
   char argv[CMD_MAX][ARGC_MAX];
   int infd;
   int outfd;
};

typedef struct input {
	char *str[MAX_CMND_ARGS]; 
	int len;
} input;

void initInput(char *str, input *in);
int LineErrCheck(input *in);

int parseline(char *line, struct stage *stg, int *concurrent, int concurrent_argnums[CMD_MAX]);

#endif
