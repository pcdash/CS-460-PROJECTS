// wait.c file

extern PROC *running;
extern PROC *sleepList;
extern int int_on(int cspr);
extern int int_off();

int giveToProc(PROC *parent, PROC *newChild){
  //printf("Collect proc %d\n", newChild -> pid);
  PROC *cur = parent->child;
  if (cur == 0){
    parent -> child = newChild;
    //newChild -> sibling = 0;
    return 0;
  }
  //Put at end of parents sibling list
  while (cur -> sibling){
    cur = cur -> sibling;
  }
  cur -> sibling = newChild;
  //newChild -> sibling = 0;
  return 0;
}

int kexit(int exitValue)  // SIMPLE kexit() for process to terminate
{
  PROC *cur, *pcur;
  int wakeup = 0, i = 0;

  //Dont let PROC 1 die
  if (running -> pid == 1){
    printf("Cannot exit on PROC 1\n");
    return -1;
  }
  //Give away all the children of the current proc
  cur = running -> child;
  if (cur){
    pcur = &proc[1];
    while (cur){
      //Give to process 1
      cur -> parent = pcur;
      cur -> ppid = pcur->pid;
      wakeup = 1;
      // PROC *temp = cur;
      // //Insert to proc 1's list of children
      // giveToProc(&proc[1], temp);
      printf("Give away proc %d to proc %d\n", cur -> pid, cur->ppid);
      cur = cur -> sibling;
    }
    giveToProc(pcur, running->child);
    //Proc 1 has no kids
    // if (pcur -> child == 0){
    //   pcur -> child = running -> child;
    // }

  }

  // for (i = 1; i < NPROC; i++){
  //   cur = &proc[i];
  //   if (cur -> status != FREE && cur -> ppid == running -> pid){
  //     cur -> ppid = 1;
  //     cur -> parent = &proc[1];
  //     wakeup = 1;
  //   }
  // }
  //Update runnings children
  running -> child = 0;
  //Update runnings exit code
  running -> exitCode = exitValue;
  //Set the status to ZOMBIE
  running -> status = ZOMBIE;
  // printf("WAKE UP PARENT PROC %d\n", running -> parent);
  //Wake up the parent
  kwakeup(running -> parent);
  // if (running -> parent == &proc[1]) printf("SHOULD HAVE WOKEN UP %d\n", running -> parent);
  // if (wakeup) printf("  WE ARE WOKE AF LETS GO\n");
  //Wakeup PROC 1 if any children were sent to it
  if (wakeup) kwakeup((int) &proc[1]);
  
  // printf("proc %d exit\n", running->pid);
  //Give up CPU
  tswitch();
}

int ksleep(int event)
{
  // implement this
  int SR = int_off(); //disable IRQ and return CPSR
  // printf("proc %d going to sleep on event=%d\n", running->pid, event);
  running->event = event;
  running->status = SLEEP;
  enqueue(&sleepList, running);
  // printsleepList(sleepList);
  tswitch();

  int_on(SR); //Restore original CPSR
}

int kwakeup(int event)
{
  int SR = int_off(); // disable IRQ and return CPSR
  
  // implement this
  PROC *temp, *p;
  temp = 0;
  // printList("sleepList", sleepList);

  while (p = dequeue(&sleepList)){
    if (p->event == event){
      //printf("wakeup %d\n", p->pid);
      p->status = READY;
      enqueue(&readyQueue, p);
    } else{
      enqueue(&temp, p);
    }
  }
  sleepList = temp;

  // printList("sleepList", sleepList);
  int_on(SR); //Restore original CSPR
}

int kwait(int *status){
  PROC *cur, *prev = 0, *temp;

  //If caller has no child return -1 for error
  if (running -> child == 0) return -1;
  //go through list
  while (1){
    prev = 0;
    cur = running -> child;
    if (cur == 0){
      printf("No more children\n");
      return -1;
    }
    while (cur && cur -> status != ZOMBIE){
      prev = cur;
      cur = cur -> sibling;
    }
    //First child is ZOMBIE
    if (prev == 0 && cur -> status == ZOMBIE){
      printf("PROC %d FOUND A ZOMBIE %d FIRST IN LIST\n", running ->pid, cur->pid);
      running -> child = cur -> sibling;
      *status = cur -> exitCode;
      cur -> sibling = 0;
      cur -> parent = 0;
      cur -> child = 0;
      cur -> status = FREE;
      cur -> priority = FREE;
      enqueue(&freeList, cur);
      return cur -> pid;
    }
    //Other cases
    if (cur && cur -> status == ZOMBIE){
      printf("PROC %d FOUND A ZOMBIE %d\n", running -> pid, cur -> pid);
      prev -> sibling = cur -> sibling;
      *status = cur -> exitCode;
      cur -> sibling = 0;
      cur -> parent = 0;
      cur -> child = 0;
      cur -> status = FREE;
      cur -> priority = FREE;
      enqueue(&freeList, cur);
      return cur -> pid;
    }
    printf("Sleep on %d\n", running);
    ksleep(running);
  }
}