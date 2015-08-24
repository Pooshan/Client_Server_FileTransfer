/*FTP Client*/
 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include<arpa/inet.h>   
#include<sys/ioctl.h> 
#include<errno.h>
 
/*for getting file size using stat()*/
#include<sys/stat.h>
 
/*for sendfile()*/
#include<sys/sendfile.h>
 
/*for O_RDONLY*/
#include<fcntl.h>
#include<termios.h>
//sending file
int send_image(int socket, char filename[])
{

   FILE *picture;
   int size, read_size, stat, packet_index;
   char send_buffer[10240], read_buffer[1024];
   packet_index = 1;

   picture = fopen(filename, "r");
   printf("Getting Picture Size\n");   

   if(picture == NULL) 
	{
        printf("Error Opening Image File"); 
	} 

   fseek(picture, 0, SEEK_END);
   size = ftell(picture);
   fseek(picture, 0, SEEK_SET);
   printf("Total Picture size: %i\n",size);

   //Send Picture Size
   printf("Sending Picture Size\n");
   write(socket, (void *)&size, sizeof(int));

   //Send Picture as Byte Array
   printf("Sending Picture as Byte Array\n");

   do 
     { //Read while we get errors that are due to signals.
      stat=read(socket, &read_buffer , 1024);
      printf("Bytes read: %i\n",stat);
     } 
   while (stat < 0);

   printf("Received data in socket\n");
   //printf("Socket data: %s\n", read_buffer);

   while(!feof(picture)) {
   
      //Read from the file into our send buffer
      read_size = fread(send_buffer, 1, sizeof(send_buffer)-1, picture);

      //Send data through our socket 
      do
	{
        stat = write(socket, send_buffer, read_size);  
        }
      while (stat < 0);
struct timeval timeout = {1,0};

      printf("Packet Number: %i\n",packet_index);
      printf("Packet Size Sent: %i\n",read_size);     
      printf(" \n");
      printf(" \n");
      packet_index++;  

      //Zero out our send buffer
      bzero(send_buffer, sizeof(send_buffer));
     //return 0;
     break;

     }
return 0;
    }
//receive file
int receive_file(int socket, char filename[])
{ // Start function 

int recv_size = 0,size = 0, read_size, write_size, packet_index =1,stat;

char filearray[10241];
FILE *fp;

//Find the size of the file
do{
stat = read(socket, &size, sizeof(int));
}while(stat<0);


char buffer[] = "Got it";

//Send our verification signal
do{
stat = write(socket, &buffer, sizeof(int));
}while(stat<0);

printf("Reply sent\n");
printf(" \n");

fp = fopen(filename, "w");

if( fp == NULL) {
printf("Error has occurred. file could not be opened\n");
return -1; }

//Loop while we have not received the entire file yet


//int need_exit = 0;
struct timeval timeout = {1,0};

fd_set fds;
int buffer_fd, buffer_out;

while(recv_size < size) {
//while(packet_index < 2){

    FD_ZERO(&fds);
    FD_SET(socket,&fds);

    buffer_fd = select(FD_SETSIZE,&fds,NULL,NULL,&timeout);

    if (buffer_fd < 0)
      { //printf("error: bad file descriptor set.\n");
         return 0;}

    if (buffer_fd == 0)
       {//printf("error: buffer read timeout expired.\n");
         fclose(fp);
         printf("File successfully Received!\n");
         return 0;}

    if (buffer_fd > 0)
    {

       do{
               read_size = read(socket,filearray, 10241);
            }while(read_size <0);

            printf("Packet number received: %i\n",packet_index);
        printf("Packet size: %i\n",read_size);


        //Write the currently read data into our file
         write_size = fwrite(filearray,1,read_size, fp);
         printf("Written file size: %i\n",write_size); 

             if(read_size !=write_size) {
                 printf("error in read write\n");    }


             //Increment the total number of bytes read
             recv_size += read_size;
             packet_index++;
             printf("Total received file size: %i\n",recv_size);
             printf(" \n");
    }

}

  fclose(fp);
  printf("File successfully Received!\n");
  return 1;
  }
 
int main(int argc,char *argv[])
{
  struct sockaddr_in server;
  struct stat obj;
  int sock;
  int choice;
  char buf[100], command[5], filename[20], *f;
  int k, size, status;
  int filehandle;
int i;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock == -1)
    {
      printf("socket creation failed");
      exit(1);
    }
  server.sin_family = AF_INET;
  server.sin_port = atoi(argv[1]);
  server.sin_addr.s_addr = 0;

  k = connect(sock,(struct sockaddr*)&server, sizeof(server));
  if(k == -1)
    {
      printf("Connect Error");
      exit(1);
    }
   size = 0;
  char  username[10],ch,ci;
  char password[10];
  char str1[7],str2[5];
  strcpy(str1, "client1");
   strcpy(str2, "pass");
  i=0;
  printf("strlen username %d, pass len = %d\n", (int)strlen(str1), (int)strlen(str2));

  printf("Enter user name: \n");
     while(ch!='\n')    // terminates if user hit enter
    {
        ch=getchar();
        username[i]=ch;
        i++;
    }
     username[i-1]='\0';
     printf("%s : len = %d\n", username, (int)strlen(username));

  send(sock, username, 100, 0);
  size = 0;
  recv(sock, buf, 100, 0);
  printf("reply from server = %s\n", buf);
  if (strcmp(buf, "success")) {
  //if (!size) {
    printf("wrong username %s\n", username);
    exit(1);
  }
 printf("Enter password: \n");
   //  ch='\0';
  //   i = 0;
   /*  while(ch != '\n')
     {
       
      ch=getchar();
      
      
      //if(ch=='\n')
      //{
        //break;
      //}
      //#endif
      //printf("****");
      password[i++]=ch;
     }
     password[i-1]='\0';
*/
  strcpy(password,getpass(""));
  size = 0;
  send(sock, password, 100, 0);
  recv(sock, buf, sizeof(int), 0);
  printf("reply from server = %s\n", buf);
  if (strcmp(buf, "success")) {
    printf("wrong password %s\n", password);
    exit (1);
  }  
 
   i = 1;
  while(1)
    {
      printf("Enter a choice:\n1- get\n2- put\n3- pwd\n4- ls\n5- cd\n6- quit\n");
      scanf("%d", &choice);
      switch(choice)
	{
	case 1:
	  printf("Enter filename to get: ");
	  scanf("%s", filename);
	  strcpy(buf, "get ");
	  strcat(buf, filename);
	  send(sock, buf, 100, 0);
	  recv(sock, &size, sizeof(int), 0);
	  if(!size)
	    {
	      printf("No such file on the remote directory\n\n");
	    break;
	    }
	  f = malloc(size);
	  recv(sock, f, size, 0);
	 while(size<=10241)
	    {
	      filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666);
	     if(filehandle == -1)
		{
		  sprintf(filename + strlen(filename), "%d", i);//needed only if same directory is used for both server and client
		}
	      else break;
	    }
	  write(filehandle, f, size, 0);
          receive_file(sock, filename);
	  close(filehandle);
	strcpy(buf, "cat ");
	  //strcat(buf, filename);
	 system(buf);
	  
	  break;
	case 2:
	  printf("Enter filename to put to server: ");
          scanf("%s", filename);
	  filehandle = open(filename, O_RDONLY);
          if(filehandle == -1)
            {
              printf("No such file on the local directory\n\n");
              break;
            }
          strcpy(buf, "put ");
	  strcat(buf, filename);
	  send(sock, buf, 100, 0);
	  stat(filename, &obj);
	  size = obj.st_size;

	  send(sock, &size, sizeof(int), 0);

	  sendfile(sock, filehandle, NULL, size);
	send_image(sock, filename);
	  recv(sock, &status, sizeof(int), 0);

	  if(status)
	    printf("File stored successfully\n");
	  else
	    printf("File failed to be stored to remote machine\n");
	  break;
	case 3:
	  strcpy(buf, "pwd");
	  send(sock, buf, 100, 0);
	  recv(sock, buf, 100, 0);
	  printf("The path of the remote directory is: %s\n", buf);
	  break;
	case 4:
	  strcpy(buf, "ls");
          send(sock, buf, 100, 0);
	  recv(sock, &size, sizeof(int), 0);
          f = malloc(size);
          recv(sock, f, size, 0);
	  filehandle = creat("temp.txt", O_WRONLY);
	  write(filehandle, f, size, 0);
	  close(filehandle);
          printf("The remote directory listing is as follows:\n");
          system("chmod 777 temp.txt");
	  system("cat temp.txt");
	  break;
	case 5:
	  strcpy(buf, "cd ");
	  printf("Enter the path to change the remote directory: ");
	  scanf("%s", buf + 3);
          send(sock, buf, 100, 0);
	  recv(sock, &status, sizeof(int), 0);
          if(status)
            printf("Remote directory successfully changed\n");
          else
            printf("Remote directory failed to change\n");
          break;
	case 6:
	  strcpy(buf, "quit");
          send(sock, buf, 100, 0);
          recv(sock, &status, 100, 0);
	  if(status)
	    {
	      printf("Server closed\nQuitting..\n");
	      exit(0);
	    }
	    printf("Server failed to close connection\n");
	}
    }
}
