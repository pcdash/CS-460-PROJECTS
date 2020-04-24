// wait.c file

extern PROC *running;
extern PROC *sleepList;
extern int int_on(int cspr);
extern int int_off();


int kexit()  // SIMPLE kexit() for process to terminate
{
  printf("proc %d exit\n", running->pid);
  running->status = ZOMBIE;
  tswitch();
}

int ksleep(int event)
{
  // implement this
  int SR = int_off(); //disable IRQ and return CPSR
  //printf("proc %d going to sleep on event=%d\n", running->pid, event);
  running->event = event;
  running->status = SLEEP;
  enqueue(&sleepList, running);
  //printsleepList(sleepList);
  tswitch();

  int_on(SR); //Restore original CPSR
}

int ksleep_kget(int event)
{
  // implement this
  int SR = int_off(); //disable IRQ and return CPSR
  //printf("proc %d going to sleep on event=%d\n", running->pid, event);
  running->event = event;
  running->status = SLEEP;
  enqueue(&sleepList, running);
  //printsleepList(sleepList);
  tswitch();

  int_on(SR); //Restore original CPSR
}

int kwakeup(int event)
{
  int SR = int_off(); // disable IRQ and return CPSR
  
  // implement this
  PROC *temp, *p;
  temp = 0;
  //printList("sleepList", sleepList);

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

  //printList("sleepList", sleepList);
  int_on(SR); //Restore original CSPR
}
