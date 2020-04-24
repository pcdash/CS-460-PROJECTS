/******************************** cat.c file ***********************************
 * Paul Valdez
 * CPTS 460
 * 
 * *******************************************************************************/

#include "ucode.c"
#define printk printf

#define BUFSIZE 4096

int tokenize(char *line, char *buf[], char delim);

char cret = '\r', newline = '\n';
char *myargv[BUFSIZE];
int myargc;

int main(int argc, char *argv[ ]){
    int fd, i, n, r, stdout, s, t;
    struct stat sp, myst1;
    char buf[BUFSIZE], mytty[64], *cp, c;

    //Set fd to default stdin
    fd = 0;

    //Set default redirection
    stdout = 0;

    // get tty
    gettty(mytty);
    // stat
    stat(mytty, &sp);
    // We send in file descriptor 1 because thats stdout
    fstat(1, &myst1);

    // Check if stdout is redirected
    if (sp.st_ino != myst1.st_ino) stdout = 1;

    // want to check if a file was passed in to cat to
    if (argc > 1){
        // printf(" OPEN FILE\n");
        fd = open(argv[1], O_RDONLY);
        if (fd < 0) exit(1);
    }

    // Dont print if redirected
    if (!stdout){
        printf("####################################\n");
        printf("## This is PAUL's CAT in progress ##\n");
        printf("####################################\n");
    }

    if (fd){
        // read from fd and write to stdout
        while (n = read(fd, buf, BUFSIZE)){
            buf[n] = 0;
            cp = buf;
            // if not redirected, run through a byte at a time and add \r for each \n
            if (!stdout){
                // printf("IN MY NON Redirected\n");
                // WHY AM I TOKENIZING IT??? iM PRINTINT IT OUT ONE CHAR AT A TIME ANYWAYS
                for (i = 0; i < n; i++){
                    c = *cp++;
                    // Write to fd
                    write(1, &c, 1);
                    // Check for new lines, if we hit one we end
                    switch(c){
                        case '\r':
                            write(1, &newline, 1);
                            write(1, &cret, 1);
                            break;
                        case '\n':
                            write(1, &cret, 1);
                            break;
                    }
                }

                // myargc = tokenize(buf, myargv, '\n');
                // for (i = 0; i < myargc; i++){
                //     r = write(1, myargv[i], sizeof(char) * strlen(myargv[i]));
                //     s = write(1, &newline, 1);
                //     // Dont write a carriage return if its the end
                //     if (i != myargc)
                //         t = write(1, &cret, 1);
                //     if (r < 0 || s < 0 || t < 0){
                //         printf("Failed to write.\n");
                //         exit(1);
                //     }
                // }
            } else{
                // r = write(1, "Redirected!", n);
                // We dont add any anything for redirected files
                r = write(1, buf, n);
                if (r < 0){
                    printf("Failed to write.\n");
                    exit(1);
                }
            }
        }
        // close the file
        close(fd);
    } else{
        while (n = gets(buf)){
            buf[n] = 0;
            r = write(1, buf, sizeof(char) * (strlen(buf)));
            s = write(1, &newline, 1);
            t = write(1, &cret, 1);
        }
    }
    return 0;
}

int tokenize(char *line, char *buf[], char delim){
    char *s = line, *t;
    int n = 0, i;
    while (*s){
        //Move past whitespace
        while (*s == delim) *s++ = 0;
        if (*s){
            buf[n++] = s;
        }
        while (*s && *s != delim) s++;
        //check if we hit delim or end of line
        if (*s) *s = 0;
        else break;
        s++;
    }
    if (n > 0)
        t = buf[n-1];
    i = strlen(t) - 1;
    while (*(t + i) && (*(t + i) == '\n' || *(t + i) == '\r')){
        // printf("IN LAST?\n");
        *(t + i) = 0;
        i--;
    }
    // Check the very last string for other delimiters
    buf[n] = 0;
    return n;
}