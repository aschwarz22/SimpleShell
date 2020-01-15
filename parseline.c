#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "parseline.h"

int parseline(char *line, struct stage *stg, int *concurrent, int concurrent_argnums[CMD_MAX])
{
   int filedes[2];
   int c;
   char *cmdarg;
   char *arg;
   char *stgdup;
   char *argdup;
   input fullString;
   char str2[512];
   strncpy(str2, line, 512);
   initInput(str2, &fullString);
   
   if(LineErrCheck(&fullString)<0){
      return -1;
   }

   memset(stg, 0, sizeof(struct stage) * STAGE_MAX);
   cmdarg = strtok_r(line, "|", &stgdup);
   c = 0;
   while (cmdarg != NULL)
   {
      strcpy(stg[c].cmd, cmdarg);

      if (c != 0)
      {
         pipe(filedes);
         stg[c-1].outfd = filedes[WR_END];
         stg[c].infd = filedes[RD_END];
      }
 
      arg = strtok_r(cmdarg, " ", &argdup);

      stg[c].argc = 0;
      while (arg != NULL)
      {
         if (!strcmp(arg, "&"))
         {
            *concurrent = 1;
         }
  
         if (!strcmp(arg, "<") && (!(arg = strtok_r(NULL, " ", &argdup)) || 
                index(stg[c].cmd, '<') != rindex(stg[c].cmd, '<') ||
                !strcmp(arg, ">")))
         {
            fprintf(stderr, "%s: bad output redirection\n", stg[c].cmd);
            return -1;

         if ((stg[c].infd = open(arg, O_RDONLY)) < 0)
            {
               perror(arg);
               return -1;
            }

         }
         else if (strcmp(arg, ">") == 0)
         {
            if (!(arg = strtok_r(NULL, " ", &argdup)) ||
                index(stg[c].cmd, '>') != rindex(stg[c].cmd, '>') ||
                !strcmp(arg, "<"))
            {
               fprintf(stderr, "%s: bad input redirection\n", stg[c].cmd);
               return -1;
            }
            if ((stg[c].outfd = open(arg, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR
               | S_IWUSR)) < 0)
            {
               perror(arg);
               return -1;
            }
         }
         else 
            strcpy(stg[c].argv[stg[c].argc++], arg);

         arg = strtok_r(NULL, " ", &argdup);
      }
      
      cmdarg = strtok_r(NULL, "|", &stgdup);
      c++;
   }

   if (!stg[0].infd){
      stg[0].infd = dup(STDIN_FILENO);
   }
   if (!stg[c-1].outfd){
      stg[c-1].outfd = dup(STDOUT_FILENO);
   }

   return c;
}

int LineErrCheck(input *in) {
   int stages = 0;
   int argcs = 0;
   int AlligatorIn = 0; 
   int AlligatorOut = 0; 
   char * cmd;
   int i;

   if(!in->str[0]){
      return -1;
   }
   for (i = 0; i < in->len; i++) {

      if (!argcs) {

         if (in->str[i][0] == '|') {
            fprintf(stderr, "Invalid null command.\n"); 
            return -1;
         }

         else if(in->str[i][0] == '<'){
            fprintf(stderr, "Invalid null command.\n"); 
            return -1;
         }

         else if (in->str[i][0] == '>'){
            fprintf(stderr, "Invalid null command.\n"); 
            return -1;
         }

         else {
            if (++stages > MAX_PIPE_LEN) { 
               fprintf(stderr, "Pipeline too deep.\n");
            return -1;
            }

         }

      cmd = in->str[i];
      argcs += 1;

      }

      else if (in->str[i][0] == '|'){
         if (AlligatorOut == 2){
            fprintf(stderr, "%s: Ambiguous Output\n", cmd);
            return -1;
         }
         else{
            AlligatorOut = 0;
            argcs = 0;
            AlligatorIn = 1;
         }
      }
      else if (in->str[i][0] == '<'){
         if (AlligatorIn == 1){
            fprintf(stderr, "%s: Ambiguous Input\n", cmd);
            return -1;
         }
         else if (AlligatorIn == 2){
            fprintf(stderr, "%s: Bad Input Redirection\n", cmd);
            return -1;
         }
         else{
            AlligatorIn = 2;
            if (in->str[i+1][0] == '|' || in->str[i+1][0] == '<' 
               || in->str[i+1][0] == '>' || in->str[i+1][0] == '\0'){
               fprintf(stderr, "%s: bad input redirection.\n", cmd);
            return -1;
            }
         }
      }

      else if (in->str[i][0] == '>'){
         if (AlligatorOut == 2) {
            fprintf(stderr, "%s: bad input redirection.\n", cmd);
            return -1;
         } 
         else {
            AlligatorOut = 2;
            if (in->str[i+1][0] == '|'){
               fprintf(stderr, "%s: bad input redirection.\n", cmd);
            return -1;

            } 
            if (in->str[i+1][0] == '<'){
               fprintf(stderr, "%s: bad input redirection.\n", cmd);
            return -1;
            } 
            if(in->str[i+1][0] == '>'){
               fprintf(stderr, "%s: bad input redirection.\n", cmd);
            return -1;
            } 
            if (in->str[i+1][0] == '\0') {
               fprintf(stderr, "%s: bad input redirection.\n", cmd);
            return -1;
            }
         
      else {
         if (++argcs > MAX_CMND_NUM) { 
            fprintf(stderr, "%s: Too many arguments.\n", cmd);
            return -1;
         }
      }  
   }
   }
   }
   return 1;
}

void initInput(char *str, input *in) {
   in->len = 0;
   in->str[in->len++] = strtok(str, " \n");

   while ((in->str[in->len++] = strtok(NULL, " \n")) != NULL) {
      if (in->len == (131) 
         && in->str[in->len - 1] != NULL) {
         break;
      }
   }

   in->len--; 
}
