#ifndef _MUSH_H
#define _MUSH_H


#define EXIT 2
#define CHDIR 1
#define MORE  0
#define ERROR -1
#define FOREVER 42

void executeCommand(struct stage*, int);
void handler(int );



#endif
