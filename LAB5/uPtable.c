/*********************** uPtable.c file ************************
 * Paul Valdez
 * CPTS 460
 * ************************************************************/

extern PROC proc[], *running;
extern int tswitch();

//Builds a page table for process p
int uPtable(PROC *p){
    int i, pentry, *ptable;
    //Each proc has a ptable at 4MB + (pid)*16KB
    ptable = 0x400000 + (p->pid)*(0x4000);
    pentry = 0 | 0x412;
    //Zero out the ptable
    for (i=0; i < 4096; i++){
        ptable[i] = 0;
    }

    //Fill 128 MB of ram
    for (i=0; i < 128; i++){
        ptable[i] = pentry;
        pentry += 0x100000;
    }

    // 2MB I/O space at 256 MB
    pentry = 256*0x100000 | 0x412;
    for (i=256; i<258; i++){
        ptable[i] = pentry;
        pentry += 0x100000;
    }

    //Each proc has a 1MB Umode image
    ptable[2048] = (0x800000 + (p->pid -1)*0x100000) | 0xC32;
    p->pgdir = ptable;
    // p->pgdir[0] = p->pgdir[2048];
}
