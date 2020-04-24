// kbd.c file

#define KCNTL 0x00
#define KSTAT 0x04
#define KDATA 0x08
#define KCLK  0x0C
#define KISTA 0x10

#include "keymap"
#include "keymap2"

typedef struct kbd{
  char *base;
  char buf[128];
  int head, tail, data, room;
}KBD;

KBD kbd;
int release;
int shifted;
int control;
int event;

int keyset;
int kbd_init()
{
  char scode;
  keyset = 1; // default to scan code set-1
  
  KBD *kp = &kbd;
  kp->base = (char *)0x10006000;
  *(kp->base + KCNTL) = 0x10; // bit4=Enable bit0=INT on
  *(kp->base + KCLK)  = 8;
  kp->head = kp->tail = 0;
  kp->data = 0; kp->room = 128;

  release = 0;
  shifted = 0;
  control = 0;

  printf("Detect KBD scan code: press the ENTER key : ");
  while( (*(kp->base + KSTAT) & 0x10) == 0);
  scode = *(kp->base + KDATA);
  printf("scode=%x ", scode);
  if (scode==0x5A)
    keyset=2;
  printf("keyset=%d\n", keyset);
}

// kbd_handler1() for scan code set 1
void kbd_handler1()
{
  u8 scode, c;
  KBD *kp = &kbd;

  //Get the scan code in data register
  scode = *(kp->base + KDATA);
  
  //Ignore key releases
  if (scode & 0x80){
    release = 1;
    return;
  }
   
  if (release){
    release = 0;
    if ((scode == 0x12 || scode == 0x59)){
      shifted = 0;
    }
    if (scode == 0x14){
      control = 0;
    }
    return;
  }

  //If shifted left or right
  if ((scode == 0x12 || scode == 0x59)){
    shifted = !shifted;
  }

  //If control is pressed. Couldnt test scan code for right side because it breaks out of qemu window
  if (scode == 0x14){
    control = !control;
  }

  if (shifted){
    c = unsh[scode];
  } else{
    c = sh[scode];
  }
  if (control && (c == 'd' || c == 'D')){
    printf("control-D key");
    c = 0x04;
  } else if (control && (c == 'c' || c == 'C')){
    printf("control-C key");
  } else{
    printf("%c", c);
  }
  

  // c = unsh[scode];
  if (c=='\r')
    kputc('\n');
  kputc(c);
  
  kp->buf[kp->head++] = c;
  kp->head %= 128;
  kp->data++; kp->room--;
  kwakeup(&kp->data);
}

// kbd_handelr2() for scan code set 2
void kbd_handler2()
{
  u8 scode, c;
  KBD *kp = &kbd;

  scode = *(kp->base + KDATA);

  if (scode == 0xF0){ // key release 
    release = 1;
    return;
  }
  
  if (release){
    release = 0;
    if ((scode == 0x12 || scode == 0x59)){
      shifted = 0;
    }
    if (scode == 0x14){
      control = 0;
    }
    return;
  }

  // //Enter was pressed.
  // if (scode == 0x5A){
  //   kp->data++;
  //   kwakeup(&kp->data);
  //   kp->data--;
  // }
  
  //If shifted left or right
  if ((scode == 0x12 || scode == 0x59)){
    shifted = !shifted;
  }

  //If control is pressed. Couldnt test scan code for right side because it breaks out of qemu window
  if (scode == 0x14){
    control = !control;
  }

  if (shifted){
    c = utab[scode];
  } else{
    c = ltab[scode];
  }
  if (control && (c == 'd' || c == 'D')){
    printf("control-D key");
    c = 0x04;
  } else if (control && (c == 'c' || c == 'C')){
    printf("control-C key");
  } else{
    printf("%c", c);
  }

  kp->buf[kp->head++] = c;
  kp->head %= 128;
  kp->data++; kp->room--;

  //wakeup sleeping process if there is any
  kwakeup((int)&kp->data);
  // kwakeup(event);
}

void kbd_handler()
{
  if (keyset == 1)
    kbd_handler1();
  else
    kbd_handler2();
}


int kgetc()
{
  char c;
  KBD *kp = &kbd;
  while (kp->data == 0);   // BUSY WAIT for kp>data
  // lock();
  // while (kp->data == 0){
  //   lock();
  //   if (kp->data <= 0){
  //     unlock();
  //     ksleep((int) &kp->data);
  //     // kwakeup(event);
  //     // unlock(); //Enable IRQ interrupts
  //     // event = &kp->data;
  //     // lock();
  //     // ksleep(event); //Sleep on the data
  //     //unlock();
  //   } else{
  //     break;
  //   }
  // }
  lock();
  c = kp->buf[kp->tail++];
  kp->tail %= 128;
  kp->data--; kp->room++;
  unlock();
  return c;
}

int kgets(char s[ ])
{
  char c;
  while( (c = kgetc()) != '\r'){
    if (c=='\b'){
      s--;
      continue;
    }
    *s++ = c;
  }
  *s = 0;
  return strlen(s);
}
