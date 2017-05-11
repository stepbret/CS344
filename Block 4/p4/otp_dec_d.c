/********************************************************
 *                  otp_dec_d.c
 * Name: Brett Stephenson
 * Class: CS344
 * Desc: Daemon that will run and decrpyt the files 
 *      sent to it
 ********************************************************/
#include <fcntl.h>     
#include <netinet/in.h>
#include <stdio.h>     
#include <stdlib.h>   
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#define SIZE    16000
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
  int sent;
  int pid;
  int encryptFileSize;
  
  // Check number of arguments 
  if (argc < 2){
    printf("Usage: otp_dec_d port\n");
    exit(1);
  }
  
  portno = atoi(argv[1]);
  // Start socket 
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    printf("Error: otp_dec_d could not create socket\n");
    exit(1);
  }
  
  memset(&serv_addr, '\0', sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  
  // bind process 
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
    printf("Error: otp_dec_d unable to bind socket to port %d\n", portno);
    exit(2);
  }
  
  if (listen(sockfd, 5) == -1){
    printf("Error: otp_dec_d unable to listen on port %d\n", portno);
    exit(2);
  }
  clilen = sizeof(cli_addr);
  
  // Loop for all connections until finished 
  while (1){
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0){
      printf("Error: otp_dec_d unable to accept connection\n");
      continue;
    }
    
    // Fork starts here 
    pid = fork();
    
    if (pid < 0){
      perror("otp_dec_c: error on fork\n");
    }
    
    // child process
    if (pid == 0){
      memset(buffer, 0, SIZE);
      
      // read encrypted file 
      encryptFileSize = read(newsockfd, buffer, SIZE);
      if (encryptFileSize < 0){
        printf("Error: otp_end_d could not read ciphertext on port %d\n", portno);
        exit(2);
      }
      // send acknowledgement to client
      sent = write(newsockfd, "!", 1);
      if (sent < 0){
        printf("otp_enc_d error to client\n");
        exit(2);
      }
      // clear buffer 
      memset(keyBuffer, 0, SIZE);
      
      // read key file 
      keyFileSize = read(newsockfd, keyBuffer, SIZE);
      if (keyFileSize < 0)
      {
        printf("Error: otp_dec_d could not read key on port %d\n", portno);
        exit(2);
      }
      
      // Check for bad characters 
      for (i = 0; i < encryptFileSize; i++) {
        if ((int) buffer[i] > 90 || ((int) buffer[i] < 65 && (int) buffer[i] != 32)) {
          printf("otp_dec_d error: ciphertext contains bad characters\n");
          exit(1);
        }
      }
      
      // validate key 
      for (i = 0; i < keyFileSize; i++){
        if ((int) keyBuffer[i] > 90 || ((int) keyBuffer[i] < 65 && (int) keyBuffer[i] != 32)){
          printf("otp_dec_d error: key contains bad characters\n");
          exit(1);
        }
      }
      
      // key must be at least as long as input file 
      if (keyFileSize < encryptFileSize){ 
        printf("otp_dec_d error: key is too short\n");
        exit(1);
      }
      
      // Reverse of the encryption process 
      for (i = 0; i < encryptFileSize; i++){
        // change spaces to asterisks
        if (buffer[i] == ' '){
          buffer[i] = '@';
        }
        if (keyBuffer[i] == ' '){
          keyBuffer[i] = '@';
        }
        
        // typecast to char for easier ASCII manipulation 
        int inputChar = (int) buffer[i];
        int keyChar = (int) keyBuffer[i];
        
        // Change ASCII range 
        inputChar = inputChar - 64;
        keyChar = keyChar - 64;
        
        // Update per program specifications for decryption 
        int cleanText = inputChar - keyChar;
        // Handle negative numbers 
        if (cleanText < 0){
          cleanText = cleanText + 27;
        }
        
        // range for capital letters 
        cleanText = cleanText + 64;

        // type conversion back to char
        buffer3[i] = (char) cleanText + 0;
        
        // after decryption, change asterisks to spaces
        if (buffer3[i] == '@'){
          buffer3[i] = ' ';
        }
      }
      
      // Write file 
      sent = write(newsockfd, buffer3, encryptFileSize);
      if (sent < encryptFileSize){
        printf("otp_dec_d error writing to socket\n");
        exit(2);
      }
      
      close(newsockfd);
      close(sockfd);
      
      exit(0);
    }
    
    else close(newsockfd);
  } 
  
  return 0;
}
