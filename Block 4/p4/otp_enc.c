/******************************************************
 *                  otp_enc.c
 * Name: Brett Stephenson
 * Class: CS344
 * Desc: This is the daemon that will encrypt the file 
 *       when connected to
 *******************************************************/
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
#define SIZE    128000
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
  int retrieved;
  int sent;
  int inputFileSize;
  
  // Verify Number of Arguments
  if (argc < 4){
    printf("Usage: otp_enc plaintext key port\n");
    exit(1);
  }
  
  portno = atoi(argv[3]);
  // Read Only
  fp = open(argv[1], O_RDONLY);
  
  if (fp < 0){
    printf("Error: cannot open plaintext file %s\n", argv[1]);
    exit(1);
  }
  
  // Get contents and length
  inputFileSize = read(fp, buffer, SIZE);
  
  // Check for bad characters
  for (i = 0; i < inputFileSize - 1; i++){
    if ((int) buffer[i] > 90 || ((int) buffer[i] < 65 && (int) buffer[i] != 32)){
      printf("otp_enc error: plaintext contains bad characters\n");
      exit(1);
    }
  }
  
  close(fp);
  
  // Repeat process for the key file
  fp = open(argv[2], O_RDONLY);
  
  if (fp < 0){
    printf("Error: cannot open key file %s\n", argv[2]);
    exit(1);
  }
  
  // Get contents and Length of file
  keyFileSize = read(fp, keyBuffer, SIZE);
  
  // Validation
  for (i = 0; i < keyFileSize - 1; i++){
    if ((int) keyBuffer[i] > 90 || ((int) keyBuffer[i] < 65 && (int) keyBuffer[i] != 32)){
      printf("otp_enc error: key contains bad characters\n");
      exit(1);
    }
  }
  
  close(fp);
  
  // Key file must be at least as long as the input file
  if (keyFileSize < inputFileSize){
    printf("Error: key '%s' is too short\n", argv[2]);
  }
  
  // Start Socket Portion:
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0){
    printf("Error: could not contact otp_enc_d on port %d\n", portno);
    exit(2);
  }
  
  memset(&serv_addr, '\0', sizeof(serv_addr));
  
  server = gethostbyname("localhost");
  if (server == NULL){
    printf("Error: could not connect to otp_enc_d\n");
    exit(2);
  }    
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);         
  serv_addr.sin_port = htons(portno);
  
  if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
    printf("Error: could not connect to otp_enc_d on port %d\n", portno);
    exit(2);
  }
  
  // write the input file
  sent = write(sockfd, buffer, inputFileSize - 1);
  if (sent < inputFileSize - 1){
    printf("Error: could not send plaintext to otp_enc_d on port %d\n", portno);
    exit(2);
  }
  
  memset(buffer3, 0, 1);
  
  // get acknowledgement from server
  retrieved = read(sockfd, buffer3, 1);
  if (retrieved < 0){
    printf("Error receiving acknowledgement from otp_enc_d\n");
    exit(2);
  }
  
  // write the key
  sent = write(sockfd, keyBuffer, keyFileSize - 1);
  if (sent < keyFileSize - 1){
    printf("Error: could not send key to otp_enc_d on port %d\n", portno);
    exit(2);
  }
  
  // clear buffer
  memset(buffer, 0, SIZE);
  
  do{
    // text should come back encrypted; read it
    retrieved = read(sockfd, buffer, inputFileSize - 1);
  }
  while (retrieved > 0);
  
  if (retrieved < 0){
    printf("Error receiving ciphertext from otp_enc_d\n");
    exit(2);
  }
  
  // Print to console
  for (i = 0; i < inputFileSize - 1; i++){
    printf("%c", buffer[i]);
  }
  printf("\n");
  
  // close socket
  close(sockfd);
  
  return 0;
}
