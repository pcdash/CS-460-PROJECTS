/******************************** more.c file ***********************************
 * Paul Valdez
 * CPTS 460
 * 
 * *******************************************************************************/

#include "ucode.c"
#define printk printf

#define BLKSIZE 4096
// Counted the screen size, its around 24 lines high and 80 wide
#define PHEIGHT 23
#define PWIDTH 80

int writepage();
int writeline();
int endread();

char cret = '\r', newline = '\n';
int fd, gd, remain, stdin;
char buf[BLKSIZE], *cp;

int main(int argc, char *argv[ ]){
    int r, n, s, t;
    struct stat sp, myst0;
    char mytty[64], c;

    printf("#####################################\n");
    printf("## This is PAUL's MORE in progress ##\n");
    printf("#####################################\n");

    // Set remain to default 0
    remain = 0;

    //Set default redirection
    stdin = 0;

    // set default file descriptor
    fd = 0;

    // get tty
    gettty(mytty);
    // stat
    stat(mytty, &sp);
    // We send in file descriptor 0 because thats stdin
    fstat(0, &myst0);

    // Check if stdout is redirected
    if (sp.st_ino != myst0.st_ino) stdin = 1;

    // Check if a file to read from is passed in
    if (argc > 1){
        fd = open(argv[1], O_RDONLY);
    } else if (stdin){
        // printf("OPENING OWN TERMINAL\n");
        fd = dup(0);
        close(0);
        gd = open(mytty, O_RDONLY);
    }
    // otherwise fd = regular 

    // Make sure we opened up the write to file descriptor
    if (fd < 0){
        printf("Cannot open file: %s\n", argv[1]);
        exit(1);
    }

    // write the first page if its not our own terminal
    if (fd || stdin){
        r = writepage();

        // Run so long as program is not quit
        while (1){
            // What to do if we have reached the end
            if (r <= 0) break;
            read(0, &c, 1);
            // Get user input on what to do
            switch(c){
                // case of a space we want to print a page
                case ' ':
                    r = writepage();
                    break;
                case '\r':
                    r = writeline();
                    c = 'a';
                    break;
                case '\n':
                    r = writeline();
                    c = 'a';
                    break;
            }
            // Check if control d was pressed and work only if stdout was not redirected
            if (c & 0x4 || c == EOF){
                if (!stdin) break;
                else { endread(); break; }
            }
        }
    } else{
        // Run through getting a character each time the user presses it
        // while (c = getc()){
        while (read(fd, &c, 1)){
            // check for specials
            switch(c){
                // case of a newline we dont want to print anything
                case '\n':
                    write(1, &c, 1);
                    c = 'a';
                    break;
                case '\r':
                    write(1, &c, 1);
                    c = 'a';
                    break;
                default:
                    write(1, &c, 1);
            }
            // Check if the control d was pressed
            if (c == EOF) break;
        }
    }
    // make sure that they are closed
    if (stdin) close(gd);
    if (fd) close(fd);
    return 0;
}

int writepage(){
    int ret, i;
    // set r to default
    ret = 0;

    // Just write a line for the whole screen size
    for (i = 0; i < PHEIGHT; i++){
        ret = writeline();
        if (ret <= 0){
            break;
        }
    }
    return ret;
}

// int writeline(){
//     int r, s, t, n, i;
//     char c;
//     i = 0;
//     end = 0;
//     while (n = read(fd, &c, 1) && i++ < PWIDTH){
//         write(1, &c, 1);
//         switch(c){
//             case '\r':
//                 s = write(1, &newline, 1);
//                 t = write(1, &cret, 1);
//                 end = 1;
//                 break;
//             case '\n':
//                 // s = write(1, &newline, 1);
//                 t = write(1, &cret, 1);
//                 end = 1;
//                 break;
//         }
//         if (end) break;
//     }
//     return n;
// }
int writeline(){
    int i = 0, end = 0;
    char c;

    // Want to read in a blk if there is none
    if (remain == 0){
        buf[0] = 0;
        remain = read(fd, buf, BLKSIZE * sizeof(char));
        // if remain is 0, nothing was read in and we can return 0
        if (remain <= 0) return remain;
        // Let cp point to buf again
        cp = buf;
    }

    // Go through the block and write while we havent hit the ends
    while (i++ < PWIDTH && remain){
        remain--;
        c = *cp++;
        // Write to fd
        write(1, &c, 1);

        // if std was redirected do NOT put a carriage return, just break
        if (stdin && (c == '\r')) break; 

        // Check for new lines, if we hit one we end
        switch(c){
            case '\n':
                write(1, &cret, 1);
                end = 1;
                break;
            case '\r':
                write(1, &cret, 1);
                write(1, &newline, 1);
                end = 1;
                break;
        }
        if (end) break;
    }
    // Want to return i
    return i;
}

// What does end read do? If finishs reading from the file so that we can close it and exit
int endread(){
    int n;
    while (n = read(fd, buf, BLKSIZE * sizeof(char)));
    return n;
}