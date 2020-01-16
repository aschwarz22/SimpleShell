#ifndef _MUSH_H
#define _MUSH_H


#define EXIT 2
#define CHDIR 1
#define MORE  0
#define ERROR -1
#define FOREVER 42
#define CMD_MAX 512
#define STAGE_MAX 10
#define ARGC_MAX 11

void executeCommand(struct stage*, int, int concurrent_argnums[]);
void handler(int );



#endif
