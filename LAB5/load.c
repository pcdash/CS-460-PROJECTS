/*********************** load.c file ************************
 * Paul Valdez
 * CPTS 460
 * ************************************************************/

#define BLK 1024

char buf1[BLK], buf2[BLK];

//Search function from LAB 1 booter
u16 search(INODE *ip, char *name){
  int i;
  char c;
  DIR *dp;
  for (i = 0; i < 12; i++){
    if ((u16) ip->i_block[i]){
      getblock((u16) ip->i_block[i], buf2);
      dp = (DIR *) buf2;
      while ((char *) dp < &buf2[BLK]){
        c = dp->name[dp->name_len];
        dp->name[dp->name_len] = 0;
        printf(dp->name);
        kputc(' ');
        if (strcmp(dp->name, name) == 0){
          printf("found %s\n\r", name);
          return ((u16) dp->inode);
        }
        dp -> name[dp->name_len] = c;
        dp = (char *) dp + dp->rec_len;
      }
    }
  }
  return 0;
}

int load(char *filename, PROC *p){
    int i, blk, iblk, ino, bytesLoaded = 0;
    char *cp, *name[2];
    u32 *up;
    GD *gp;
    INODE *ip;
    DIR *dp;

    //Want to look for bin/filename
    name[0] = "bin";
    name[1] = filename;

    //Want to load in ptables Umode image area
    u32 paddr = (0x800000 + (p->pid - 1)*0x100000);

    //Read blk #2
    getblock(2, buf1);
    gp = (GD *)buf1;

    //Inodes begin blk
    iblk = (u16) gp->bg_inode_table;

    //Get root inode
    getblock(iblk, buf1);
    ip = (INODE *)buf1 + 1;

    //Step through data block to find filename
    for (i=0; i < 2; i++){
        ino = search(ip, name[i]) - 1;
        if (ino < 0) return 0;
        getblock(iblk+(ino/8), buf1);
        ip = (INODE *)buf1 + (ino % 8);
    }

    //Read indirect blocks into buf2
    if (ip->i_block[12]){
        getblock((u16) ip->i_block[12], buf2);
    }
   
    //We cannot set ES here because we dont have it
    //So we want to go through and set direct as well as indirect names into the proc
    for (i=0; i < 12; i++){
        if (ip->i_block[i] == 0) break;
        getblock((u16) ip->i_block[i], (char *) paddr); kputc('*');
        paddr += 1024;
        bytesLoaded += 1024;
    }
    if (ip->i_block[12]){
        up = (u32 *) buf2;
        while (*up){
            getblock((u16) *up, (char *) paddr); kputc('.');
            paddr += 1024;
            bytesLoaded += 1024;
            up++;
        }
    }
    printf("\n%d bytes loaded\n", bytesLoaded);
    if (ino > 0) return ino;
    return 0;
}
