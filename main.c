#include<stdio.h>
#include<signal.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>


void intake();
void alrm();
int main (int argc, char *argv[]) {

    
    int fd[2];
    int wtr, rdr;

    pipe(fd);
    rdr = fd[0];
    wtr = fd[1];

    /* file descriptors */    
    /* 0: stdin         */
    /* 1: stdout        */
    /* 2: stderr        */
    /* 3: fork_rdr      */
    /* 4: fork_wtr      */
   
    int c_pid = fork();
    if (c_pid) {
 
        close(rdr);
        /* file descriptors */    
        /* 0: stdin         */
        /* 1: stdout        */
        /* 2: stderr        */
        /* 3: NULL          */
        /* 4: fork_wtr      */

        signal(SIGINT, intake); 
        signal(SIGALRM, alrm);

        /* input check loop */
        while (1) {
            alarm(1);
            pause();
            alarm(0);
            printf("%d\n", test);
        }

    }
    else {
        signal(SIGINT, SIG_IGN);
        close(wtr);
        /* file descriptors */    
        /* 0: stdin         */
        /* 1: stdout        */
        /* 2: stderr        */
        /* 3: fork_rdr      */
        /* 4: NULL          */
   
        /* open a pipe so the alarm handler can    */
        /* relay the string being read from parent */ 
        int fd[2];
        int alrm_rdr, alrm_wtr;
        pipe(fd); 
        alrm_rdr = fd[0];
        alrm_wtr = fd[1];

        /* file descriptors */    
        /* 0: stdin         */
        /* 1: stdout        */
        /* 2: stderr        */
        /* 3: fork_rdr      */
        /* 4: alrm_rdr      */
        /* 5: alrm_wtr      */
    
    }
}

/* just satisfies the fnx requirement for signal()*/
void alrm(){
}

/* used by the parent to take user input and write to the child */
void intake() {
    signal(SIGINT, SIG_IGN); 
    printf("Interrupt receieved; Enter new message: ");
    char input[513];
    fgets(input, 513, stdin);    
    printf("OK!!:<%s>\n", input);
    signal(SIGINT, intake);
    //write(stdin, input, strlen(input));

    /* now test sending a signal to the child to get its attention */

    //* now write to child and let child handle the string how it needs */
    //write(4, input, 513);
}
