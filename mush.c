#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#include "parseline.h"
#include "mush.h"



int main(int argc, char *argv[])
{
   int stagec;
   int concurrent;
   char cmd[CMD_MAX];
   char cmd_prev[CMD_MAX];
   struct stage stage[STAGE_MAX];
   FILE *f;
   struct sigaction sa;
   int normal;
   int history = 0;
   /* Set signal masks */
   sa.sa_handler = handler;
   sa.sa_flags = 0;
   sigemptyset(&sa.sa_mask);
   concurrent = 1;
   
   /* catch sigint */
   if (sigaction(SIGINT, &sa, NULL) < 0)
   {
      perror("sigaction");
      exit(1);
   }

   /* should only be ./mush */
   if (argc > 2)
   {
      printf("usage: %s [ script ]\n", argv[0]);
      exit(1);
   }

   /* input file validation */
   if(argc == 2 && (!(f = fopen(argv[1],"r")))){
         perror("File supplied is wrong");
         exit(1);
   } else {
      f = stdin;
   }

   /* main shell loop, exits with 'exit' in MUSH */
   while(FOREVER){
      memset(stage, 0, sizeof(stage));
      if (isatty(fileno(f))){
         if(isatty(STDOUT_FILENO)){
            printf("osh>");   
         }
      }

      /* handles empty MUSH line */
      if (!fgets(cmd, CMD_MAX, f)){
         if(errno == EINTR){
            printf("\n"); 
         } else {
            exit(0);
            break;
         }
      }

      cmd[strlen(cmd)-1] = '\0'; 
      if (!strcmp(cmd, "exit")){
         exit(0);
         return 1;
      }

      if (!strcmp(cmd, "!!")){
         if (history){
            strcpy(cmd, cmd_prev);
         }
         else{
            printf("No commands in history.\n");
            continue;
         }
      }
      strcpy(cmd_prev, cmd);
      history = 1;

      /* checks if parseline returns error */ 
      if ((stagec = parseline(cmd, stage, &concurrent)) < 1){
         continue;
      }
      /* shouldn't reach here */
      if(!stagec){
         return ERROR;
      }

      /* gets initial command in pipeline */


      if (!strcmp(stage[0].argv[0], "quit") && (stagec != 1 || stage[0].argc != 1)){
         normal =  ERROR;
      } 
      else if(!strcmp(stage[0].argv[0], "quit")){
         normal =  EXIT;
      }
      if (!strcmp(stage[0].argv[0], "cd") && (stagec != 1 || stage[0].argc != 2)){
         normal = ERROR;
      } 
      else if(!strcmp(stage[0].argv[0], "cd")){
         normal = CHDIR;
      }
      else{
         normal = MORE;
      }

      /* handles basic MUSH commands */
      while(1){
         if (normal == EXIT){
            exit(0);
            break;
         }
         else if (normal == CHDIR){
            if (chdir(stage[0].argv[1]) < 0){
               perror(stage[0].argv[1]);
            }
            break;
         }
         else if (normal == MORE){
            executeCommand(stage, stagec);
            break;
         }
         else{
            printf("Invalid command\n");
            break;
         }
      }
      
   }

   return 0;
}


void handler(int signum){
   int i;
   for (i = 0; i < 1; i++){
      if (signum == SIGINT){
         break;
      }
   }
}

int close_fd(struct stage* stage, int filenum){
   int i;
   for (i = 0; i < filenum; i++)
   {
      close(i[stage].infd);
      close(i[stage].outfd);
   }
   return i;
}

void executeCommand(struct stage *stage, int c){
   pid_t cpid[STAGE_MAX];
   int i;
   int j;
   int st;
   i = 0;

   while (i < c){
   
      /* creates child process for command */
      cpid[i] = fork();
      if (cpid[i] == 0) /* Child process */
      {
         char *argv[STAGE_MAX];

         for (j = 0; j < i[stage].argc; j++){
            argv[j] = i[stage].argv[j];
         }
         if (j == i[stage].argc){
               argv[j] = NULL;
         }

         /* checks for error in fd duplication */
         if (dup2(stage[i].infd, STDIN_FILENO) == ERROR)
         {
            perror(stage[i].cmd);
            exit(1);
         }
         if (dup2(stage[i].outfd, STDOUT_FILENO) == ERROR){
            perror(stage[i].cmd);
            exit(1);  
         }

         close_fd(stage, c);


         execvp(0[argv], argv); /*Execute */
         fprintf(stderr, "%s: command not found\n", argv[0]);
         exit(1);
      } /*Entering parent process */

      /*close files used in parent */
      close(i[stage].infd);
      close(i[stage].outfd);
      i++;
   }

   for (i = 0; i < c; i++){
      while (waitpid(cpid[i], &st, 0) < 0);
   }
   /* clear stdout */
   fflush(stdout);
}
