#define MAX_LEN 513  // 512 + terminator

#include<stdio.h>
#include<signal.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
int pid;
int msg = 0;  /* when 1: indicates a new message needs to be processed  */

void placeholder();
void writePipe(char[], int);
void intake();
void alrm();
void readSig();
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
   
    char next_string[MAX_LEN] = "Default message";  /* stores text entered after keyboard interrupt */

    int c_pid = fork();
    if (c_pid) {
        pid = c_pid;  
        printf("child process id is %d\n", pid);
        close(rdr);
        /* file descriptors */    
        /* 0: stdin         */
        /* 1: stdout        */
        /* 2: stderr        */
        /* 3: NULL          */
        /* 4: fork_wtr      */

        /* prep interrupts */
        signal(SIGINT, intake); 
        signal(SIGALRM, alrm);


        /* input check loop */
        while (1) {

            if (msg) {
                //signal(SIGINT, SIG_IGN);
                //fgets(next_string, MAX_LEN, stdin); 
                // writeMsg(next_string, wtr)
                msg = 0;
            }
            alarm(1);
            pause();
            alarm(0);
        }

    }
    else {
        close(wtr);
        /* file descriptors */    
        /* 0: stdin         */
        /* 1: stdout        */
        /* 2: stderr        */
        /* 3: fork_rdr      */
        /* 4: NULL          */
        printf("ever?\n");   
        signal(SIGINT, SIG_IGN);
        signal(SIGFPE, placeholder); /* readSig will be there in the real version */
        signal(SIGALRM, alrm);
        while (1) {
            alarm(1);
            pause();
            alarm(0);             
            //printf("alive\n");
        }


        /* open a pipe so the alarm handler can    */
        /* relay the string being read from parent */ 
        /*
        int fd[2];
        int alrm_rdr, alrm_wtr;
        pipe(fd); 
        alrm_rdr = fd[0];
        alrm_wtr = fd[1];
        */
        /* file descriptors */    
        /* 0: stdin         */
        /* 1: stdout        */
        /* 2: stderr        */
        /* 3: fork_rdr      */
        /* 4: alrm_rdr      */
        /* 5: alrm_wtr      */
    
    }
}

void placeholder() {
    //printf("in placeholder\n");
    char string[513];
    read(3, string, 513);
    printf("<%s>\n", string);
}
/* parent function */
/* writes user-supplied string to child */
void writePipe(char string[], int fd) {

    int len = strlen(string);
    
    if (write(fd, string, ++len) != len) {
        fprintf(stdout, "Error writing to pipe\n");
        // signal child process to die
        exit(1);
    }

}

/* indicates new message to process */
/* used by both parent and child */
void readSig(){
    msg = 1;
}

/* just satisfies the fnx requirement for alarm */
void alrm(){
}

/* used by the parent to take user input and write to the child */
void intake() {
    signal(SIGINT, SIG_IGN); 
    printf("Interrupt receieved; Enter new message: ");
    char input[513];
    fgets(input, 513, stdin);    
    write(4, input, 513);
    printf("%s\n", strerror(errno));
    printf("OK!!:<%s>\n", input);
    if (kill(pid, SIGFPE) == -1) printf("%s\n", strerror(errno));    
    msg = 1;
    signal(SIGINT, intake);
    //write(stdin, input, strlen(input));

    /* now test sending a signal to the child to get its attention */

    //* now write to child and let child handle the string how it needs */
    //write(4, input, 513);
}
