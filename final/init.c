/******************************** init.c file ***********************************
 * Paul Valdez
 * CPTS 460
 * 
 * *****************************************************************************/

#include "ucode.c"
#define printk printf

int console, s0, s1;

int parent(){
  int pid, status;
  while(1){
    // printf("INIT : Wait for ZOMBIE child\n");
    pid = wait(&status);
    if (pid == console){ // login child died
      printf("PC INIT forks a new login on console\n");
      console = fork();
      if (console){
        continue;
      }
      else{   // child
        exec("login /dev/tty0");
      }
    }
    if (pid == s0){ // login child died
      printf("PC INIT forks a new login on s0\n");
      s0 = fork();
      if (s0){
        continue;
      }
      else{   // child
        exec("login /dev/ttyS0");
      }
    }
    if (pid == s1){ // login child died
      printf("PC INIT forks a new login on s1\n");
      s1 = fork();
      if (s1){
        continue;
      }
      else{   // child
        exec("login /dev/ttyS1");
      }
    }
    printf("PC INIT buried an orphan proc %d\n", pid);	
  }
  return 0;
}

int main(int argc, char *argv[ ])
{
  int in, out;
  int pid, status;
  
  in  = open("/dev/tty0", O_RDONLY);
  out = open("/dev/tty0", O_WRONLY);

  pid = getpid();

  printf("#####################################\n");
  printf("## This is PAUL's INIT in progress ##\n");
  printf("#####################################\n");

  // printf("proc %d in umode", getpid());
  // printf("argc=%d, argv=%s %s\n", argc, argv[0], argv[1]);

  // printf("PC INIT %d: fork a login proc on console\n", pid);
  console = fork();
  if (console){ // parent
    // Fork for other logins
    s0 = fork();
    if (s0){
      s1 = fork();
      if (s1){
        parent();
      } else{
        exec("login /dev/ttyS1");
      }
    } else{
      exec("login /dev/ttyS0");
    }
  } else{
    // Childe exec to login on tty0
    exec("login /dev/tty0");
  }
  return 0;
}
