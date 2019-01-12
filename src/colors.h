#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//color codes
#define KNRM  "\x1B[0m"    //normal
#define KRED  "\x1B[31m"   // red
#define KGRN  "\x1B[32m"   // green
#define KYEL  "\x1B[33m"   // yellow
#define KBLU  "\x1B[34m"   //blue
#define KMAG  "\x1B[35m"   // magenta
#define KCYN  "\x1B[36m"   // cyan
#define KWHT  "\x1B[37m"   //white
#define BOLD  "\e[1m"      //Bold font

char* color = "";

void paint() {
    int random = rand() % 5;
    switch(random) {
        case 1:color = KGRN;break;
        case 2:color = KCYN;break;
        case 3:color = KBLU;break;
        case 4:color = KMAG;break;
        case 5:color = KWHT;break;
    }
}
