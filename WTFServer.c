#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>           /* read(), write(), close() */
#include <errno.h>
#include <sys/signal.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

int obtain_socket(int port);
void show_message(int sd);
void close_down(int sigtype);
void *connection_handler(void *socket_desc);
void createPro(int sd);
void currentver(int sd);
void checkout(int sd);
void destroy(int sd);
void update(int sd);
void upgrade(int sd);
void push(int sd);
void history(int sd);
void rollback(int sd);

#define SIZE 512

int ssockfd;     /* socket for PORT; global for close_down() */
pthread_mutex_t lock; //mutex for threads

int main (int argc, char** argv){
    
    if(argc != 2){
        printf("Incorrect arguments check and try again!\n");
        exit(1);
    }
    //check to see if projects directory exists and if not create it
    DIR *dir = opendir("./projects");
    //if dir is null projects directory doesnt exist so create it
    if(dir == NULL){
      mkdir("projects", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      printf("Projects directory created for server!\n");
    }
    closedir(dir);
    dir = opendir("./projects/rollbacks");
    if(dir == NULL){
      mkdir("./projects/rollbacks", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      printf("Rollbacks directory created for server!\n");
    }
    closedir(dir);
    int sd, client_len, PORT, *sock;
    struct sockaddr_in client;
    signal(SIGINT, close_down);
    PORT = atoi(argv[1]);
    if(PORT == 0){
        printf("Error reading PORT number, check argument and try again!\n");
        exit(2);
    }
    printf("Listening on port %d\n", PORT);
    //get socket
    ssockfd = obtain_socket(PORT);
    //create thread
    pthread_t thread;
    while(1){
        client_len = sizeof(client);
        if((sd = accept(ssockfd, (struct sockaddr *) &client, &client_len)) < 0){
            perror("Accept connection failure");
            exit(3);
        }
        sock = (int*)malloc(sizeof(int));
        *sock = sd;
        //create new thread and start connection handler and give it socket desc then wait for more connections
        pthread_create(&thread, NULL, connection_handler, (void*)sock);
       
    }
    pthread_exit(NULL);
    return 0;
}

/* Perform the first four steps of creating a server:
   create a socket, initialise the address data structure,
   bind the address to the socket, and wait for connections. 
*/
int obtain_socket(int port){
  int sockfd;
  struct sockaddr_in serv_addr;

  /* open a TCP socket */
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("could not create a socket");
    exit(1);
  }

  /* initialise socket address */
  memset((char *)&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(port);

  /* bind socket to address */
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("could not bind socket to address");
    exit(2);
  }

  /* set socket to listen for incoming connections */
  /* allow a queue of 5 */
  if (listen(sockfd, 5) == -1) {
    perror("listen error");
    exit(3);
  }
  return sockfd;
}
/* Close socket connection to PORT when ctrl-C is typed */
void close_down(int sigtype){
  close(ssockfd);
  printf("Listen terminated\n");
  exit(0);
}
//spawns new thread when a client connects and handles its request
void *connection_handler(void *socket_desc){
    //attemp to lock mutex
    pthread_mutex_lock(&lock);

    //buffer for recieving command from client
    char buff_comm[50] = {'\0'};
    
    int sd = *(int *)socket_desc;
    //get # bytes of command
    int size;
    read(sd, &size, sizeof(int));
    //read command into buff
    read(sd, buff_comm, size);
    //run function for requested command
    if(strcmp(buff_comm, "create")==0){
        createPro(sd);
    }
    else if(strcmp(buff_comm, "currentversion")==0){
        currentver(sd);
    }
    else if(strcmp(buff_comm, "checkout")==0){
        checkout(sd);
    }
    else if(strcmp(buff_comm, "destroy")==0){
        destroy(sd);
    }
    else if(strcmp(buff_comm, "update")==0){
        update(sd);
    }
    else if(strcmp(buff_comm, "upgrade")==0){
        upgrade(sd);
    }
    else if(strcmp(buff_comm, "commit")==0){
        //on commit we just need .Manfiest so send client manifest using update
        update(sd);
    }
    else if(strcmp(buff_comm, "push")==0){
        push(sd);
    }
    else if(strcmp(buff_comm, "history")==0){
        history(sd);
    }
    else if(strcmp(buff_comm, "rollback")==0){
        rollback(sd);
    }
    //close sd when done and unlock mutex
    close(sd);
    free(socket_desc);
    pthread_mutex_unlock(&lock);
    //exit thread
    pthread_exit(NULL);
}
//create project
void createPro(int sd){
    char buff_proName[1024] = {'\0'};
    int wrt, res = 1;
    //read project name into buffer
    read(sd, buff_proName, 1024);
    struct dirent *dp;
    DIR *dir = opendir("./projects");
    while((dp = readdir(dir)) != NULL){
       if(strcmp(dp->d_name, buff_proName) == 0){
          res = -1;
          send(sd, &res , sizeof(int), 0);
          return;
       }
    }
    closedir(dir);
    //project doesnt exist we can create it
    char buff[1024];
    char buff2[1024];
    strcpy(buff, "./projects/");
    strcat(buff, buff_proName);
    mkdir(buff, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    //create .Manifest file after making directory and send to client
    strcpy(buff2, buff);
    strcat(buff2, "/.Manifest");
    wrt = creat(buff2, S_IRWXU);
    write(wrt, "0\n", 2);
    close(wrt);
    send(sd, &res , sizeof(int), 0);
    send(sd, "0\n", 2, 0);
    return;
}
//get current version of project
void currentver(int sd){
  char buff_proName[1024] = {'\0'};
  char path[2048];
  int fp, res = 1, status= -1, curr, size;
  //read project into buffer
  read(sd, buff_proName, 1024);
  strcpy(path,"./projects/");
  strcat(path, buff_proName);
  strcat(path, "/.Manifest");
  //attempt to open manifest
  fp = open(path, O_RDONLY);
  //if cant open project doesnt exist
  if(fp < 0){
    res = -1;
    send(sd, &res , sizeof(int), 0);
    return;
  }
  //alert client server has project
  send(sd, &res , sizeof(int), 0);
  //get size of file
  curr = lseek(fp, 0, SEEK_CUR);
  size = lseek(fp, 0, SEEK_END);
  lseek(fp, curr, SEEK_SET);
  //send size of file to client
  send(sd, &size, sizeof(int), 0);
  //send manifest to client
  char buff[size+1];
  buff[size]='\0';
  read(fp, &buff, size);
  send(sd, &buff, size, 0);
  //after we send manifest to client we are finished
  close(fp);
  return;
}
//get project tar and send to client
void checkout(int sd){
  char buff_proName[1024] = {'\0'};
  char bf = '\0'; 
  char path[2048];
  char command[2048];
  int fp, res = 1, status= -1, curr, size;
  //read project into buffer
  read(sd, buff_proName, 1024);
  strcpy(path,"./projects/");
  strcat(path, buff_proName);
  //check if project exists
  DIR *dir = opendir(path);
  //project doesnt exist inform client and exit
  if(dir == NULL){
      res = -1;
      send(sd, &res , sizeof(int), 0);
      closedir(dir);
      return;
  }
  closedir(dir);
  //server does have project inform client
  send(sd, &res , sizeof(int), 0);
  //tar folder to send to client
  strcpy(command, "tar -C ./projects/ -czf ");
  strcat(command, buff_proName);
  strcat(command, ".tar.gz ");
  strcat(command, buff_proName);
  system(command);
  //send size of file to client
  fp = open(path, O_RDONLY);
  curr = lseek(fp, 0, SEEK_CUR);
  size = lseek(fp, 0, SEEK_END);
  lseek(fp, curr, SEEK_SET);
  send(sd, &size, sizeof(int), 0);
  //send project to client
  status = read(fp, &bf, 1);
  while(status > 0){
    send(sd, &bf, 1, 0);
    status = read(fp, &bf, 1);
  }
  //close fp and delete tar of project
  close(fp);
  strcat(path, ".tar.gz");
  strcpy(command, "rm -rf ");
  strcat(command, path);
  system(command);
  return;
}
//delete project given by client
void destroy(int sd){
  char buff_proName[1024] = {'\0'};
  char path[2048];
  char command[2048];
  int res = 1;
  //read project into buffer
  read(sd, buff_proName, 1024);
  strcpy(path,"./projects/");
  strcat(path, buff_proName);
  //attempt to open project directory
  DIR *dir = opendir(path);
  //server doesnt have project notify client
  if(dir == NULL){
    res = -1;
    send(sd, &res, sizeof(int), 0);
    closedir(dir);
    return;
  }
  strcpy(command, "rm -rf ");
  strcat(command, path);
  //call command to remove project
  system(command);
  //notfiy client project deleted
  send(sd, &res, sizeof(int), 0);
  return;
}
//on update send client project manifest
void update(int sd){
  char buff_proName[1024] = {'\0'};
  char path[2048];
  int fp, res = 1, status= -1, curr, size;
  //read project into buffer
  read(sd, buff_proName, 1024);
  strcpy(path,"./projects/");
  strcat(path, buff_proName);
  strcat(path, "/.Manifest");
  //attempt to open manifest
  fp = open(path, O_RDONLY);
  //if cant open project doesnt exist
  if(fp < 0){
    res = -1;
    send(sd, &res , sizeof(int), 0);
    return;
  }
  //alert client server has project
  send(sd, &res , sizeof(int), 0);
  //get size of file
  curr = lseek(fp, 0, SEEK_CUR);
  size = lseek(fp, 0, SEEK_END);
  lseek(fp, curr, SEEK_SET);
  //send size of file to client
  send(sd, &size, sizeof(int), 0);
  //send manifest to client
  char buff[size+1];
  buff[size]='\0';
  read(fp, &buff, size);
  send(sd, &buff, size, 0);
  //after we send manifest to client we are finished
  close(fp);
  return;
}
//send files to client that is updating
void upgrade(int sd){
  char buff_proName[1024] = {'\0'};
  char path[2048], file_path[1024], path2[2048], bf = '\0';
  int fp, man, res = 1, status= -1, curr, size, done = 1;
  //read project into buffer
  read(sd, buff_proName, 1024);
  strcpy(path,"./projects/");
  strcat(path, buff_proName);
  strcat(path, "/.Manifest");
  //attempt to open manifest
  man = open(path, O_RDONLY);
  //if cant open project doesnt exist
  if(fp < 0){
    res = -1;
    send(sd, &res , sizeof(int), 0);
    return;
  }
  //alert client server has project
  send(sd, &res , sizeof(int), 0);
  //recieve files to send
  while(done){
    //check if client is requesting file
    read(sd, &done, sizeof(int));
    //if client is not requesting file break
    if(done == 0){
      break;
    }
    //read file path name
    read(sd, file_path, 1023);
    strcpy(path2, "./projects");
    strcat(path2, file_path);
    //open file
    fp = open(path2, O_RDONLY);
    //get file size
    curr = lseek(fp, 0, SEEK_CUR);
    size = lseek(fp, 0, SEEK_END);
    lseek(fp, curr, SEEK_SET);
    //send size of file to client
    send(sd, &size, sizeof(int), 0);
    //send file contents
    status = read(fp, &bf, 1);
    while(status > 0){
      send(sd, &bf, 1, 0);
      status = read(fp, &bf, 1);
    }
    close(fp);
    //read to see if client is done requesting files
    read(sd, &done, sizeof(int));
  }
  //send client .Manifest
  //get manifest size
  curr = lseek(man, 0, SEEK_CUR);
  size = lseek(man, 0, SEEK_END);
  lseek(man, curr, SEEK_SET);
  //send size of manifest to client
  send(sd, &size, sizeof(int), 0);
  //send file contents
  status = read(man, &bf, 1);
  while(status > 0){
    send(sd, &bf, 1, 0);
    status = read(man, &bf, 1);
  }
  //done sending files to client return
  close(man);
  return;
}
//recieves files to add and delete to repository
void push(int sd){
  char buff_proName[1024] = {'\0'}, spath[1024] = {'\0'};
  char path[2048], u_line[1024], m_line[1024], fpath[1024], hpath[1024], path2[1024], buff = '\0', command[2048], ver[50];
  int res = 1, status= -1, curr, size, done = 1, nif = 1, i, wrt;
  FILE *fp, *commit, *tmp, *hist, *ovrwrt;
  char *token, *token2;
  //read project into buffer
  read(sd, buff_proName, 1024);
  strcpy(path,"./projects/");
  strcat(path, buff_proName);
  strcat(path, "/.Manifest");
  strcpy(path2, "./projects");
  strcpy(hpath, "./projects/");
  strcat(hpath, buff_proName);
  strcat(hpath, "/.History");
  //attempt to open manifest
  fp = fopen(path, "r");
  //if cant open project doesnt exist
  if(fp == NULL ){
    res = -1;
    send(sd, &res , sizeof(int), 0);
    return;
  }
  //back up project to rollback
  fgets(ver, 1024, fp);
  ver[strlen(ver) - 1] = '\0';
  strcpy(command, "tar -C ./projects/ -czf ");
  strcat(command, "./projects/rollbacks/");
  strcat(command, buff_proName);
  strcat(command, ver);
  strcat(command, ".tar.gz ");
  strcat(command, buff_proName);
  system(command);
  //alert client server has project
  send(sd, &res , sizeof(int), 0);
  //recieve size of .Commit
  read(sd, &size, sizeof(int));
  commit = tmpfile();
  //read commit
  for(i=0; i<size; i++){
      read(sd, &buff, 1);
      fputc(buff, commit);
  }
  rewind(commit);
  //go through .Commit and perform MAD
  while(1){
    if(fgets(u_line, 1024, commit) == NULL){
      //printf("--END OF CLIENT MANIFEST--\n");
      break;
    }
    //get where to M,A,D
    token = strtok(u_line, "\t\n");
    //file is being added
    if(strcmp(token, "A") == 0){
      //get file path
      token = strtok(NULL, "\t\n");
      strcpy(fpath, token);
      //get path without file name at end
      for(i = strlen(fpath) - 1; i >= 0; i--){
        if(fpath[i] == '/'){
          break;
        }
      }
      strncpy(spath, fpath, i);
      strcpy(path2, "./projects");
      strcat(path2, &spath[1]);
      //creates all directories if they dont exist
      strcpy(command, "mkdir -p ");
      strcat(command, path2);
      system(command);
      //tell server we are requesting file
      send(sd, &done, sizeof(int), 0);
      //ask client for file
      send(sd, fpath, 1023, 0);
      // get file size and read file from server
      read(sd, &size, sizeof(int));
      //create file
      strcpy(path2, "./projects");
      strcat(path2, &fpath[1]);
      wrt = creat(path2, S_IRWXU);
      for(i=0; i<size; i++){
        read(sd, &buff, 1);
        write(wrt, &buff, 1);
      }
      //close file
      close(wrt);
      //inform server we are still requesting files
      send(sd, &done, sizeof(int), 0);
    }
    //file is being override with one from server
    else if(strcmp(token, "M") == 0){
      //get file path
      token = strtok(NULL, "\t\n");
      strcpy(fpath, token);
      strcpy(path2, "./projects");
      strcat(path2, &fpath[1]);
      //open file to be overwritten
      ovrwrt = fopen(path2, "w");
      //tell client we are requesting file
      send(sd, &done, sizeof(int), 0);
      //ask client for file
      send(sd, fpath, 1023, 0);
      // get file size and read file from server
      read(sd, &size, sizeof(int));
      for(i=0; i<size; i++){
        read(sd, &buff, 1);
        fputc(buff, ovrwrt);
      }
      //close file
      fclose(ovrwrt);
      //inform server we are still requesting files
      send(sd, &done, sizeof(int), 0);
    }
    //file is being deleted
    else if(strcmp(token, "D") == 0){
      //get file path
      token = strtok(NULL, "\t\n");
      strcpy(fpath, token);
      strcpy(path2, "./projects");
      strcat(path2, &fpath[1]);
      //delete file
      remove(path2);
    }
  }
  //inform client we are done requesting files
  done = 0;
  send(sd, &done, sizeof(int), 0);
  rewind(commit);
  //update .Manifest
  tmp = tmpfile();
  //go thru .Commit and add anything with an A or M
  int v = atoi(ver);
  v++;
  sprintf(ver, "%d", v);
  //print new version number
  fprintf(tmp, "%s\n", ver);
  while(1){
    if(fgets(u_line, 1024, commit) == NULL){
      //printf("--END OF CLIENT MANIFEST--\n");
      break;
    }
    //get where to M,A,D
    strcpy(spath, u_line);
    token = strtok(u_line, "\t\n");
    //add to tmp
    if(strcmp(token, "A") == 0 || strcmp(token, "M") == 0){
      fprintf(tmp, "%s", &spath[2]);
    }
  }
  rewind(commit);
  //now add all file paths that are in .Manifest but not .Commit
  while(1){
    if(fgets(m_line, 1024, fp) == NULL){
      break;
    }
    strcpy(spath, m_line);
    token = strtok(m_line, "\t\n");
    while(1){
      if(fgets(u_line, 1024, commit) == NULL){
      //printf("--END OF CLIENT MANIFEST--\n");
      break;
      }
      //get path
      token2 = strtok(u_line, "\t\n");
      token2 = strtok(NULL, "\t\n");
      if(strcmp(token, token2) == 0){
          nif = 0;
          break;
      }
    }
    if(nif){
      fprintf(tmp, "%s", spath);
    }
    else{
      nif = 1;
    }
  }
  rewind(commit);
  //add .Commit to .History
  time_t t;
  struct tm *tm;
  hist = fopen(hpath, "a");
  //print time stamp
  t = time(NULL);
  tm = localtime(&t);
  fprintf(hist, "%s", asctime(tm));
  while(1){
    if(fgets(u_line, 1024, commit) == NULL){
      break;
    }
    fprintf(hist, "%s", u_line);
  }
  //close commit and man
  fclose(hist);
  fclose(commit);
  fclose(fp);
  //rewind tmp and overwrite man
  rewind(tmp);
  fp = fopen(path, "w");
  while(1){
    if(fgets(m_line, 1024, tmp) == NULL){
      break;
    }
    fprintf(fp, "%s", m_line);
  }
  fclose(fp);
  fp = fopen(path, "r");
  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  send(sd, &size, sizeof(int), 0);
  rewind(fp);
  //send manifest to client
  for(i = 0; i <size; i++){
      buff = fgetc(fp);
      send(sd, &buff, sizeof(char), 0);
  }
  fclose(fp);
  return;
}
//give client project history
void history(int sd){
  char buff_proName[1024] = {'\0'};
  char path[2048];
  int fp, res = 1, status= -1, curr, size;
  //read project into buffer
  read(sd, buff_proName, 1024);
  strcpy(path,"./projects/");
  strcat(path, buff_proName);
  strcat(path, "/.History");
  //attempt to open manifest
  fp = open(path, O_RDONLY);
  //if cant open project doesnt exist
  if(fp < 0){
    res = -1;
    send(sd, &res , sizeof(int), 0);
    return;
  }
  //alert client server has project
  send(sd, &res , sizeof(int), 0);
  //get size of file
  curr = lseek(fp, 0, SEEK_CUR);
  size = lseek(fp, 0, SEEK_END);
  lseek(fp, curr, SEEK_SET);
  //send size of file to client
  send(sd, &size, sizeof(int), 0);
  //send manifest to client
  char buff[size+1];
  buff[size]='\0';
  read(fp, &buff, size);
  send(sd, &buff, size, 0);
  //after we send manifest to client we are finished
  close(fp);
  return;
}
//rollback project to previous version
void rollback(int sd){
  char buff_proName[1024] = {'\0'};
  char pro_ver[1024] = {'\0'};
  char bf = '\0'; 
  char path[2048];
  char command[2048];
  int fp, res = 1, status= -1, curr, size, nsize;
  //read project into buffer
  read(sd, &nsize, sizeof(int));
  read(sd, buff_proName, nsize);
  read(sd, &nsize, sizeof(int));
  read(sd, pro_ver, nsize);
  strcpy(path,"./projects/rollbacks/");
  strcat(path, buff_proName);
  strcat(path, pro_ver);
  strcat(path, ".tar.gz");
  //check if project exists
  fp = open(path, O_RDONLY);
  //project doesnt exist inform client and exit
  if(fp < 0){
      res = -1;
      send(sd, &res , sizeof(int), 0);
      return;
  }
  //server does have project inform client
  send(sd, &res , sizeof(int), 0);
  //remove current project version
  strcpy(command, "rm -rf ./projects/");
  strcat(command, buff_proName);
  system(command);
  //send size of file to client
  curr = lseek(fp, 0, SEEK_CUR);
  size = lseek(fp, 0, SEEK_END);
  lseek(fp, curr, SEEK_SET);
  send(sd, &size, sizeof(int), 0);
  //send project to client
  status = read(fp, &bf, 1);
  while(status > 0){
    send(sd, &bf, 1, 0);
    status = read(fp, &bf, 1);
  }
  //close fp and unzip tar of project
  close(fp);
  strcpy(command, "tar -xvzf ");
  strcat(command, path);
  strcat(command, " -C ");
  strcat(command, "./projects/");
  system(command);
  return;
}