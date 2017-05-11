/*****************************************************
 *                  otp_enc_d
 * Name: Brett Stephenson
 * Class:CS344
 * Program 4
 * Desc: background Daemon for Encryption
 * Source: class slides
 *****************************************************/
#include <fcntl.h>     
#include <netinet/in.h>
#include <stdio.h>     
#include <stdlib.h>    
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#define SIZE 128000

int main(int argc, char *argv[]){
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;
  int n;
  char buffer[SIZE];
  char keyBuffer[SIZE];
  char buffer3[SIZE];
  int i;
  int keyFileSize;
  int inputFileSize;
  int pid;
  int sent;
  
  // Check number of arguments 
  if (argc < 2){
    printf("Usage: otp_enc_d port\n");
    exit(1);
  }
  
  portno = atoi(argv[1]);
  
  // Create socket
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)  {
    printf("Error: opt_enc_d could not create socket\n");
    exit(1);
  }
  
  // zero out the IP address memory space
  memset(&serv_addr, '\0', sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  
  // Bind
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
    printf("Error: otp_enc_d unable to bind socket to port %d\n", portno);
    exit(2);
  }
  
  // listen for connections
  if (listen(sockfd, 5) == -1){
    printf("Error: otp_enc_d unable to listen on port %d\n", portno);
    exit(2);
  }
  clilen = sizeof(cli_addr);
  
  // Open for all connections - loop until finished
  while (1){
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0){
      printf("Error: opt_enc_d unable to accept connection\n");
      continue;
    }
    
    // Start Fork here
    pid = fork();
    
    if (pid < 0){
      perror("opt_enc_c: error on fork\n");
    }
    
    // Child Process
    if (pid == 0){
      
      // zero out buffer
      memset(buffer, 0, SIZE);
      // Get Length and Contents of input file 
      inputFileSize  = read(newsockfd, buffer, SIZE);
      if (inputFileSize  < 0){
        printf("Error: otp_end_d could not read plaintext on port %d\n", portno);
        exit(2);
      }
      
      // send acknowledgement to client
      sent = write(newsockfd, "!", 1);
      if (sent < 0){
        printf("otp_enc_d error sending acknowledgement to client\n");
        exit(2);
      }
      
      // Clear Buffer
      memset(keyBuffer, 0, SIZE);
      
      // Get contents and length of key file 
      keyFileSize = read(newsockfd, keyBuffer, SIZE);
      if (keyFileSize < 0){
        printf("Error: otp_end_d could not read key on port %d\n", portno);
        exit(2);
      }
      
      // Check for bad characters
      for (i = 0; i < inputFileSize ; i++){
        if ((int) buffer[i] > 90 || ((int) buffer[i] < 65 && (int) buffer[i] != 32)){
          printf("otp_enc_d error: plaintext contains bad characters\n");
          exit(1);
        }
      }
      
      // key validation
      for (i = 0; i < keyFileSize; i++){
        if ((int) keyBuffer[i] > 90 || ((int) keyBuffer[i] < 65 && (int) keyBuffer[i] != 32)){
          printf("otp_enc_d error: key contains bad characters\n");
          exit(1);
        }
      }
      // key must be at least as long as input file 
      if (keyFileSize < inputFileSize){ 
        printf("otp_enc_d error: key is too short\n");
        exit(1);
      }
      
      // encryption segement 
      for (i = 0; i < inputFileSize ; i++){
        // change spaces to asterisks
        if (buffer[i] == ' '){
          buffer[i] = '@';
        }
        if (keyBuffer[i] == ' '){
          keyBuffer[i] = '@';
        }
        
        // typecast for easier ASCII maniuplation 
        int inputChar = (int) buffer[i];
        int keyChar = (int) keyBuffer[i];
        
        // change ASCII range 
        inputChar = inputChar - 64;
        keyChar = keyChar - 64;
        
        // Sum and then mod by 27 per program specifications
        int cipherText = (inputChar + keyChar) % 27;
        
        // change characters to capital letters
        cipherText = cipherText + 64;
        
        // and typecast back to characters
        buffer3[i] = (char) cipherText + 0;
        
        // after encryption, change asterisks to spaces
        if (buffer3[i] == '@'){
          buffer3[i] = ' ';
        }
      }
      
      // write encrytped text 
      sent = write(newsockfd, buffer3, inputFileSize);
      if (sent < inputFileSize){
        printf("otp_enc_d error writing to socket\n");
        exit(2);
      }
      
      // close sockets
      close(newsockfd);
      close(sockfd);
      
      exit(0);
    }
    
    else close(newsockfd);
  } 
  
  return 0;
}
