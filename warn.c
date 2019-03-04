
#define WRT_ERR 1 /* error occurred writing to pipe */

#define MAX_LEN 513 // max message length. 512 + terminator 

#include<unistd.h>
#include<stdio.h>
#include<signal.h>
#include<stdlib.h>

/* global signaling variables */
int msg = 0;  // Used by both parent and child
int msg_sent = 0; // Used only by child

void parentHandler();
void placeholder(); // currently a debug fnx for the child process
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
    /* 3: rdr           */
    /* 4: wtr           */
   
    char print_string[MAX_LEN] = "Default message";  /* stores text entered after keyboard interrupt */

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

        printf("ever?\n");   
        signal(SIGINT, SIG_IGN);
        signal(SIGFPE, placeholder); /* debug just to read and print */
        signal(SIGALRM, alrm);
        while (1) {
            alarm(1);
            pause();
            alarm(0);             
            //printf("alive\n");
        }
    }
}

void placeholder() {

    char str[MAX_LEN];
    if (read(3, str, MAX_LEN) == -1) {
        printf("error reading form pipe\n");
        exit(3);
    } 
    printf("string: <%s>\n", str);
}

/* simply satisfies arg2 of signal() */
void alrm() {
}

/* tells parent to process new input */
void parentHandler() {
    signal(SIGINT, SIG_IGN);
    msg = 1;
}


