// kernel.c file
extern int kexit();

#define NPROC 9
PROC proc[NPROC], *running, *freeList, *readyQueue;
PROC *sleepList;
int procsize = sizeof(PROC);
int body();
int markKFORK;

int init()
{
  int i, j; 
  PROC *p;
  kprintf("kernel_init()\n");
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->status = READY;
    p->next = p + 1;
    p->parent = 0;
    p->child = 0;
    p->sibling = 0;
  }
  proc[NPROC-1].next = 0; // circular proc list

  freeList = &proc[0];
  readyQueue = 0;
  sleepList = 0;
  
  p = dequeue(&freeList);
  p->priority = 0;
  p->ppid = 0;
  //Set the beginning of the tree
  p->sibling = 0;
  p->child = 0;
  p->parent = p;

  running = p;

  kprintf("running = %d\n", running->pid);
  printList("freeList", freeList);
  markKFORK = 0;
}

  
int kfork(int func, int priority)
{
  int i;
  PROC *p = dequeue(&freeList);
  if (p==0){
    printf("no more PROC, kfork failed\n");
    return 0;
  }

  p->status = READY;
  p->priority = priority;
  p->ppid = running->pid;
  p->parent = running;
  p->child = 0;
  p->sibling = 0;

  //Enter p into runnings child list
  giveToProc(running, p);
  
  // set kstack for new proc to resume to func()
  // stack = r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r14
  //         1  2  3  4  5  6  7  8  9  10 11  12  13  14
  for (i=1; i<15; i++)
      p->kstack[SSIZE-i] = 0;
  p->kstack[SSIZE-1] = (int)func;  // in dec reg=address ORDER !!!
  p->ksp = &(p->kstack[SSIZE-14]);
  enqueue(&readyQueue, p);

  printf("proc %d kforked a child %d\n", running->pid, p->pid);
  printList("readyQueue", readyQueue);
  return p->pid;
}

int scheduler()
{
  //  kprintf("proc %d in scheduler ", running->pid);
  if (running->status == READY)
     enqueue(&readyQueue, running);
  running = dequeue(&readyQueue);
  //kprintf("next running = %d\n", running->pid);
  if (running->pid){
    color = running->pid;
  }
}  
int body()
{
  char c, cmd[64];
  int event, status;

  printf("proc %d resume to body()\n", running->pid);
  while(1){
    printf("-------- proc %d running -----------\n", running->pid);
    
    printList("freeList  ", freeList);
    printList("readyQueue", readyQueue);
    printsleepList(sleepList);
    printChildren(running);
    printTimer();
    printf("Enter a command [switch|kfork|sleep|wakeup|wait|t|exit] : ", running->pid);
    kgets(cmd);

    if (strcmp(cmd, "switch")==0){
       tswitch();
    }
    else if (strcmp(cmd, "kfork")==0){
      markKFORK = 1;
      event = kfork((int)body, 1);
      kwakeup(event);
      //kfork(0, 1);
    }
    else if (strcmp(cmd, "sleep")==0){
      printf("Enter a value to sleep on : ");
      event = geti();
      ksleep(event);
    }
    else if (strcmp(cmd, "wakeup")==0){
      printf("Enter a value to wakeup with : ");
      event = geti();
      kwakeup(event);
    }
    else if (strcmp(cmd, "wait") == 0){
      event = kwait(&status);
      if (event == -1){
        printf("PROC %d has no child to wait for.\n", running->pid);
      } else{
        printf("PROC %d waited for a ZOMBIE child with exitValue %d\n", running->pid, status);
      }
    }
    else if (strcmp(cmd, "t") == 0){
      t();
    }

    else if (strcmp(cmd, "exit")==0){
      //Exit with running PID value
       kexit(running -> pid);
    }
    cmd[0] = 0;
  }
}
