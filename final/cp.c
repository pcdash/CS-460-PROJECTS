/******************************** cat.c file ***********************************
 * Paul Valdez
 * CPTS 460
 * 
 * *******************************************************************************/

#include "ucode.c"
#define printk printf

#define BLKSIZE 4096

char buf[BLKSIZE], c;
int fd, gd, n;

int main(int argc, char *argv[ ]){
    int i;

    // defaults
    fd = 0;
    gd = 0;

    // Check if we have enough args
    if (argc > 2){
        fd = open(argv[1], O_RDONLY);
        gd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC);
    } else{
        printf("cp usage: <src file> <dest file>\n");
        exit(1);
    }

    // Make sure the files were open
    if (fd < 0){
        printf("Cannot open file: %s\n", argv[1]);
        exit(1);
    }
    if (gd < 0){
        printf("Cannot open file: %s\n", argv[2]);
        exit(1);
    }

    // Now read from fd and write to gd
    while (n = read(fd, buf, BLKSIZE * sizeof(char))){
        buf[n] = 0;
        write(gd, buf, sizeof(char) * strlen(buf));
    }
    // while (n = read(fd, &c, 1)){
    //     write(gd, &c, 1);
    // }
    // now close files
    close(fd);
    close(gd);

    return 0;
}