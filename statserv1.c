#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*for getting file size using stat()*/
#include<sys/stat.h>
 
/*for sendfile()*/
#include<sys/sendfile.h>
 
/*for O_RDONLY*/
#include<fcntl.h>
#include<pthread.h>
   #include<arpa/inet.h>   
   #include<errno.h>


void *processRequest(void *sdPtr);
 
int main(int argc,char *argv[])
{
  struct sockaddr_in server, client;
  struct stat obj;
  int sock1, sock2;
  char buf[100], command[5], filename[20];
  int k, i, size, len, c;
  int filehandle;
  pthread_t thread_id;

  sock1 = socket(AF_INET, SOCK_STREAM, 0); //socket creation
  if(sock1 == -1)
    {
      printf("Socket creation failed");
      exit(1);
    }
  server.sin_family = AF_INET;
  server.sin_port = atoi(argv[1]);
  server.sin_addr.s_addr = 0;
  k = bind(sock1,(struct sockaddr*)&server,sizeof(server)); //binding socket to local port
  if(k == -1)
    {
      printf("Binding error");
      exit(1);
    }
  k = listen(sock1,1); //listening
  if(k == -1)
    {
      printf("Listen failed");
      exit(1);
    }
  len = sizeof(client);
 int cnt=0;
    while(1)
{ 
  if(cnt<=10)
  {
    if((sock2 = accept(sock1,(struct sockaddr*)&client, &len)) != 0)
    {

printf("no. of active connections: %d\n",cnt+1);
//printf("\naccepting request from client\n");

int *new_sock;
 pthread_t sniffer_thread;
new_sock = malloc(1);
        *new_sock = sock2;
       if( pthread_create( &sniffer_thread , NULL , processRequest, (void*) new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }

        puts("Handler assigned");
    }
  cnt++;
  }
else
{

close(&sock2);
}

}


}
void *processRequest(void *sdPtr) {
FILE *picture;
int sock2 = *(int *)sdPtr;
struct stat obj;
char buf[100],command[5], filename[20];
int k, i, size, len, c, ret;
  int filehandle;
  i = 1;

//authentication

  //read username
  ret = 0;
  recv(sock2, buf, 100, 0);
  
  if (strcmp(buf, "client1"))
 {
   if (strcmp(buf, "client2")) 
{   
    printf("wrong username %s\n", buf);
    ret = 1; //indicates error
    send(sock2, "failure", 7, 0);
    close(sock2);
    return;
  }
}
/*else 
  }*/
 

send(sock2, "success", 7, 0);
 bzero(buf, sizeof(buf));
  //read pass
  recv(sock2, buf, 100, 0);
  if (strcmp(buf, "pass")) {
    printf("wrong password\n");
    ret = 1; //indicates error
    send(sock2, "failure", 7, 0);
    close(sock2);
    return;
  }
  send(sock2, "success", 7, 0);

  while(1)
    {
//printf("\nreceiving a file from the server %s \n",buf);
      recv(sock2, buf, 100, 0);

      sscanf(buf, "%s", command);
//printf("\naction is %s\n",command);
      if(!strcmp(command, "ls"))
	{
	  system("ls >temps.txt");
	  i = 0;
	  stat("temps.txt",&obj);
	  size = obj.st_size;

	  send(sock2, &size, sizeof(int),0);

	  filehandle = open("temps.txt", O_RDONLY);
printf("\nthe list is logged\n" );
	  sendfile(sock2,filehandle,NULL,size);
	}
      else if(!strcmp(command,"get"))
	{
	  sscanf(buf, "%s%s", filename, filename);
	  stat(filename, &obj);
	  filehandle = open(filename, O_RDONLY);

	  size = obj.st_size;
	  if(filehandle == -1)
	      size = 0;
printf("\nfile dowmloaded from the server %s\n",filename);
	 // send(sock2, &size, sizeof(int), 0);
         picture = fopen(filename, "r");
          fseek(picture, 0, SEEK_END);
   size = ftell(picture);
   fseek(picture, 0, SEEK_SET);
   	printf("Total file size: %i\n",size);
	  if(size)
	  sendfile(sock2, filehandle, NULL, size);

 
	}
      else if(!strcmp(command, "put"))
        {
	  int c = 0, len;
	  char *f;
	  sscanf(buf+strlen(command), "%s", filename);
printf("\nfile recieved on the server is %s\n",filename);
printf("\nthe content of file is %s\n",buf);
picture = fopen(filename, "r");
          fseek(picture, 0, SEEK_END);
   size = ftell(picture);
   fseek(picture, 0, SEEK_SET);
   	printf("Total file size: %i\n",size);
	  recv(sock2, &size, sizeof(int), 0);

	  i = 1;
	  while(1)
	    {
	      filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666);
	      if(filehandle == -1)
		{
		  sprintf(filename + strlen(filename), "%d", i);
		}
	      else

		break;
	    }
	  f = malloc(size);
	  recv(sock2, f, size, 0);
	  c = write(filehandle, f, size);
	//send_image(sock2, filename);
	  close(filehandle);
	  send(sock2, &c, sizeof(int), 0);
        }
      else if(!strcmp(command, "pwd"))
	{
	  system("pwd>temp.txt");
	  i = 0;
          FILE*f = fopen("temp.txt","r");
          while(!feof(f))
            buf[i++] = fgetc(f);
          buf[i-1] = '\0';
	  fclose(f);
printf("the path is stored in temp.txt");
          send(sock2, buf, 100, 0);
	}
      else if(!strcmp(command, "cd"))
        {
          if(chdir(buf+3) == 0)
	    c = 1;
	  else
	    c = 0;

          send(sock2, &c, sizeof(int), 0);
        }
 
 
      else if(!strcmp(command, "quit"))
	{
	  printf("FTP server quitting..\n");
	  i = 1;
	  send(sock2, &i, sizeof(int), 0);
	  exit(0);
	}
    }
  return 0;
}
