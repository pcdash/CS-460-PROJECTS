//Paul Valdez
//Timer c file

#define NPROC 9
#define TLOAD 0x0
#define TVALUE 0x1
#define TCNTL 0x2
#define TINTCLR 0x3
#define TRIS 0x4
#define TMIS 0x5
#define TBGLOAD 0x6
#define DIVISOR 400

typedef struct timer{
    int hh, mm, ss,tick;
    u32 *base;
    char clock[16];
}TIMER;

TIMER timer; //timer structure
char *clock = "00:00:00";

extern int unkpchar(char, int, int);

extern PROC *running;
extern PROC *readyQueue;
extern PROC proc[NPROC];
extern int color;

int flag = 0;

typedef struct tqe{
    struct tqe *next;
    PROC *proc; //Pointer for process
    int time;   // time to expiration
    void (*action)(); //action function = twakeup
} TQE;
TQE *tq = 0, tqe[NPROC];    // tq = timer queue pointer

int printTimer(){
    TQE *cur = tq;
    printf("timerQueue=");
    while(cur){
        printf("[%d, %d]->", cur->proc->pid, cur->time);
        cur =cur->next;
    }
    printf("NULL\n");
}

int itimer(int t){
    lock();
    TQE *newT = &tqe[running->pid], *cur = tq, *prev = 0;
    int temp = t;

    //Fill in TQE[pid] information, e.g. proc pointer, action
    newT -> action = running -> ksp;
    newT -> next = 0;

    //Insert TQE into timer queue at correct position
    while (cur && cur -> time < temp - cur->time){
        temp = temp - cur->time;
        prev = cur;
        cur = cur -> next;
    }
    newT -> time = temp;
    //Not first
    if (prev){
        prev -> next = newT;
        newT -> next = cur;
    } else{
        newT -> next = tq;
        tq = newT;
    }

    //Update all relative times after if they exists
    prev = newT;
    cur = newT->next;
    while (cur){
        cur -> time = cur -> time - temp;
        if (cur->time < 0)
            cur->time = 0;
        temp = cur -> time - temp;

        prev = cur;
        cur = cur->next;
    }
    //Unlock to unmask interrupts
    unlock();
}

int t(){
    int temp = 0;
    flag = 2;
    printf("Enter a time value t in seconds: ");
    temp = geti();
    if (temp <= 0){
        printf("Cannot have negative time: %d\n", temp);
        printf("Auto set to 10\n");
        temp = 10;
        //return -1;
    }
    flag = 0;
    itimer(temp);
    ksleep(running);
}

int handleTQ(){
    TQE *cur = tq, *prev = 0;

    //Make sure list exists
    if (cur){
        cur -> time = cur -> time - 1;
        if (cur -> time <= 0){
            printf("Wakeup proc %d on %d\n",cur->proc->pid, cur-> proc);
            tq = tq->next;
            kwakeup(cur->proc);
        }
    }
}

void timer_handler()
{
    int i, tempC;
    TIMER *t = &timer;
    t->tick++;
    if (t->tick >= DIVISOR){
        t->tick = 0; 
        t->ss++;
        if (t->ss==60){
            t-> ss = 0; 
            t->mm++;
            if (t->mm==60){
                t->mm = 0; 
                t->hh++;
            }
        }
        clock[7]='0'+(t->ss%10); clock[6]='0'+(t->ss/10);
        clock[4]='0'+(t->mm%10); clock[3]='0'+(t->mm/10);
        clock[1]='0'+(t->hh%10); clock[0]='0'+(t->hh/10);
    }
    if (t->tick==0){ //Every second display a wall clock
        tempC = color;
        //1 represents green
        color = 1;
        for (i=0; i<8; i++){
            kpchar(clock[i], 0, 70+i);
        }
        color = tempC;
        handleTQ();
        //Print if not null
        if (tq && flag != 2){
            printTimer();
            flag = 1;
        }
        else if (flag == 1){
            flag = 0;
            printf("timerQueue -> NULL\n");
        }

    }
    timer_clearInterrupt(); // clear timer interrupt
    return;
}

void timer_init()
{
    int i;
    TIMER *t = &timer;
    TQE *tqtemp;
    tq = 0;
    char temp[64];
    printf("timer_init()\n");
    for (i=0; i<NPROC; i++){
        tqtemp = &tqe[i];
        tqtemp->proc = &proc[i];
        tqtemp->time = 0;
        tqtemp->action = 0;
        tqtemp->next = 0;
    }
    tqe[NPROC-1].next = 0; // circular proc list
   
    // set timer base address of versatilepb-A9 board
    t->base = (u32 *) 0x101E2000;
    // set control counter regs to defaults
    *(t->base + TLOAD) = 0x0; // reset
    *(t->base + TVALUE)= 0xFFFFFFFF;
    *(t->base + TRIS) = 0x0;
    *(t->base + TMIS) = 0x0;
    *(t->base + TLOAD) = 0x100;
    //0x62=|011-0010=|NOTEn|Pe|IntE|-|scal=00|1=32-bit|0=wrap|
    *(t->base + TCNTL) = 0x62;
    *(t->base +TBGLOAD) = 0xF0000/DIVISOR;
    t->hh = t->mm = t->ss = t->tick = 0;
    t->tick = DIVISOR;
    //strcpy((char *) timer->clock,"00:00:00");
}

void timer_start(int n) // timer_start(0), 1, etc.
{
    TIMER *t = &timer;
    printf("timer_start\n");
    *(t->base + TCNTL) |= 0x80;
}
int timer_clearInterrupt() // timer_start(0), 1, etc.
{
    TIMER *t = &timer;
    *(t->base + TINTCLR) = 0xFFFFFFFF;
}