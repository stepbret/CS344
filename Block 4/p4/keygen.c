/*****************************************************
 *                  keygen
 * class: CS344 
 * Name: Brett Stephenson
 * desc: Generates random key with specified length
 *****************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
int randInt(int min, int max);
int main(int argc, char *argv[]){
  char randLetter;
  int i;
  int length;
  time_t sysClock;
  
  // Check number of arguments 
  if (argc < 2){
    printf("Usage: keygen keyLength\n");
    exit(1);
  }
  
  // Get the specified length for the key file
  sscanf(argv[1], "%d", &length);

  if (length < 1){
    printf("keygen: invalid keyLength\n");
    exit(1);
  }
  
  // Seed random number generator
  srand((unsigned) time(&sysClock));
  
  // Get random letters
  for (i = 0; i < length; i++){
    randLetter = (char) randInt(64, 90);
    
    // Special case
    if (randLetter == '@'){
      randLetter = ' ';
    }
    
    printf("%c", randLetter);
  }
  
  printf("\n");
  
  return 0;
}
/*******************************************************
 *               randInt
 *
 * Function used to produce randomness in a certain
 * range.
 ******************************************************/
int randInt(int min, int max){   
    return rand() % (max - min + 1) + min;
}
