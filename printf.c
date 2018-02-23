#include <stdio.h>
#include <stdlib.h>

int main(int argc, char ** argv){
  printf("Yes, printf program in current PATH works!\n");
  for (int i = 1; i < argc; i++){
    printf("(%s) ",argv[i]);
  }
  printf("\n");
  return EXIT_SUCCESS;
}
