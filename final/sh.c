/******************************** sh.c file ***********************************
 * Paul Valdez
 * CPTS 460
 * 
 * *****************************************************************************/

#include "ucode.c"
#define printk printf
#define BUFSIZE 256 //size = 4kb = 1024
#define PATHSIZE 1024

int do_pipe(char *cmdline, int *pd);
int do_command(char *cmdline);
int scan(char *cmdline, char *head, char *tail);
int remove_white(char *str);

int myargc;
int lpd[2], numpipes;
char *buf[BUFSIZE], *cp, *tempcp, *myargv[BUFSIZE], head[BUFSIZE], tail[BUFSIZE];

int tokenize(char *line, char *info[], char delim){
    char *s = line;
    int n = 0;
    while (*s){
        //Move past whitespace
        while (*s == ' ') *s++ = 0;
        if (*s) info[n++] = s;
        while (*s && *s != delim) s++;
        //check if we hit delim or end of line
        if (*s) *s = 0;
        else break;
        s++;
    }
    info[n] = 0;
    return n;
}

// Will get name from password file
int whoami(char *myname){
    char buf1[PATHSIZE], *info_buf[BUFSIZE], *user_info[BUFSIZE], pathname[BUFSIZE], cwd[BUFSIZE];
    int uid, fd, n;
    // Get user id
    uid = getuid();
    getcwd(cwd);
    strcpy(pathname, cwd);
    if (strcmp(cwd, "/")!=0){
        chdir("/");
    }
    getcwd(cwd);
    //Open password file for read only
    fd = open("etc/passwd", O_RDONLY);
    // fd = open(pathname, O_RDONLY);
    read(fd, buf1, PATHSIZE);
    // tokenize into the lines
    n = tokenize(buf1, info_buf, '\n');
    // Now get the username from the info buf
    if (uid < n){
        tokenize(info_buf[uid], user_info, ':');
        strcpy(myname, user_info[0]);
    } else{
        //If it didnt succeed, give it my name :)
        strcpy(myname, "paul");
    }
    chdir(pathname);
    close(fd);
    return 0;
}

int main(int argc, char *argv[ ]){
    int pid, status, i, n;
    char cwd[64], temp[BUFSIZE], myname[BUFSIZE], *dirs[BUFSIZE];
    //FOR LATER, CAN USER GETUID() TO GET THE USERNAME!!

    printf("#####################################\n");
    printf("## This is PAUL's SH in progress ##\n");
    printf("#####################################\n");
    whoami(myname);

    while (1){
        numpipes = 1;
        getcwd(cwd);
        // If not root, dont allow user to know they can go backwards
        if (getuid()){
            n = tokenize(cwd, dirs, '/');
            strcpy(temp, "/");
            for (i = 3; i < n; i++){
                strcat(temp, dirs[i]);
            }
            printf("%s@%s:~%s$ ", myname, "pc-SHELL", temp);
        } else{
            printf("%s@%s:~%s$ ", myname, "pc-SHELL", cwd);
        }
        
        //Get the command string
        gets(buf);

        //Make sure something was entered
        if (buf[0] == 0)
            continue;
        // make sure that the string is not all blank
        // cp = buf;
        // remove_white(cp);
        remove_white(buf);
        // Check if it was blank now
        // if (!cp || !strcmp(cp, ""))
        //     continue;
        if (!buf || !strcmp(buf, ""))
            continue;

        // Copy into buf
        // strcpy(buf, cp);
        // strcpy(temp, cp);
        strcpy(temp, buf);

        // Get our commands and the number of them into myargv and myargc
        myargc = eatpath(temp, myargv);
        // printf("::%s::\n", myargv[0]);

        if (strcasecmp(myargv[0], "cd")==0){
            // if no argument given for cd, give it to root
            if (myargv[1] == 0){
                myargv[1] = "/";
            }
            // Make sure user is root
            if (!getuid())
                chdir(myargv[1]);
        } else if (strcasecmp(myargv[0], "clear")==0){
            for (i = 0; i < 25; i++){
                printf(" \n");
            }
        } else if (strcasecmp(myargv[0], "chname")==0){
            chname(name[i]);
        } else if (strcasecmp(myargv[0], "help")==0){
            printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
            printf("                HELP: Simple commands\n");
            printf("  ~ cd ~ chname ~ logout ~ exit ~ ls ~ ps ~ clear ~\n");
            printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        } else if (strcasecmp(myargv[0], "logout")==0){
            printf("Goodbye\n");
            exit(0);
        } else if (strcasecmp(myargv[0], "exit")==0){
            exit(0);
        } else{
            // do fork and command
            pid = fork();
            // printf("parent sh %d forks a childe %d\n", getpid(), pid);

            if (pid){
                // parents pid, wait for child
                // printf("parent sh %d waits for a childe %d\n", getpid(), pid);
                pid = wait(&status);
                // printf("parent sh %d: childe %d exit status %d\n", getpid(), pid, status);
            } else{
                // printf("childe sh %d running: cmd=%s\n", getpid(), buf);
                do_pipe(buf, 0);
            }
        }
    }
    return 0;
}

int do_pipe(char *cmdline, int *pd){
    int hasPipe = 0, r = 0;
    // int lpd[2];
    // printf("DO PIPE ON CMD: %s\n", cmdline);
    // Check if a pipe was passed in as a writer on pipe pd
    if (pd){
        // printf("CLOSE PIPE AS READER\n");
        // close reader and let writer take control
        close(pd[0]);
        dup2(pd[1],1);
        // close(pd[1]);
    }

    // divide cmdline into head and tail by rightmost pipe symbol
    hasPipe = scan(cmdline, head, tail);

    // Make sure that there is a pipe
    if (hasPipe){
        // create a pipe
        r = pipe(lpd);
        if (r < 0){
            printf("Error: Pipe not created.\n");
            exit(1);
        }
        // numpipes += 1;
        // fork
        pid = fork();
        if (pid){ // parent
            numpipes += 1;
            // open pipe as reader on lpd
            close(lpd[1]);
            dup2(lpd[0], 0);
            // close(lpd[0]);
            // printf("PC SH: Piped for %s\n", tail);
            // do tail command now
            do_command(tail);
        } else{ // childe
            // printf("PC SH: does pipe for %s\n", head);
            // do pipe again with head and lpd
            return 1 + do_pipe(head, lpd);
            // do_command(head);
        }
    } else{
        // if it has no pipe just do the command
        do_command(cmdline);
    }
    return 1;
}

// int scan(char *cmdline, char *head, char *tail){
//     int length, i;
//     length = strlen(cmdline);

//     // Go from the back end and look for the pipe
//     for (i = length - 1; i >= 0; i--){
//         // check if pipe found
//         if (*(cmdline + i) == '|'){
//             strcpy(tail, cmdline + i + 1);
//             strncpy(head, cmdline, i);
//             cmdline[i] = '\0';
//             return 1;
//         }
//     }
//     // if no pipe, then set head to command line and tail to null
//     strcpy(head, cmdline);
//     tail = 0;
//     return 0;
// }
int scan(char *cmdline, char *head, char *tail){
    int length, i;
    length = strlen(cmdline);

    // Go from the backend and look for the pipe
    for (i = length - 1; i >= 0; i--){
        // if pipe is found
        if (*(cmdline + i) == '|'){
            printf("PC SH: FOUND PIPE!\n");
            strcpy(tail, cmdline + 1 + i);
            remove_white(tail);
            cmdline[i] = 0;
            strcpy(head, cmdline);
            remove_white(head);
            return 1;
        }
    }
    // if no pipe the set head to command line and tail to null
    strcpy(head, cmdline);
    remove_white(head);
    tail = 0;
    return 0;
}
int remove_white(char *str){
    if (!str) return 0;
    int length = strlen(str) - 1;
    // remove from end
    while (*(str + length) == ' '){
        *(str + length) = 0;
        length--;
    }
    // remove from front
    while (*str == ' '){
        str++;
    }
    return 0;
}

int do_command(char *cmdline){
    char *cmd, temp[BUFSIZE], *s, *names[BUFSIZE];
    int fd = 0, i, j, r, endcmd, n;
    temp[0] = 0;

    strcpy(temp, cmdline);
    n = eatpath(temp, names);
    endcmd = n;
    // Check for IO redirection before anything
    for (i = n - 1; i >= 0; i--){
        // Open file to read
        if (strcmp(names[i], "<")==0){
            close(0);
            // filename to open will be the next argument
            fd = open(names[i+1], O_RDONLY);
            // clear arguments at tail end
            names[i] = 0;
            endcmd = i;
            break;
        }
        // Open file to write
        if (strcmp(names[i], ">")==0){
            close(1);
            // filename to open will be the next argument
            fd = open(names[i+1], O_WRONLY|O_CREAT);
            // clear arguments at tail end
            names[i] = 0;
            endcmd = i;
            break;

        }
         // Open file to append
        if (strcmp(names[i], ">>")==0){
            close(1);
            // filename to open will be the next argument
            fd = open(names[i+1], O_WRONLY|O_CREAT|O_APPEND);
            // clear arguments at tail end
            names[i] = 0;
            endcmd = i;
            break;
        }
    }
    // Check and make sure a file was opened
    if (fd < 0){
        printf("Error: file failed to open\n");
        exit(2);
    }

    strcpy(cmd, names[0]);
    // copy rest of command line to command
    for (j = 1; j < endcmd; j++){
        // printf("BUILDING OUR CMD\n\n");
        strcat(cmd, " ");
        remove_white(names[j]);
        strcat(cmd, names[j]);
    }
    // printf("DO COMMAND %s\n\n", cmd);
    r = exec(cmd);
    if (r < 0){
        printf("Error!\n");
        exit(1);
    }
    while (1){
        printf("%d: enter a key : ", getpid());
        getc();
    }
    return 0;
}



