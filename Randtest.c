#include <stdio.h>
#include <stdlib.h>

main(int argc, char ** argv){
int i;
srand(10);
for(i=0; i<5; i++) {
printf("%d\n", rand());
}
}
