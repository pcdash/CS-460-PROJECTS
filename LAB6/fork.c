/****************** fork.c file *******************
* Paul Valdez
* CPTS 460
* Spring 2020
***************************************************/

extern PROC *running, *freeList, *readyQueue;

int my_fork(){
    int i;
    char *PA, *CA;
    PROC *p = dequeue(&freeList);
    if (p == 0){
        printf("Fork failed\n");
        return -1;
    }

    p->ppid = running->pid;
    p->parent = running;
    p->status = READY;
    p->priority = 1;
    PA = (char *)(running->pgdir[2048] & 0xFFFF0000); // parent UMODE PA
    CA = (char *)(p->pgdir[2048] & 0xFFFF0000); // childe UMODE PA
    memcpy(CA, PA, 0x100000); //copy 1MB UMODE image
    for (i = 1; i <= 14; i++){ //copy bottom 14 entries of kstack
        p->kstack[SSIZE-1] = running->kstack[SSIZE-1];
    }
    // for (i = 15; i <= 28; i++){
    //     p->kstack[SSIZE-i] = 0;
    // }

    p->kstack[SSIZE-14] = 0;        // child return pid = 0
    p->kstack[SSIZE-15] = (int)goUmode; //child resumes to goUmode
    p->ksp = &(p->kstack[SSIZE-28]);    // child saved ksp
    p->usp = running->usp;          //Same ups as parent
    p->cpsr = running -> cpsr;      //Same cpsr as parent
    p->upc = running -> upc;
    enqueue(&readyQueue, p);
    return p->pid;

}
