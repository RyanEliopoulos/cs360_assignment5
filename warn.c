
#define WRT_ERR 1 /* error occurred writing to pipe */
#define RD_ERR 2  /* error reading from the pipe */

#define MAX_LEN 513 // max message length. 512 + terminator 

#include<unistd.h>
#include<stdio.h>
#include<signal.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<sys/wait.h>

/* global signaling variables */
int msg = 0;  /* used by both parent and child */
              /* parent: New input to process? */
              /* child: parent will be writing, cease printing */

int done = 0; /* child process var. if 1, parent finished writing to pipe */

void parsePipe(int, int *, char[]); /* used by child process for new strings. Checks for updates to delay setting */
void parentProcessInput(char *);
void parentHandler(); /* flips msg flag indicating new input to process */
void childProcessInput(int, int*, char[]);
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
                kill(c_pid, SIGUSR1); /* initial signal to child. Prevents further printing */
                printf("Interrupt received -- Enter new message:");

                /* read input into print_string */
                /* transform if necessary       */
                parentProcessInput(print_string);
                if (write(wtr, print_string, MAX_LEN) == -1) {  
                    fprintf(stderr, "error writing to pipe\n");
                    kill(c_pid, SIGKILL);
                    exit(WRT_ERR);
                }                  
                // signal the child to read from the pipe */
                kill(c_pid, SIGFPE); 
                /* check if it was the exit signal */
                if (!strcmp("exit\n", print_string)) {
                    /* child will parse this and know to exit */
                    printf("got the exit signal, mang\n");
                    wait(NULL);
                    exit(0);
                }
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
        msg = 1; /* prevents printing until first user-input string */
        signal(SIGINT, SIG_IGN);
        signal(SIGALRM, alrm);

        signal(SIGUSR1, childHandlerBlock); /* cease printing while parent processes new input */
        signal(SIGFPE, childHandlerRead); /* read from pipe and resume printing new string */

        while (1) {
            if (!msg) {
                printf("%s", print_string);
                fflush(stdout);
            }

            if (msg && done) {

                /* check message */
                childProcessInput(rdr, &delay, print_string);
                if (!strcmp("exit", print_string)) {
                    printf("Exit signal received. Terminating..\n");
                    exit(0);    
                }

                /* read in new string */
                //if (parsePipe(rdr, &delay, print_string)) { // Rets 0 if only updating the delay
                    //if (!strcmp("exit\n", print_string)) {
                        //close(rdr);
                        //printf("Received exit signal. Terminating..\n");
                        //exit(0);
                    //}
                    /* put child filter here */
                    /* reset signaling variables */
                msg = 0;
                done = 0;
                //}
            }
            alarm(delay);
            pause();
            alarm(0);             
        }
    }
}


/* turns pipe contents into a string */
/* updates delay value, if present */
void parsePipe(int fd, int *delay, char print_string[]) {

    char string_candidate[MAX_LEN]; /* temp var for checking incoming content */
    /* read string from the pipe */
    if (read(fd, string_candidate, MAX_LEN) == -1) {
        fprintf(stderr, "Error reading from the pipe");
        exit(RD_ERR);
    }
    printf("<%s>\n", string_candidate);
    // Remove leading whitespace
    //lstrip(string_candidate);


    
    /* evaluate string contents */
    int delay_candidate;  /* storage for potential new delay setting */

    /* check for a new delay setting and new string */ 
    int ret = sscanf(print_string, "%d %s", &delay_candidate, print_string);
    //int ret = sscanf(print_string, "%d %[A-Z,a-z,0-9, ]", &delay_candidate, string_candidate);
    printf("ret is: %d\n", ret);
    if (ret == 2) {
        *delay = delay_candidate;

        // need to remove the integer value from the input string 
        //int i = 0;
        //while (

        strcpy(print_string, string_candidate);
        return;
    } 
    if (ret == 1) {
        /* only a new delay was present */
        *delay = delay_candidate;
        return;
    }

    /* No integer at all so accept as a single string */
    strcpy(print_string, string_candidate);
}


void childProcessInput(int fd, int *delay, char print_string[]) {

    /* read data from pipe */
    char temp_string[MAX_LEN]; 
    if (read(fd, temp_string, MAX_LEN) == -1) {
        fprintf(stderr, "error reading from pipe\n"); 
        exit(WRT_ERR); 
    } 
    
    /* check for special cases */
    if (!strcmp("exit\n", temp_string)) {
        printf("Exit signal received. Terminating..\n");
        exit(0);
    }

    /* special case: since sscanf doesn't treat whitespace like it deserves */
    if (!strcmp("\n", temp_string)) {
        strcpy(print_string, "\n");
        return;
    }
    
    /* pipe didn't contain a special case */   

    int temp_delay;  
    char scan_string[MAX_LEN];
    int ret = sscanf(temp_string, "%d %s", &temp_delay, scan_string);
       
    if (ret == 2) {
        /* there is a new delay time and string */
        *delay = temp_delay;

        /* update print_string but exclude the integer */  
        int i = 0;
        while ( isdigit(scan_string[i]) || isspace(scan_string[i++]) )
        strcpy(print_string, scan_string + --i);
    }
    else {
        /* a standalone integer is treated simply as a string */
        strcpy(print_string, temp_string);   
    }
}

/* strips leading whitespace characters    */
/* or, if line is only whitespace, subs in */
/* a single newline character              */
void parentProcessInput(char print_string[]) {
    
    /* get user input */
    fgets(print_string, MAX_LEN, stdin);

    /* strip leading whitespace */
    int i = 0;
    while(isspace(print_string[i++]) && i < strlen(print_string)) 

    if (i > strlen(print_string)) {
        /* the input string is entirely newline chars */
        strcpy(print_string, "\n");
        return;
    }

    /* there is at least one non-whitespace char at i-1 */
    /* shift string forward from there */
    strcpy(print_string, print_string + --i);
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
void childHandlerBlock() {
    msg = 1;
}

/* flags pipe as having content */
void childHandlerRead() {
    done = 1;
}


