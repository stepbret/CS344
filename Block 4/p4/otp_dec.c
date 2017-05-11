/***********************************************
 *                  otp_dec.c 
 *  Name: Brett Stephenson
 *  Class: CS344
 *  desc: This will send the file and the key
 *        to get encrypted
 ************************************************/
#include <arpa/inet.h>
#include <fcntl.h>     
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>     
#include <stdlib.h>   
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <unistd.h>
#define SIZE    16000
int main(int argc, char *argv[]){
  
  int sockfd, portno, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char buffer[SIZE];
  char keyBuffer[SIZE];
  char buffer3[1];
  int fp;
  int i;
  int keyFileSize;
  int encryptFileSize;
  int retrieved;
  int sent;
  
  // Check number of arguments 
  if (argc < 4){
    printf("Usage: otp_dec ciphertext key port\n");
    exit(1);
  }
  
  portno = atoi(argv[3]);
  // This should be the encrypted file
  fp = open(argv[1], O_RDONLY);
  
  if (fp < 0){
    printf("Error: cannot open ciphertext file %s\n", argv[1]);
    exit(1);
  }
  
  // Get contents and length of file 
  encryptFileSize = read(fp, buffer, SIZE);
  
  // Check for bad characters 
  for (i = 0; i < encryptFileSize - 1; i++){
    if ((int) buffer[i] > 90 || ((int) buffer[i] < 65 && (int) buffer[i] != 32)){
      printf("otp_dec error: ciphertext contains bad characters\n");
      exit(1);
    }
  }
  
  close(fp);
  
  // open key file
  fp = open(argv[2], O_RDONLY);
  
  if (fp < 0){
    printf("Error: cannot open key file %s\n", argv[2]);
    exit(1);
  }
  
  // Get contents and length of key file 
  keyFileSize = read(fp, keyBuffer, SIZE);
  
  // validation 
  for (i = 0; i < keyFileSize - 1; i++){
    if ((int) keyBuffer[i] > 90 || ((int) keyBuffer[i] < 65 && (int) keyBuffer[i] != 32)){
      printf("otp_dec error: key contains bad characters\n");
      exit(1);
    }
  }
  
  close(fp);
  
  // key file must be at least as long as input file 
  if (keyFileSize < encryptFileSize){
    printf("Error: key '%s' is too short\n", argv[2]);
  }
  
  // Start socket 
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0){
    printf("Error: could not contact otp_dec_d on port %d\n", portno);
    exit(2);
  }
  
  memset(&serv_addr, '\0', sizeof(serv_addr));
  server = gethostbyname("localhost");
  if (server == NULL){
    printf("Error: could not connect to otp_dec_d\n");
    exit(2);
  }    
  serv_addr.sin_family = AF_INET;
  
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);         
  serv_addr.sin_port = htons(portno);
 
  
  if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
    printf("Error: could not connect to otp_dec_d on port %d\n", portno);
    exit(2);
  }
  
  // Write file for decryption 
  sent = write(sockfd, buffer, encryptFileSize - 1);
  if (sent < encryptFileSize - 1){
    printf("Error: could not send ciphertext to otp_dec_d on port %d\n", portno);
    exit(2);
  }
  
  memset(buffer3, 0, 1);
    
  // get acknowledgement from server
  retrieved = read(sockfd, buffer3, 1);
  if (retrieved < 0){
    printf("Error receiving acknowledgement from otp_dec_d\n");
    exit(2);
  }
  
  // Write key for decryption 
  sent = write(sockfd, keyBuffer, keyFileSize - 1);
  if (sent < keyFileSize - 1){
    printf("Error: could not send key to otp_dec_d on port %d\n", portno);
    exit(2);
  }
  
  // clear buffer 
  memset(buffer, 0, SIZE);
  
  do{
    // Read contents 
    retrieved = read(sockfd, buffer, encryptFileSize - 1);
  }
  while (retrieved > 0);
  
  if (retrieved < 0){
    printf("Error receiving ciphertext from otp_dec_d\n");
    exit(2);
  }
  
  // Print to console 
  for (i = 0; i < encryptFileSize - 1; i++){
    printf("%c", buffer[i]);
  }
  printf("\n");
  
  close(sockfd);
  
  return 0;
}
