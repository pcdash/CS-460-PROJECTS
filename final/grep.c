/******************************** grep.c file ***********************************
 * Paul Valdez
 * CPTS 460
 * 
 * *******************************************************************************/

#include "ucode.c"
#define printk printf

#define BLKSIZE 4096
#define PATHSIZE 256

char cret = '\r', newline = '\n';
int fd, gd, remain, stdin, stdout;
char buf[BLKSIZE], *cp;

int grep(char *str);
int writeline();

int main(int argc, char *argv[ ]){
    int r, n, s, t, length;
    struct stat sp, myst0, myst1;
    char mytty[64], c, temp[PATHSIZE], *tcp;

    printf("#####################################\n");
    printf("## This is PAUL's GREP in progress ##\n");
    printf("#####################################\n");

    //Set default redirection
    stdin = 0;

    // set default file descriptor
    fd = 0;

    // get tty
    gettty(mytty);
    // stat
    stat(mytty, &sp);
    // We send in file descriptor 0 because thats stdin
    fstat(0, (char *) &myst0);
    fstat(1, (char *) &myst1);

    // Check if stdout is redirected
    if (sp.st_ino != myst0.st_ino) stdin = 1;
    // Check if stdout is redirected
    if (sp.st_ino != myst1.st_ino) stdout = 1;

    // Check if a file was entered
    if (argc > 2){
        stdin = 1;
        fd = open(argv[2], O_RDONLY);
    } else if (argc > 1) {
        // if input has been redirected, then we want to duplicat the lowest 
        if (stdin)
            fd = dup(0);
        close(0);
        gd = open(mytty, O_RDONLY);
    } else{
        // exit because grep requires one operand
        printf("proper usage: grep <pattern> [file]\n");
        exit(1);
    }

    // Make sure fd is open
    if (fd < 0){
        printf("Cannot open file: %s\n", argv[2]);
        exit(1);
    }

    // Now grep
    if (fd || stdin || stdout){
        // printf("fd=:%d:, stdin=:%d:\n", fd, stdin);
        grep(argv[1]);

        // printf("STRING TO FIND WAS :%s:\n", argv[1]);
    // write(1, &newline, 1);
    } else{
        // printf("NON REDIRECTED\n");
        // Run through getting a character each time the user presses it
        // while (c = getc()){
        // while (read(gd, &c, 1)){
        //     // check for specials
        //     switch(c){
        //         case '\n':
        //             // write(1, &c, 1);
        //             c = 'a';
        //             break;
        //         case '\r':
        //             write(1, &c, 1);
        //             c = 'a';
        //             break;
        //         default:
        //             write(1, &c, 1);
        //     }
        //     // Check if the control d was pressed
        //     if (c == EOF) break;
        // }
         close(0);
        gd = open(mytty, O_RDONLY);
        length = strlen(argv[1]);
        buf[0] = 0;
        // printf("GREP :%s:\n", argv[1]);
        // while (n = read(gd, buf, BLKSIZE*sizeof(char))){
        while (n = gets(buf)){
            buf[n] = 0;
            cp = buf;
        
            // Go through the block and write when we find the right lines
            while (*cp){
                // copy n characters from tcp into temp so we can compare disregarding case
                strncpy(temp, cp, length);
                // printf("Now comparing ::%s::", temp);

                // If the length of the string to compare is more than what we read in, return
                if (strlen(temp) > length) break;

                // Now compare the string
                if (strcasecmp(argv[1], temp) == 0){
                    // point cp back to the start of buf
                    cp = buf;
                    // print out to the screen
                    writeline();  
                    // break out of loop and get new line
                    break;
                }
                // update cp
                cp++;
            }
            buf[0] = 0;
        }
    }

    // close file
    if (fd) close(fd);
    if (stdin) close(gd);
    return 0;
}

// Probably use strncmp()
int grep(char *str){
    int ret, length = strlen(str), end;
    char temp[PATHSIZE], *tcp;
    // printf("string to find = :%s:\n", str);

    while (1){
        // Want to read in a blk if there is none
        if (remain <= 0){
            buf[0] = 0;
            remain = read(fd, buf, BLKSIZE * sizeof(char));
            // zero out the end
            buf[remain] = 0;
            // if remain is 0, nothing was read in and we can return 0
            if (remain <= 0) return remain;

            // Let cp point to buf
            cp = buf;
            // Let tcp = cp
            tcp = cp;
        }
        // Go through the block and write when we find the right lines
        while (remain >= 0 || *tcp){
            temp[0] = 0;
            // copy n characters from tcp into temp so we can compare disregarding case
            strncpy(temp, tcp, length);
            temp[length] = 0;

            // If the length of the string to compare is more than what we read in, return
            if (strlen(temp) > length) break;

            // Now compare the string
            if (strcasecmp(str, temp) == 0){
                // printf("mytmp=:%s:\n", temp);
                // print out to the screen
                remain -= writeline();
                tcp = cp;
            } else if (*tcp == '\n' || *tcp == '\r'){
                // If we hit a newline char or carriage return update cp and tcp
                *tcp++ = 0;
                cp = tcp;
                remain--;
            } else { tcp++; remain--; }
        }
    }
    return 0;
}

int writeline(){
    char *tcp;
    int w, i = 0;
    // move cp past newlines
    while (*cp && (*cp == '\r' || *cp == '\n')) cp++;
    // set tcp
    tcp = cp;
    // move tcp to the end of the current line we want to print
    while (*tcp && (*tcp != '\n' && *tcp != '\r')) {i++; tcp++;}
    // clear tcp of newline and returns
    while (*tcp && (*tcp == '\n' || *tcp == '\r')) { *tcp++ = 0;}

    // make sure we still have a line
    if (!cp) return 0;

    // strncpy(temp, cp, i);
    // Now write out the line
    // w = write(1, temp, sizeof(char) * strlen(temp));
    w = write(1, cp, sizeof(char) * strlen(cp));
    w += write(1, &newline, 1);
    // if not redirected write a newline and carriage return
    if (!stdin || !stdout){
        w += write(1, &cret, 1);
    }
    // Let cp point to tcp now
    cp = tcp;
    return w;
}