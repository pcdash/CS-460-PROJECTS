/******************************** grep.c file ***********************************
 * Paul Valdez
 * CPTS 460
 * 
 * *******************************************************************************/

#include "ucode.c"
#define printk printf

#define BLKSIZE 4096
#define PATHSIZE 256

int stdin, stdout, fd, gd;
char buf[BLKSIZE], *cp;
char cret = '\r', newline = '\n';

// want to read in characters, convert them to uppercase and print back
// To convert, assuming its a character and not a number, we add lower case 'a' to it
// if c >= 'a' and c <= 'z' then subtract 'a' - 'A' to it. everything else is not a alhpabetic character
int main(int argc, char *argv[ ]){
    int r, n, s, t;
    struct stat sp, myst0, myst1;
    char mytty[64], c;

    printf("#####################################\n");
    printf("## This is PAUL's GREP in progress ##\n");
    printf("#####################################\n");

    //Set default redirection
    stdin = 0;
    stdout = 0;

    // set default file descriptors
    fd = 0;
    gd = 1;

    // get tty
    gettty(mytty);
    // stat
    stat(mytty, &sp);
    // We send in file descriptor 0 because thats stdin
    fstat(0, (char *) &myst0);
    fstat(1, (char *) &myst1);

    // Check if I/O is redirected
    if (sp.st_ino != myst0.st_ino) stdin = 1;
    if (sp.st_ino != myst1.st_ino) stdout = 1;

    // Check if files were entered
    if (argc > 2){
        stdin = 1;
        stdout = 1;
        // get inputs from file
        fd = open(argv[1], O_RDONLY);
        // put outputs in file
        gd = open(argv[2], O_WRONLY);
        // printf("argv=::%s::\n", argv[2]);
    } else if (argc > 1){
        stdin = 1;
        close(0);
        // get inputs from file
        fd = open(argv[1], O_RDONLY);
        // if stdout is redirected, update
        if (stdout)
            gd = dup(1);
        close(1);
        open(mytty, O_WRONLY);
    } else{
        // Check if they are redirected
        if (stdin)
            fd = dup(0);
        close(0);
        open(mytty, O_RDONLY);

        if (stdout)
            gd = dup(1);
        close(1);
        open(mytty, O_WRONLY);
    }
    // if (stdin)
    //     printf("REDIRECTED, STDIN=%d, STDOUT=%d\n", stdin, stdout);
    // Make sure fd and gd are open
    if (fd < 0 || gd < 0){
        printf("Cannot open files.\n");
        exit(1);
    }

    // Now we want to read in from fd and print out to gd
    while (n = read(fd, buf, BLKSIZE)){
        buf[n] = 0;
        // if the last bit is a return carriage, remobe it
        if (buf[n-1] == '\r') buf[n-1] = 0;
        cp = buf;
        while (c = *cp++){
            // Check if alphabetical number. If so, shift c by subtracting the difference between a and A
            if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
            // Check if we should stop or continue
            if (stdout && (c == '\r')) continue;

            // write c in
            r = write(gd, &c, 1);

            // Check for new lines, if we hit one we end
            if ((!stdout || !stdin) && c == '\n')
                    write(gd, &cret, 1);
            else if (!stdin && c == '\r'){
                    write(gd, &cret, 1);
                    write(gd, &newline, 1);
            }   
        }
    }

    // Close files
    if (fd) close(fd);
    if (gd) close(gd);
    return 0;
}