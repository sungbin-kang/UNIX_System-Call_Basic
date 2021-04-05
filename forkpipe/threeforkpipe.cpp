/**
 * UNIX Shell command program:
 *    % ps -A | grep worker | wc -l
 */
#include <stdlib.h>   //exit
#include <stdio.h>    //perror
#include <unistd.h>   //fork, pipe
#include <sys/wait.h> //wait
#include <iostream>
using namespace std;

const int BUF_SIZE = 5000;

int main()
{
   enum
   {
      READ,
      WRITE
   };
   int pipeFD_ps[2], pipeFD_grep[2], pipeFD_wc[2];
   pid_t pid_ps, pid_grep, pid_wc;

   if (pipe(pipeFD_ps) < 0)
   {
      perror("Error in creating pipe ps");
      exit(EXIT_FAILURE);
   }
   if (pipe(pipeFD_grep) < 0)
   {
      perror("Error in creating pipe grep");
      exit(EXIT_FAILURE);
   }
   if (pipe(pipeFD_wc) < 0)
   {
      perror("Error in creating pipe wc");
      exit(EXIT_FAILURE);
   }

   pid_ps = fork();

   if (pid_ps < 0)
   {
      perror("Error during fork A");
      exit(EXIT_FAILURE);
   }

   if (pid_ps == 0)
   {
      cout << "----- Child A -----" << endl;

      close(pipeFD_wc[READ]);
      close(pipeFD_wc[WRITE]);
      close(pipeFD_grep[READ]);
      close(pipeFD_grep[WRITE]);
      close(pipeFD_ps[READ]);

      dup2(pipeFD_ps[WRITE], 1); // stdout is pipe ps write
      execlp("/bin/ps", "ps", "-A", NULL);

      exit(EXIT_FAILURE);
   }
   else
   {
      pid_grep = fork();

      if (pid_grep < 0)
      {
         perror("Error during fork B");
         exit(EXIT_FAILURE);
      }
      if (pid_grep == 0)
      {
         cout << "----- Child B -----" << endl;

         close(pipeFD_wc[READ]);
         close(pipeFD_wc[WRITE]);
         close(pipeFD_grep[READ]);
         close(pipeFD_ps[WRITE]);

         dup2(pipeFD_ps[READ], 0);    // stdin is pipe ps read
         dup2(pipeFD_grep[WRITE], 1); // stdout is pipe grep write
         execlp("grep", "grep", "worker", NULL);

         exit(EXIT_FAILURE);
      }
      else
      {
         pid_wc = fork();

         if (pid_wc < 0)
         {
            perror("Error during fork wc");
            exit(EXIT_FAILURE);
         }
         else if (pid_wc == 0)
         {
            cout << "----- Child C -----" << endl;

            close(pipeFD_ps[READ]);
            close(pipeFD_ps[WRITE]);
            close(pipeFD_grep[WRITE]);
            close(pipeFD_wc[READ]);

            dup2(pipeFD_grep[READ], 0); // stdin is pipe grep read
            dup2(pipeFD_wc[WRITE], 1); // stdout is pipee wc write

            execlp("wc", "wc", "-l", NULL);

            exit(EXIT_FAILURE);
         }
         else
         {
            cout << "----- Parent -----" << endl;

            close(pipeFD_grep[WRITE]);
            close(pipeFD_ps[READ]);
            close(pipeFD_ps[WRITE]);

            char buf[BUF_SIZE];
            int n = read(pipeFD_wc[READ], buf, BUF_SIZE);
            buf[n] = '\0';
            cout << buf;

            cout << "Parent exiting.." << endl;
            exit(EXIT_SUCCESS);
         }
      }
   }
}
