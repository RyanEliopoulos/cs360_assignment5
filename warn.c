
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


/* generalized string processing function */
/* wrapped by parent and child specific logic */

/* return value is meant for the child only          */
/* if 1, childProcessInput is meant to replace       */
/* the current string with the new pipe-provided one */
int genericProcess(int *, char[]);

/* global signaling variables */
int msg = 0;  /* used by both parent and child */
              /* parent: New input to process? */
              /* child: parent will be writing, cease printing */

int done = 0; /* child process var. if 1, parent finished writing to pipe */

/* processes user input before sending to child */
/* input of only whitespace is converted to \n  */
/* leading white space is removed               */
int parentProcessInput(char *);

/* flips msg flag indicating new input to process */
void parentHandler(); 

/* reads from pipe, relies upon genericProcess */
void childProcessInput(int, int*, char[]);

/* flips msg flag. Pauses printing until new string received */
void childHandlerBlock(); 

/* currently a debug fnx for the child process */
void childHandlerRead(); 

/* cuz we need a fnx using signal and alarm. Does nothing */
void alrm(); 


/******   Program Begins ********/
int main (int argc, char *argv[]) {

    /* prep for fork */
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

                signal(SIGINT, SIG_IGN); /* No further interrupts until current input is processed */
                kill(c_pid, SIGUSR1);    /* initial signal to child. Prevents further printing */
                printf("Interrupt received -- Enter new message:");
                int exit_flag = parentProcessInput(print_string);

                /* send string to the child */
                if (write(wtr, print_string, MAX_LEN) == -1) {  
                    fprintf(stderr, "error writing to pipe\n");
                    kill(c_pid, SIGKILL);
                    exit(WRT_ERR);
                }                  
                /* signal the child to read from the pipe */
                kill(c_pid, SIGFPE); 

                /* Now check if what was just sent was the exit signal */
                if (exit_flag) {
                    /* child will parse this and know to exit */
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
                
                /* reset state for next loop */
                msg = 0;
                done = 0;

                /* ensure delay isn't an illegal value */
                delay = (delay < 1) ? 1 : delay;
            }
            alarm(delay);
            pause();
            alarm(0);             
        }
    }
}


int genericProcess(int *delay, char print_string[]) {

    char temp_string[MAX_LEN];
    strcpy(temp_string, print_string);

    /* screen for leading floating point number that might mess up sscanf %d */ 
    int garb1, garb2;
    char c;
    int ret = sscanf(temp_string, "%d %[.] %d", &garb1, &c, &garb2); 

    if (ret == 3) {
        /* leading number is considered a floating point */ 
        strcpy(print_string, temp_string); 
        return 1;
    }
    else {

        /* pipe didn't contain a special case */   
        /* analyze for delay modification */
        int temp_delay;  
        char scan_string[MAX_LEN];
        ret = sscanf(temp_string, "%d %s", &temp_delay, scan_string);
        if (ret == 2) {
            /* check if there is any whitespace following the integer */
            /* if not, the integer is part of a larger string and doesn't count */

            /* find integer end */
            int i = 0;
            /* handle possible negative sign check for negative sign */
            if (temp_string[i] == '-') i++;
            while (isdigit(temp_string[i])) i++;  /* get past the integer */
            if (isspace(temp_string[i])) {  /* The integer is a dictinct unit */

                /* there is a new delay time and string */
                *delay = abs(temp_delay);
                while (isspace(temp_string[i])) i++;  /* now get past the whitespace */
                strcpy(print_string, temp_string + i);
                return 1;
            }
            else {
                /* the integer was only a substring and thus doesn't qualify */
                strcpy(print_string, temp_string);
                return 1;
            }
        }
        else if (ret == 1) {
            /* a standalone integer. We simply change the delay time */
            *delay = abs(temp_delay);
            return 0;
        }
    }

    /* making it here means the info from the pipe was */
    /* neither special nor involved number shenanigans */
    /* The child is clear to copy the string.          */
    return 1;
}

void childProcessInput(int fd, int *delay, char print_string[]) {

    /* read data from pipe */
    char temp_string[MAX_LEN]; 
    if (read(fd, temp_string, MAX_LEN) == -1) {
        fprintf(stderr, "error reading from pipe\n"); 
        exit(WRT_ERR); 
    } 

    /* special case: since sscanf doesn't treat whitespace like it deserves */
    if (!strcmp("\n", temp_string)) {
        strcpy(print_string, "\n");
        return;
    }
   
    /* check if there is a new string */ 
    if (genericProcess(delay, temp_string)) {
        strcpy(print_string, temp_string);
    }

    /* check if the string was an exit signal */
    if (!strcmp("exit\n", temp_string)) {
        printf("Exit signal received. Terminating..\n");
        exit(0);
    }
}

/* returns 1 if exit signal received */
int parentProcessInput(char print_string[]) {
    
    /* get user input */
    fgets(print_string, MAX_LEN, stdin);

    /* strip leading whitespace */
    int i = 0;
    while(isspace(print_string[i++]) && i < strlen(print_string)) 

    /* check if it was solely whitespace */
    if (i > strlen(print_string)) {
        strcpy(print_string, "\n");
        return 0;
    }

    /* there is at least one non-whitespace char at i-1 */
    /* shift string forward from there */
    strcpy(print_string, print_string + --i);

    /* now analyze string for exit signal */
    int temp_delay;  // satisfies process reqs
    char temp_string[MAX_LEN];
    strcpy(temp_string, print_string);

    genericProcess(&temp_delay, temp_string);
    /* signal exit command, if present */
    if (!strcmp("exit\n", temp_string)) return 1;
    return 0;
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


