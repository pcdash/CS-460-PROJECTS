/******************************** ls.c file ***********************************
 * Paul Valdez
 * CPTS 460
 * 
 * *******************************************************************************/

#include "ucode.c"

#define printk printf
#define PATHSIZE 256 //size = 4kb = 1024
#define BUFSIZE 1024

int ls_dir(char *dname);
int ls_file(char *fname);
char *basename(char *fname);
int tokenize(char *line, char *buf[], char delim);
int remove_white(char *str);

char *t = "xwrxwrxwr-------";
struct stat mystat, *sp;

int main(int argc, char *argv[ ]){
    int r;
    struct stat mystat, *sp;
    char *filename, path[PATHSIZE], cwd[PATHSIZE];
    sp = &mystat;
    
    printf("###################################\n");
    printf("## This is PAUL's LS in progress ##\n");
    printf("###################################\n");

    //default
    filename = "./";
    // Check if a filename was entered
    if (argc > 1){
        filename = argv[1];
    }
    // Remove any whitespace from filename
    remove_white(filename);

    // stat filename
    r = stat(filename, sp);
    if (r < 0){
        printf("stat failed on %s.\n", filename);
        exit(1);
    }
    strcpy(path, filename);
    // check if filename is relative
    if (path[0] != '/'){
        getcwd(cwd);
        strcpy(path, cwd);
        strcat(path, filename);
    }
    // printf("GOING TO STAT ::%s::\n", path);
    // Chcek if its a directory or file
    if ((sp->st_mode & 0xF000) == 0x4000){
        ls_dir(path);
    } else{
        ls_file(path);
    }
    return 0;
}

int ls_file(char *fname){
    int r, i;
    char ftime[64], buf[PATHSIZE], temp[PATHSIZE];
    struct stat fstat, *sp;
    
    // Check if relative
    if (*fname != '/'){
        getcwd(temp);
        if (*(temp + strlen(temp) - 1) != '/')
            strcat(temp, "/");
        strcat(temp, fname);
        strcpy(fname, temp);
    }
    sp = &fstat;
    r = stat(fname, sp);
    // Check if stat worked
    if (r < 0){
        printf("stat failed on %s.\n", fname);
        exit(1);
    }

    // Now go through everything
    if ((sp->st_mode & 0xF000) == 0x8000){ // if S_ISREG
        printf("%s", " -");
    }
    if ((sp->st_mode & 0xF000) == 0x4000){ // if S_ISDIR
        printf("%s", " d");
    }
    if ((sp->st_mode & 0xF000) == 0xA000){ // if S_ISLNK
        printf("%s", " l");
    }

    // print permissions
    for (i = 8; i >= 0; i--){
        if (sp->st_mode & (1<<i)){
            printc(t[i]); // print r|x|w
        } else{
            printc('-');
        }
    }
    // print other info
    printf(" %d ", sp->st_nlink); // link count
    printf(" %d ", sp->st_gid);   // gid
    printf(" %d ", sp->st_uid);   // uid
    printf(" %d ", sp->st_size);  // file size

    // print time of access and changes and such
    // strcpy(ftime, tim)
    // WE DONT HAVE A TIME ACCESS RIGHT NOW? CAN I ADD ONE???

    // print name
    printf("%s", basename(fname)); // print basename
    
    // print linkname if symbolic
    if ((sp->st_mode & 0xF000) == 0xA000){
        // read link
        readlink(fname, buf);
        printf(" -> %s", buf); // print linked name
    }
    printf("\r\n");
    return 0;
}

int ls_dir(char *dname){
    // We want to open the directory, read it, then call ls_file(name)
    char buf[BUFSIZE], temp[PATHSIZE], *cp, fname[PATHSIZE];
    int i, r, fd;
    DIR *dp;

    fd = open(dname, O_RDONLY);
    if (fd < 0){
        printf("Error: Couldn't open %s.\n", dname);
        exit(1);
    }
    // Read the directory in 
    r = read(fd, buf, BUFSIZE);
    if (r < 0){
        printf("Error: Couldn't read %s.\n", dname);
        close(fd);
        exit(1);
    }

    cp = buf;
    dp = (DIR *) buf;
    // run through directory
    while (cp < &buf[PATHSIZE]){
        strncpy(temp, dp->name, dp->name_len);
        //Set to zero because the names arent true strings ended in null
        temp[dp->name_len] = 0;
        // strip whitespace from temp
        remove_white(temp);

        // build name
        strcpy(fname, dname);
        if (fname[strlen(fname) - 1] != '/')
            strcat(fname, "/");
        strcat(fname, temp);
        //Run ls on the name
        ls_file(fname);
        cp += dp->rec_len;
        dp = (DIR *) cp;
    }

    // close file descripter
    close(fd);
    return 0;
}

char *basename(char *fname){
    int myargc;
    char temp[PATHSIZE], *myargv[PATHSIZE];
    // make sure we dont try it on root
    if (strcmp(fname, "/")==0){
        return fname;
    }
    // Copy into temp because tokenize will destroy it
    strcpy(temp, fname);
    myargc = tokenize(temp, myargv, '/');
    // Check and make sure name was not all spaces or /
    if (myargc == 0){
        return "/";
    } else{
        return myargv[myargc-1];
    }
}



int tokenize(char *line, char *buf[], char delim){
    char *s = line;
    int n = 0, i;
    while (*s){
        //Move past whitespace
        while (*s == ' ' || *s == delim) *s++ = 0;
        if (*s){
            buf[n++] = s;
        }
        while (*s && *s != delim) s++;
        //check if we hit delim or end of line
        if (*s) *s = 0;
        else break;
        s++;
    }
    for (i = 0; i < n; i++){
        remove_white(buf[i]);
    }
    buf[n] = 0;
    return n;
}

// removes whitespace and newlines
int remove_white(char *str){
    if (str == 0) return 0;
    int length = strlen(str) - 1;
    // remove from end
    while (*(str + length) == ' ' || *(str + length) == '\n'){
        *(str + length) = 0;
        length--;
    }
    // remove from front
    while (*str == ' '){
        str++;
    }
    return 0;
}

