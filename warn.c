
#define WRT_ERR 1 /* error occurred writing to pipe */
#define RD_ERR 2  /* error reading from the pipe */

#define MAX_LEN 513 // max message length. 512 + terminator 

#include<unistd.h>
#include<stdio.h>
#include<signal.h>
#include<stdlib.h>

/* global signaling variables */
int msg = 0;  /* used by both parent and child */
              /* parent: New input to process? */
              /* child: parent will be writing, cease printing */

int done = 0; /* child process var. if 1, parent finished writing to pipe */

void parentHandler(); /* flips msg flag indicating new input to process */
void childHandlerBlock(); /* flips msg flag. Pauses printing until new string received */
void childHandlerRead(); // currently a debug fnx for the child process

void alrm(); /* cuz we need a fnx using signal and alarm. Does nothing */

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
    /* 3: rdr           */
    /* 4: wtr           */
   
    char print_string[MAX_LEN] = "Default message\n";  /* stores text entered after keyboard interrupt */

    int c_pid = fork();
    if (c_pid) {
        close(rdr);
        /* file descriptors */    
        /* 0: stdin         */
        /* 1: stdout        */
        /* 2: stderr        */
        /* 3: None          */
        /* 4: wtr           */

        /* prep interrupts */
        signal(SIGINT, parentHandler); 
        signal(SIGALRM, alrm);

        /* input check loop */
        while (1) {

            /* check if there is user input to process */
            if (msg) {
                signal(SIGINT, SIG_IGN); // No further interrupts until current input is processed 
                //----- HERE put a signal to the child to cease furthing printing 
                kill(c_pid, SIGUSR1);
                printf("Interrupt received -- Enter new string:");

                /* read the user input and write to the pipe */
                fgets(print_string, MAX_LEN, stdin); 
                if (write(wtr, print_string, MAX_LEN) == -1) {
                    fprintf(stderr, "error writing to pipe\n");
                    exit(WRT_ERR);
                }                  

                // signal the child to read from the pipe and continue operations */ 
                kill(c_pid, SIGFPE); 
                /* reset parent state and continue loop */
                msg = 0;
                signal(SIGINT, parentHandler);
            }
            alarm(1);
            pause();
            alarm(0);
        }

    }

    /* child process logic */
    else {
        close(wtr);

        /* file descriptors */    
        /* 0: stdin         */
        /* 1: stdout        */
        /* 2: stderr        */
        /* 3: rdr           */

        int delay = 2;
        signal(SIGINT, SIG_IGN);
        signal(SIGALRM, alrm);

        signal(SIGUSR1, childHandlerBlock); /* cease printing while parent processes new input */
        signal(SIGFPE, childHandlerRead); /* read from pipe and resume printing new string */

        while (1) {

            if (!msg) printf("%s", print_string);

            if (msg && done) {
                /* read in new string */
                //--------------
                /* this needs to be replaced with a function */
                /* that uses sscanf checking for optional delay */
                /* or the exit string */
                if (read(rdr, print_string, MAX_LEN) == -1) {
                    fprintf(stderr, "Error reading from the pipe");
                    exit(RD_ERR);
                }
                msg = 0;
                done = 0;
            }
            alarm(delay);
            pause();
            alarm(0);             
        }
    }
}

void childHandlerRead() {
    /*
    char str[MAX_LEN];
    if (read(3, str, MAX_LEN) == -1) {
        printf("error reading form pipe\n");
        exit(3);
    } 
    printf("string: <%s>\n", str);
    */
    done = 1;
}

/* simply satisfies arg2 of signal() */
void alrm() {
}

/* tells parent to process new input */
void parentHandler() {
    signal(SIGINT, SIG_IGN);
    msg = 1;
}

/* prevents child from writing */
/* when parent is processing input */
void childHandlerBlock() {
    msg = 1;
}
