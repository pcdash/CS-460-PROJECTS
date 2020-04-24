/******************************** login.c file ***********************************
 * Paul Valdez
 * CPTS 460
 * 
 * *******************************************************************************/

#include "ucode.c"
#define printk printf
#define PATHSIZE 256 //size = 4kb = 1024
#define BUFSIZE 1024

int stdin, stdout, stderr;

int tokenize(char *line, char *buf[], char delim);
int remove_white(char *str);

int main(int argc, char *argv[ ]){
    int fd, n, myargc, logc, i;
    char buf[BUFSIZE], *myargv[PATHSIZE], username[PATHSIZE], password[PATHSIZE], *loginfo[PATHSIZE], temp[PATHSIZE];

    // close stdin and stdout to replace them with own terminals, tty which is contained in argv[1]
    close(0);
    close(1);
    stdin = open(argv[1], 0);
    stdout = open(argv[1], 1);
    stderr = open(argv[1], 2);

    // set the tty in proc.tty
    settty(argv[1]);

    printf("######################################\n");
    printf("## This is PAUL's LOGIN in progress ##\n");
    printf("######################################\n");

    // open the password file
    fd = open("/etc/passwd", O_RDONLY);

    //Make sure it was opened
    if (fd < 0){
        printf("Error: cannot login\n");
        exit(1);
    }
    //Read in to buf from fd
    n = read(fd, buf, BUFSIZE);
    buf[n] = 0;

    // Separate the buf into strings
    logc = tokenize(buf, loginfo, '\n');

    // Try to login now
    while (1){
        // display to own stdout login to get username and password
        printf("login: ");
        gets(username);
        printf("password: ");
        gets(password);

        // Remove whitespace
        remove_white(username);
        remove_white(password);

        // Now look through the login info to see if the username and password are in there
        for (i = 0 ; i < logc; i++){
            strcpy(temp, loginfo[i]);
            // tokenize the current login line
            myargc = tokenize(temp, myargv, ':');

            // Check to see if username and password match
            if (strcmp(username, myargv[0]) == 0 && strcmp(password, myargv[1]) == 0){
                // Change uid and gid because P2 becomes the users process by taking on the users uid
                chuid(atoi(myargv[2]), atoi(myargv[3]));
                
                // Move to users home directory
                chdir(myargv[5]);
                // close the account info file
                close(fd);
                
                // Do exec on loginfo listed program, which is usually sh
                exec(myargv[6]);
                break;
            }
        }
        // If we reach here, login has failed
        printf("Login failed: Invalid username or password.\n");
    }
    // if canceled, close file
    close(fd);
    return 0;
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

