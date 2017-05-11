#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ftw.h>


/***********************************************************************************
 *                                  createDir
 * Desc: creates a directory with the specified name and process
 * Param: array of char to have something to do
 * ********************************************************************************/
void createDir(char *empty){
  char dirName[25] = "stepbret.rooms.";
  int pid = getpid();
  char spid[8];
  sprintf(spid, "%d", pid);
  strcat(dirName, spid);
  strcpy(empty, dirName);
  mkdir(dirName, 0777);
}


/**********************************************************************************
 *                                  makeRooms
 * Desc: Makes the rooms within the directory
 * Param: name for the directory to make the rooms, room names, and types of room
 * *******************************************************************************/
void makeRooms(char *dirName, char *rStart){
 //names of rooms
 char rNames [10] [20];
 strcpy(rNames[0], "Kitchen");
 strcpy(rNames[1], "Ballroom");
 strcpy(rNames[2], "Conservatory");
 strcpy(rNames[3], "BillardRoom");
 strcpy(rNames[4], "Library");
 strcpy(rNames[5], "Study");
 strcpy(rNames[6], "Hall");
 strcpy(rNames[7], "Lounge");
 strcpy(rNames[8], "DinningRoom");
 strcpy(rNames[9], "Cellar");


 //randomization for the first rooms
 int pick[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
 int pick2[7];
 int i = 0;
 int h = 0;
 int max = 9;
 for(h; h < 7; h++){
   int new = rand() % 10;
   int tmp = pick[new];
   pick[new] = pick[max];
   pick[max] = tmp;
   max--;
 }


 //randoms for the second rooms
 int h2 = 0;
 for(h2; h2 < 7; h2++){
   pick2[h2] = pick[h2];
 }
 h2 = 0;
 max = 6;
 for(h2; h2 < 7; h2++) {
   int new = rand() % 7;
   int tmp = pick2[new];
   pick2[new] = pick2[max];
   pick2[max] = tmp;
   max--;
 }

 time_t t;
 srand((unsigned) time(&t));
 //types of rooms
 char rType[3][15];
 strcpy(rType[0], "START_ROOM\n");
 strcpy(rType[1], "MID_ROOM\n");
 strcpy(rType[2], "END_ROOM\n");
 char dirTemp[60];
 int eFlag = 0;
 int sFlag = 0;

 //loop for creating the filenames and where to put them
 for(i; i < 7; i++) {
   strcpy(dirTemp, dirName);
   strcat(dirTemp, "/");
   strcat(dirTemp, rNames[pick[i]]);
   int fileDesc;
   ssize_t nwritten;

   //creates the file or throws error
   fileDesc = open(dirTemp, O_WRONLY | O_CREAT, 0777);
   if(fileDesc < 0) {
     fprintf(stderr, "Couldn't open or create %s\n", dirName);
     perror("in main");
     exit(1);
   }
   
   //Template for what is in the room
   char template[1000] = "ROOM NAME: ";
   strcat(template, rNames[pick[i]]);
   strcat(template, "\n");
   int j = 0;
   int number;
   number = rand() % 4 + 3;

   //places the randomly chosen number of rooms in the file
   for(j; j < number; j++){
     char con[] = "CONNECTION ";
     int h = j + 1;
     char hNum[2];
     sprintf(hNum, "%d", h);
     strcat(template, con);
     strcat(template, hNum);
     strcat(template, ": ");

     //randomly choose the names of the rooms 
     strcat(template, rNames[pick2[j]]);
     strcat(template, "\n");
   }

  //The first room created is the start room, the second room is the end room
  //all others are mid rooms
  strcat(template, "\nROOM TYPE: ");
  if(sFlag == 0 && eFlag == 0 ){
    strcat(template, rType[0]);
    sFlag = 1;
    strcpy(rStart, dirTemp);
  }
  else if(sFlag == 1 && eFlag == 0){
    strcat(template, rType[2]);
    eFlag = 1;
  }
  else{
    strcat(template, rType[1]);
  }
  nwritten = write(fileDesc, template, strlen(template) * sizeof(char));
  }
}


/********************************************************************************
 *                              EndGame()
 * Description: ends the game with the message, pulls the file to give steps and
 *              rooms
 * ******************************************************************************/


void endGame(int steps, char *dir) {
  FILE *fp;
  char dirCpy[30];
  strcpy(dirCpy, dir);
  strcat(dirCpy, "/trackFile.txt");
  printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
  printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", steps);
  fp = fopen(dirCpy, "r");
  char rooms[30];
  while(fgets(rooms, 30, fp) != NULL){
    printf("%s\n", rooms);
  }
  
}


/*******************************************************************************
 *                              playGame()
 * Description: Plays the game
 * Params: takes the starting point, adnt he current directio to read the
 *          files
 * *****************************************************************************/
void playGame(char *dir, char *rStart){
  //sets up everything for the loop
  int endFlag = 1;
  FILE *fp;
  FILE *tmp;
  char room[20];
  char rType[15];
  char current[20];
  char con[20];
  char possibles[6][20];
  char tmpFile[20];
  strcpy(tmpFile,dir);
  strcat(tmpFile, "/trackFile.txt");
  tmp = fopen(tmpFile, "w");
  strcpy(room, rStart);
  int steps = 0;
  do {
    //open file, see if it is the end of the game
    fp = fopen(room, "r");
    fseek(fp, -9, SEEK_END);
    fgets(rType, 9, fp);
    if(strcmp(rType, "END_ROOM") == 0){
      endGame(steps, dir);
      endFlag = 0;
    }
    //if not, continue
    else{
      steps++;
      fseek(fp, 10, SEEK_SET);
      fgets(current, 20, fp);
      printf("CURRENT LOCATION: %s", current);
      //open another file to keep track of path
      fprintf(tmp, current);
      printf("POSSIBLE CONNECTIONS: ");
      char test[20];
      fgets(test, 11, fp);
      int tFlag = 0;
      int count = 0;

      //prints the next possible locations
      while (tFlag == 0){
        fseek(fp, 3, SEEK_CUR);
        fgets(con, 20, fp);
        size_t len = strlen(con);

        //chomps the new lines at the end
        if(len > 0 && con[len-1] == '\n'){
          con[--len] = '\0';
        }
        strcpy(possibles[count], con);
        count++;
        fgets(test, 11, fp);

        //if it == connection then don't put a ',' I'm sure there is a better way
        if(strcmp(test, "CONNECTION") != 0){
          printf("%s.\n", con);
          tFlag = 1;
        }
        else{
          printf("%s, ", con);
        }
      }

      //prompt for user and get input
      int confused = 1;
      while(confused == 1){
        printf("WHERE TO? >");
        char input[20];
        char input2[20];
        strcpy(input, " ");
        scanf("%s", input2);
        strcat(input, input2);

        //compare input to possibilities
        int i = 0;
        int mark;
        int check = 0;
        for(i; i < count; i++){
          if(strcmp(possibles[i], input) == 0){
            mark = i;
            check = 1;
          }
        }

        //if the room exists, move on, if not, keep trying
        if(check){
          strcpy(room, possibles[mark]);
          confused = 0;
        }
        else{
          printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
        }
      }

      //clean up the space at the front
      char *token;
      const char s[2] = " ";
      token = strtok(room, s);

      //put it all together and change the room
      char tmpDir[30];
      strcpy(tmpDir, dir);
      strcat(tmpDir, "/");
      strcat(tmpDir, token);
      strcpy(room, tmpDir);
    }
  } while (endFlag);
  fclose(tmp);
}

int main(int argc, char* argv[]) {
  char dir[25];
  char rStart[20];
  createDir(dir);
  makeRooms(dir, rStart);
  int endFlag = 0;
  playGame(dir, rStart);
  return 0;

}
