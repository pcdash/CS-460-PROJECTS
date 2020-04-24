/****************** exec.c file *******************
* Paul Valdez
* CPTS 460
* Spring 2020
***************************************************/

extern PROC *running, *freeList, *readyQueue;

int my_exec(char *cmdline) //cmdline = VA in uspace
{
    int i, upa, usp;
    char *cp, kline[128], file[32], filename[32];
    PROC *p = running;

    strcpy(kline, cmdline); // fetch cmdline into kernel space
    // get first token of kline as filename
    cp = kline; i = 0;
    while (*cp != ' '){
        filename[i] = *cp;
        i++; cp++;
    }
    filename[i] = 0;
    file[0] = 0;
    if (filename[0] != '/'){ // if filename relative
        strcpy(file,"/bin/"); // prefix with /bin/
    }

    kstrcat(file, filename);
    upa = p->pgdir[2048] & 0xFFFF0000; // PA of UMODE image
    // loader return 0 if file non-exist or non-executable
    // if (!loadelf(file, p))
    //     return -1;
    // copy cmdline to high end of USTACK in UMODE image
    usp = upa + 0x100000 - 128; // assume cmdline len < 128
    strcpy((char *) usp, kline);
    p->usp = (int *) VA(0x100000 - 128);
    // fix syscall frame in kstack to return to VA=0 of new image
    for (i = 2; i < 14; i++) // clear UMODE regs r1-r12
        p->kstack[SSIZE-i] = 0;
    //SHOULD THIS BE INT *?????
    p->kstack[SSIZE-1] = (int)VA(0);  // return uLR = VA(0)
    return (int) p->usp; //will replace saved r0 in kstack
}