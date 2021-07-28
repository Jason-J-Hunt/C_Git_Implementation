#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> 
#include <sys/socket.h>  
#include <netinet/in.h> 
#include <string.h> 
#include <fcntl.h>
#include <openssl/sha.h>

int create_socket();
void configure(char *ip, char *port);
void createPro(char *project_name);
void add(char *project, char *filename);
void wtf_remove(char *project, char *filename);
void checkout(char *project_name);
void currentver(char *project_name);
void destroy(char *project_name);
void update(char *project_name);
void upgrade(char *project_name);
void commit(char *project_name);
void push(char *project_name);
void history(char *project_name);
void rollback(char *project_name, char *version);

int main (int argc, char** argv){
    if(argc > 4 || argc <3){
        printf("Check arguemnts and try again!\n");
        exit(1);
    }
    
    if(strcmp(argv[1], "configure") == 0){
        if(argc != 4){
            printf("Provide ip and port number!\n");
            exit(1);
        }
        configure(argv[2], argv[3]);
    }
    else if(strcmp(argv[1], "checkout") == 0){
        if(argc != 3){
            printf("Check arguments and try again!\n");
            exit(1);
        }
        checkout(argv[2]);
    }
    else if(strcmp(argv[1], "update") == 0){
        if(argc != 3){
            printf("Check arguments and try again!\n");
            exit(1);
        }
        update(argv[2]);
    }
    else if(strcmp(argv[1], "upgrade") == 0){
        if(argc != 3){
            printf("Check arguments and try again!\n");
            exit(1);
        }
        upgrade(argv[2]);
    }
    else if(strcmp(argv[1], "commit") == 0){
        if(argc != 3){
            printf("Check arguments and try again!\n");
            exit(1);
        }
        commit(argv[2]);
    }
    else if(strcmp(argv[1], "push") == 0){
        if(argc != 3){
            printf("Check arguments and try again!\n");
            exit(1);
        }
        push(argv[2]);
    }
    else if(strcmp(argv[1], "create") == 0){
        if(argc != 3){
            printf("Check arguments and try again!\n");
            exit(1);
        }
        createPro(argv[2]);
    }
    else if(strcmp(argv[1], "destroy") == 0){
        if(argc != 3){
            printf("Check arguments and try again!\n");
            exit(1);
        }
        destroy(argv[2]);
    }
    else if(strcmp(argv[1], "add") == 0){
        if(argc != 4){
            printf("Check arguments and try again!\n");
            exit(1);
        }
        add(argv[2], argv[3]);
    }
    else if(strcmp(argv[1], "remove") == 0){
        if(argc != 4){
            printf("Check arguments and try again!\n");
            exit(1);
        }
        wtf_remove(argv[2], argv[3]);
    }
    else if(strcmp(argv[1], "currentversion") == 0){
        if(argc != 3){
            printf("Check arguments and try again!\n");
            exit(1);
        }
        currentver(argv[2]);
    }
    else if(strcmp(argv[1], "history") == 0){
        if(argc != 3){
            printf("Check arguments and try again!\n");
            exit(1);
        }
        history(argv[2]);
    }
    else if(strcmp(argv[1], "rollback") == 0){
        if(argc != 4){
            printf("Check arguments and try again!\n");
            exit(1);
        }
        rollback(argv[2], argv[3]);
    }
    else{
        printf("Not a valid comment check arguments and try again!\n");
        //exit(1);
    }

    return 0;
}
//configure ip and port for server
void configure(char *ip, char *port){
    //open config file and clear it
    int fp = open("./.configure", O_RDWR | O_TRUNC);
    if(fp < 0){
        printf("configure file not found creating it and adding entries\n");
        fp = creat("./.configure", S_IRWXU);
    }
    //write ip addy to file
    write(fp, ip, strlen(ip));
    //write a tab
    write(fp, "\t", 1);
    //write port
    write(fp, port, strlen(port));
    //close fp
    close(fp);

    return;
}
//create a socket that connects to server and return
int create_socket(){
    struct sockaddr_in address;
    int sock, PORT;
    struct sockaddr_in serv_addr;
    char *ip, *sport, *cfile, *token;
 
    //get ip and port from config file
    int fp = open("./.configure", O_RDONLY);

    if(fp < 0){
        printf("unable to fetch config file!\n Please configure ip and port!\n");
        return -1;
    }

    int curr = lseek(fp, 0, SEEK_CUR);
    int fsize = lseek(fp, 0, SEEK_END);
    lseek(fp, curr, SEEK_SET);
    cfile = (char*)malloc(sizeof(char)*fsize +1);
    int status = read(fp, cfile, fsize);

    if(status < 0){
        printf("Error reading config file!\n Please configure ip port!\n");
        return -1;
    }
    //first token is ip
    token = strtok(cfile, "\t");
    ip = (char*)malloc(sizeof(char)*strlen(token)+1);
    strcpy(ip, token);
    token = strtok(NULL, "\t");
    sport = (char*)malloc(sizeof(char)*strlen(token)+1);
    strcpy(sport,token);
    PORT = atoi(sport);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    }
    //zero out mem   
    memset(&serv_addr, '0', sizeof(serv_addr)); 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 

    //return socket
    return sock;
}
//create a project
void createPro(char *project_name){
    char buff[256];
    char buff2[4];
    int sock = create_socket();
    int res, no, wrt;
    if(sock == -1){
        printf("failed to connect to server!\n");
    }
    //send length of command
    int n = strlen("create");
    send(sock, &n, sizeof(int), 0);
    //send command create to server
    send(sock , "create" , strlen("create") , 0 );
    //send server name of project
    send(sock, project_name, strlen(project_name), 0);
    //recieve reponse on if the project was created or not
    read(sock, &res, sizeof(int));
    if(res == -1){
        printf("Project already exists!\n");
        return;
    }
    else{
        strcpy(buff, "./");
        strcat(buff, project_name);
        mkdir(buff, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        strcat(buff, "/.Manifest");
        wrt = creat(buff, S_IRWXU);
        read(sock, &buff2, 2);
        write(wrt, buff2, strlen(buff2));
        close(wrt);
    }
    close(sock);
    return;
}
//add file to client manifest
void add(char *project, char *filename){
    FILE *fp, *tmpfp;
    int rd, curr, i;
    size_t size;
    char buff[1024];
    unsigned char hash[21];
    char *token, line[1024], line2[1024], *ver = NULL;
    strcpy(buff, "./");
    strcat(buff, project);
    strcat(buff, "/.Manifest"); 
    fp = fopen(buff, "a+");
    tmpfp = tmpfile();
    //if manifest doesnt exist project doesnt exist
    if(fp == NULL){
        printf("Project doesn't exist!\n");
        return;
    }
    //read through .Manifest
    while(1){
        if(fgets(line, 1024, fp) == NULL){
            break;
        }
        strcpy(line2, line);
        token = strtok(line2, "\t\n");
        //if manifest contains file return and dont add it
        if(strcmp(token, filename) == 0){
            printf("File already added to .Manifest! Hash updated!\n");
            //get version number to save
            token = strtok(NULL, "\t\n");
            ver = (char*)malloc(sizeof(char)*strlen(token)+1);
            strcpy(ver,token);
        }
        else{
            fprintf(tmpfp, "%s", line);
        }
    }
    //close manifest and open again to be overriden
    fclose(fp);
    fp = fopen(buff, "w");
    rewind(tmpfp);
    while(1){
        if(fgets(line, 1024, tmpfp) == NULL){
            break;
        }
        fprintf(fp, "%s", line);
    }
    fclose(tmpfp);
    //file isnt in manifest so generate hash and set version=1 and add
    rd = open(filename, O_RDONLY);
    if(rd < 0){
        printf("Failed to find file of given path!\n");
        return;
    }
    curr = lseek(rd, 0, SEEK_CUR);
    size = lseek(rd, 0, SEEK_END);
    lseek(rd, curr, SEEK_SET);
    char file[size+1];
    file[size] = '\0';
    read(rd, file, size);
    //generate hash from file contents
    SHA1(file, size, hash);
    //add filename followed by tab
    fprintf(fp, "%s\t", filename);
    //add version number followed by tab
    if(ver ==  NULL){
        fprintf(fp, "1\t");
    }
    else{
        fprintf(fp, "%s\t", ver);
        free(ver);
    }
    //add hash followed by tab
    for(i = 0; i<20; i++){
        fprintf(fp, "%02x", hash[i]);
    }
    fprintf(fp, "\n");
    //close file
    fclose(fp);
    return;
}
//remove file from .Manifest
void wtf_remove(char *project, char *filename){
    FILE *fp, *tmpfp;
    int rem = 0;
    char buff[1024];
    char *token, line[1024], line2[1024];
    strcpy(buff, "./");
    strcat(buff, project);
    strcat(buff, "/.Manifest"); 
    fp = fopen(buff, "a+");
    tmpfp = tmpfile();
    //if manifest doesnt exist project doesnt exist
    if(fp == NULL){
        printf("Project doesn't exist!\n");
        return;
    }
    //read through .Manifest
    while(1){
        if(fgets(line, 1024, fp) == NULL){
            break;
        }
        strcpy(line2, line);
        token = strtok(line2, "\t\n");
        //if manifest contains file dont add to tmp file
        if(strcmp(token, filename) == 0){
            printf("File removed from Manifest!\n");
            rem = 1;
        }
        else{
            fprintf(tmpfp, "%s", line);
        }
    }
    //if rem is 0 after going thru manifest return and tell user
    if(rem == 0){
        printf("File not in Manifest!\n");
        fclose(tmpfp);
        fclose(fp);
        return;
    }
    //close manifest and open again to be overriden
    fclose(fp);
    fp = fopen(buff, "w");
    rewind(tmpfp);
    while(1){
        if(fgets(line, 1024, tmpfp) == NULL){
            break;
        }
        fprintf(fp, "%s", line);
    }
    //close files and return
    fclose(tmpfp);
    fclose(fp);
    return;
}
//get project from server
void checkout(char *project_name){
    int sock = create_socket();
    int res, size, i, wrt;
    char buff = '\0';
    char path[1024];
    char command[1024];
    strcpy(path, "./");
    strcat(path, project_name);
    strcat(path, ".tar.gz");
    if(sock == -1){
        printf("failed to connect to server!\n");
    }
    //send length of command
    int n = strlen("checkout");
    send(sock, &n, sizeof(int), 0);
    //send command to server
    send(sock , "checkout" , strlen("checkout") , 0 );
    //send server name of project
    send(sock, project_name, strlen(project_name), 0);
    //recieve reponse on if server has project
    read(sock, &res, sizeof(int));
    if(res == -1){
        printf("Project doesn't exist!\n");
        close(sock);
        return;
    }
    wrt = creat(path, S_IRWXU);
    //get file size
    read(sock, &size, sizeof(int));
    for(i=0; i<size; i++){
        read(sock, &buff, 1);
        write(wrt, &buff, 1);
    }
    close(wrt);
    strcpy(command, "tar -zxf ");
    strcat(command, path);
    //unpack project
    system(command);
    //delete tar
    remove(path);

    return;

}
//get currentverison info from server
void currentver(char *project_name){
    char *man, line[1024], *token;
    int sock = create_socket();
    int res, size;
    FILE *tmpfp;
    if(sock == -1){
        printf("failed to connect to server!\n");
    }
    //send length of command
    int n = strlen("currentversion");
    send(sock, &n, sizeof(int), 0);
    //send command to server
    send(sock , "currentversion" , strlen("currentversion") , 0 );
    //send server name of project
    send(sock, project_name, strlen(project_name), 0);
    //recieve reponse on if the project was created or not
    read(sock, &res, sizeof(int));
    if(res == -1){
        printf("Project doesn't exist!\n");
        close(sock);
        return;
    }
    tmpfp = tmpfile();
    //read file size
    read(sock, &size, sizeof(int));
    man = (char*)malloc(sizeof(char)*size + 1);
    man[size] = '\0';
    //read file into buffer
    read(sock, man, size);
    //print into tmpfile
    fprintf(tmpfp, "%s", man);
    //free man
    free(man);
    rewind(tmpfp);
    fgets(line, 1024, tmpfp);
    printf("Project Version: %s", line);
    while(1){
        if(fgets(line, 1024, tmpfp) == NULL){
            printf("--END OF MANIFEST--\n");
            break;
        }
        token = strtok(line, "\t\n");
        printf("File: %s\t", token);
        token = strtok(NULL, "\t\n");
        printf("Version: %s\n", token);
    }
    //close tmpfile
    fclose(tmpfp);
    return;
}
//destroy project on server
void destroy(char *project_name){
    int sock = create_socket();
    int res;
    if(sock == -1){
        printf("failed to connect to server!\n");
    }
    //send length of command
    int n = strlen("destroy");
    send(sock, &n, sizeof(int), 0);
    //send command to server
    send(sock , "destroy" , strlen("destroy") , 0 );
    //send server name of project
    send(sock, project_name, strlen(project_name), 0);
    //recieve reponse on if server has project
    read(sock, &res, sizeof(int));
    if(res == -1){
        printf("Failed to destroy! Server doesn't have project!\n");
        close(sock);
        return;
    }
    printf("Project successfully destroyed!\n");
    close(sock);
    return;
}
//update get .Manifest from server and compare to client
void update(char *project_name){
    int sock = create_socket();
    int res, size, in_file = 0, rd, fsize, curr, i;
    char *buff, *ctoken, *stoken, *file;
    char path[1024], path2[1024], c_line[1024], s_line[1024], ver[20];
    unsigned char hash[21] = {'\0'}, hash2[41];
    //path for client manifest
    strcpy(path, "./");
    strcat(path, project_name);
    strcat(path, "/.Manifest");
    //path for output .Update
    strcpy(path2, "./");
    strcat(path2, project_name);
    strcat(path2, "/.Update");
    //file pointers for client mainfest, server mainfest and output .Update
    FILE *c_man, *s_man, *update;
    if(sock == -1){
        printf("Failed to connect to server!\n");
    }
    //send length of command
    int n = strlen("update");
    send(sock, &n, sizeof(int), 0);
    //send command to server
    send(sock , "update" , strlen("update") , 0 );
    //send server name of project
    send(sock, project_name, strlen(project_name), 0);
    //recieve reponse on if server has project
    read(sock, &res, sizeof(int));
    if(res == -1){
        printf("Failed Update! Server doesn't have project!\n");
        close(sock);
        return;
    }
    //create tmp file for server mainfest
    s_man = tmpfile();
    //read size of manifest
    read(sock, &size, sizeof(int));
    buff = (char*)malloc(sizeof(char)*(size + 1));
    buff[size] = '\0';
    read(sock, buff, size);
    //put buffer into tmpfile
    fprintf(s_man, "%s", buff);
    rewind(s_man);
    //free buff
    free(buff);
    //open client manifest
    c_man = fopen(path, "r");
    //create file for .Update
    update =  fopen(path2, "w");
    //check for files that are in server manifest but not clients (A)
    //skip project ver # of server
    fgets(s_line, 1024, s_man);
    while(1){
        //get line from server manifest to compare to client
        if(fgets(s_line, 1024, s_man) == NULL){
            //printf("--END OF SERVER MANIFEST--\n");
            break;
        }
        //get path of file in server manifest
        stoken = strtok(s_line, "\t\n");
        //scan through client and see if its absent
        //skip project ver # of client
        fgets(c_line, 1024, c_man);
        while(1){
            if(fgets(c_line, 1024, c_man) == NULL){
                //printf("--END OF CLIENT MANIFEST--\n");
                break;
            }
            //get path of file in client manifest
            ctoken = strtok(c_line, "\t\n");
            //if file paths are the same not added
            if(strcmp(ctoken, stoken) == 0){
                //update indicator to true 
                in_file = 1;
                break;
            }
        }
        //rewind pointer of client back to beginning
        rewind(c_man);
        //both client and server have the file
        if(in_file == 1){
            //set back to 0
            in_file = 0;
        }
        //else add it to .Update
        else{
            //print path to .Update
            fprintf(update, "A\t%s\t", stoken);
            stoken = strtok(NULL, "\t\n");
            //print version to .Update
            fprintf(update, "%s\t", stoken);
            stoken = strtok(NULL, "\t\n");
            //print hash to .Update with an append A for add
            fprintf(update, "%s\n", stoken);
        }
    }
    //rewind both server and client manifest to check for deleted files
    rewind(c_man);
    rewind(s_man);
    //skip project ver # of client
    fgets(c_line, 1024, c_man);
    //check for files that are in client but not server (D)
    while(1){
        //get line from client manifest to compare to server
        if(fgets(c_line, 1024, c_man) == NULL){
            //printf("--END OF CLIENT MANIFEST--\n");
            break;
        }
        //get path of file in client manifest
        ctoken = strtok(c_line, "\t\n");
        //scan through server and see if its absent
        //skip project ver # of servr
        fgets(s_line, 1024, s_man);
        while(1){
            if(fgets(s_line, 1024, s_man) == NULL){
                //printf("--END OF SERVER MANIFEST--\n");
                break;
            }
            //get path of file in server manifest
            stoken = strtok(s_line, "\t\n");
            //if file paths are the same not deleted
            if(strcmp(ctoken, stoken) == 0){
                //update indicator to true 
                in_file = 1;
                break;
            }
        }
        //rewind pointer of server back to beginning
        rewind(s_man);
        //both client and server have the file
        if(in_file == 1){
            //set back to 0
            in_file = 0;
        }
        //else add it to .Update
        else{
            //print path to .Update
            fprintf(update, "D\t%s\t", ctoken);
            ctoken = strtok(NULL, "\t\n");
            //print version to .Update
            fprintf(update, "%s\t", ctoken);
            ctoken = strtok(NULL, "\t\n");
            //print hash to .Update with an append D for delete
            fprintf(update, "%s\n", ctoken);
        }
    }
    //rewind both client and server mani to check for modified files
    rewind(c_man);
    rewind(s_man);
    //check for modified files (M)
    //skip project ver # of client
    fgets(c_line, 1024, c_man);
    while(1){
        //get line from client manifest to compare to server
        if(fgets(c_line, 1024, c_man) == NULL){
            //printf("--END OF CLIENT MANIFEST--\n");
            break;
        }
        //get path of file in client manifest
        ctoken = strtok(c_line, "\t\n");
        //scan through server and find file to compare hashes
        //skip project ver # of servr
        fgets(s_line, 1024, s_man);
        while(1){
            if(fgets(s_line, 1024, s_man) == NULL){
                //printf("--END OF SERVER MANIFEST--\n");
                break;
            }
            //get path of file in server manifest
            stoken = strtok(s_line, "\t\n");
            //if file paths are the same generate new hash for client and compare to server
            if(strcmp(ctoken, stoken) == 0){
                //open file from client manifest
                rd = open(ctoken, O_RDONLY);
                //get size of file
                curr = lseek(rd, 0, SEEK_CUR);
                fsize = lseek(rd, 0, SEEK_END);
                lseek(rd, curr, SEEK_SET);
                file = (char*)malloc(sizeof(char)*fsize + 1);
                file[fsize] = '\0';
                //read file contents into buffer
                read(rd, file, size);
                //generate hash from file contents
                SHA1(file, fsize, hash);
                for(i=0; i<20; i++){
                    sprintf((char*)&hash2[i*2], "%02x", hash[i]);
                }
                //get and copy version 
                stoken = strtok(NULL, "\t\n");
                strcpy(ver, stoken);
                //get hash
                stoken = strtok(NULL, "\t\n");
                printf("Server: %s\t Client: %s\n", stoken, hash2);
                //compare hashes and add to .Update if different
                if(strcmp(stoken, hash2) != 0){
                    //print path
                    fprintf(update, "M\t%s\t", ctoken);
                    //print server version
                    fprintf(update, "%s\t", ver);
                    //print server hash followed by M
                    fprintf(update, "%s\n", stoken);
                }
                free(file);
            }
        }
        //rewind pointer of server back to beginning
        rewind(s_man);
    }
    //close c_man and s_man
    fclose(c_man);
    fclose(s_man);
    fclose(update);
    //reopen update to read only
    update =  fopen(path2, "r");
    //check if .Update is empty
    fseek(update, 0, SEEK_END);
    //file is empty delete it and return
    if( 0 == ftell(update)){
        fclose(update);
        printf(".Update is empty!\n");
        return;
    }
    //file isnt empty rewind to print
    rewind(update);
    //print out contents of .Update
    while(1){
        if(fgets(c_line, 1024, update) == NULL){
            break;
        }
        printf("%s", c_line);
    }
    //close update
    fclose(update);
    close(sock);
    return;
}
//perform operations from .Update
void upgrade(char *project_name){
    int res, i, size, done = 1, wrt;
    char path[1024], *token, u_line[1024], fpath[1024], command[1024], spath[1024] = {'\0'}, man[1024], buff = '\0';
    //path for output .Update
    strcpy(path, "./");
    strcat(path, project_name);
    strcat(path, "/.Update");
    //get path to manifest
    strcpy(man, "./");
    strcat(man, project_name);
    strcat(man, "/.Manifest");
    //file pointers for client mainfest, server mainfest and output .Update
    FILE *update, *ovrwrt;
    update = fopen(path, "r");
    //.Update doesnt exist tell user to run update
    if(update == NULL){
        printf("No .Update! Please run update first!\n");
        return;
    }
    //check if .Update is empty
    fseek(update, 0, SEEK_END);
    //file is empty delete it and return
    if( 0 == ftell(update)){
        fclose(update);
        //delete empty .Update
        remove(path);
        printf("Project already up-to-date\n");
        return;
    }
    int sock = create_socket();
    if(sock == -1){
        printf("Failed to connect to server!\n");
    }
    //send length of command
    int n = strlen("upgrade");
    send(sock, &n, sizeof(int), 0);
    //send command to server
    send(sock , "upgrade" , strlen("upgrade") , 0 );
    //send server name of project
    send(sock, project_name, strlen(project_name), 0);
    //recieve reponse on if server has project
    read(sock, &res, sizeof(int));
    if(res == -1){
        printf("Failed Update! Server doesn't have project!\n");
        close(sock);
        return;
    }
    //file isnt empty rewind to print
    rewind(update);
    //go through .Update and perform MAD
    while(1){
        if(fgets(u_line, 1024, update) == NULL){
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
            //creates all directories if they dont exist
            strcpy(command, "mkdir -p ");
            strcat(command, spath);
            system(command);
            //tell server we are requesting file
            send(sock, &done, sizeof(int), 0);
            //ask server for file
            send(sock, &fpath[1], 1023, 0);
            // get file size and read file from server
            read(sock, &size, sizeof(int));
            //create file
            wrt = creat(fpath, S_IRWXU);
            for(i=0; i<size; i++){
                read(sock, &buff, 1);
                write(wrt, &buff, 1);
            }
            //close file
            close(wrt);
            //inform server we are still requesting files
            send(sock, &done, sizeof(int), 0);
        }
        //file is being override with one from server
        else if(strcmp(token, "M") == 0){
            //get file path
            token = strtok(NULL, "\t\n");
            strcpy(fpath, token);
            //open file to be overwritten
            ovrwrt = fopen(fpath, "w");
            //tell server we are requesting file
            send(sock, &done, sizeof(int), 0);
            //ask server for file
            send(sock, &fpath[1], 1023, 0);
            // get file size and read file from server
            read(sock, &size, sizeof(int));
            for(i=0; i<size; i++){
                read(sock, &buff, 1);
                fputc(buff, ovrwrt);
            }
            //close file
            fclose(ovrwrt);
            //inform server we are still requesting files
            send(sock, &done, sizeof(int), 0);
        }
        //file is being deleted
        else if(strcmp(token, "D") == 0){
            //get file path
            token = strtok(NULL, "\t\n");
            //delete file
            remove(token);
        }
    }
    //inform server we are done requesting files
    done = 0;
    send(sock, &done, sizeof(int), 0);
    //close & delete update
    fclose(update);
    remove(path);
    //overwrite .Manifest
    ovrwrt = fopen(man, "w");
    // get file size and read file from server
    read(sock, &size, sizeof(int));
    for(i=0; i<size; i++){
        read(sock, &buff, 1);
        fputc(buff, ovrwrt);
    }
    //close manifest file
    fclose(ovrwrt);
    //close  socket and return
    close(sock);
    return;
}
//client wants to commit changes to the repository
void commit(char *project_name){
    int sock = create_socket();
    int res, size, in_file = 0, rd, fsize, curr, i;
    char *buff, *ctoken, *stoken, *file;
    char path[1024], path2[1024], path3[1024], c_line[1024], s_line[1024], ver[50];
    unsigned char hash[20], hash2[41];
    FILE *c_man, *s_man, *commit, *update;
    //path for client manifest
    strcpy(path, "./");
    strcat(path, project_name);
    strcat(path, "/.Manifest");
    //path for output .Commit
    strcpy(path2, "./");
    strcat(path2, project_name);
    strcat(path2, "/.Commit");
    //path for output .Update
    strcpy(path3, "./");
    strcat(path3, project_name);
    strcat(path3, "/.Update");
    //check to make sure update is empty
    update = fopen(path3, "r");
    if(update != NULL){
        //get size of update
        fseek(update, 0, SEEK_END);
        //file isnt empty user needs to upgrade
        if( 0 != ftell(update)){
            fclose(update);
            printf(".Update isnt empty! Run upgrade first!\n");
            return;
        }
        fclose(update);
    }
    //if cant connect to server return
    if(sock == -1){
        printf("Failed to connect to server!\n");
        return;
    }
    //send length of command
    int n = strlen("commit");
    send(sock, &n, sizeof(int), 0);
    //send command to server
    send(sock , "commit" , strlen("commit") , 0 );
    //send server name of project
    send(sock, project_name, strlen(project_name), 0);
    //recieve reponse on if server has project
    read(sock, &res, sizeof(int));
    if(res == -1){
        printf("Failed Update! Server doesn't have project!\n");
        close(sock);
        return;
    }
    //get servers .Manifest
    s_man = tmpfile();
    //read size of manifest
    read(sock, &size, sizeof(int));
    buff = (char*)malloc(sizeof(char)*(size + 1));
    buff[size] = '\0';
    read(sock, buff, size);
    //put buffer into tmpfile
    fprintf(s_man, "%s", buff);
    rewind(s_man);
    //free buff
    free(buff);
    //open client .Manifest
    c_man = fopen(path, "r");
    //compare project version numbers
    fgets(s_line, 1024, s_man);
    fgets(c_line, 1024, c_man);
    if(strcmp(s_line, c_line) != 0){
        printf("Update project before trying to commit changes!\n");
        fclose(s_man);
        fclose(c_man);
        close(sock);
        return;
    }
    commit = fopen(path2, "w");
    //go through client & server manifests find what needs to be deleted from server
    //skip project version
    fgets(s_line, 1024, s_man);
    while(1){
        //get line from server manifest to compare to client
        if(fgets(s_line, 1024, s_man) == NULL){
            //printf("--END OF SERVER MANIFEST--\n");
            break;
        }
        //get path of file in server manifest
        strcpy(path3, s_line);
        stoken = strtok(s_line, "\t\n");
        //scan through client and see if its absent
        //skip project ver # of client
        fgets(c_line, 1024, c_man);
        while(1){
            if(fgets(c_line, 1024, c_man) == NULL){
                //printf("--END OF CLIENT MANIFEST--\n");
                break;
            }
            //get path of file in client manifest
            ctoken = strtok(c_line, "\t\n");
            //if file paths are the same not added
            if(strcmp(ctoken, stoken) == 0){
                //update indicator to true 
                in_file = 1;
                break;
            }
        }
        //rewind pointer of client back to beginning
        rewind(c_man);
        //both client and server have the file
        if(in_file == 1){
            //set back to 0
            in_file = 0;
        }
        //else add it to .Update
        else{
            //print path to .Update
            fprintf(commit, "D\t%s", path3);
        }
    }
    //rewind both server and client manifest to check for deleted files
    rewind(c_man);
    rewind(s_man);
    //go through manifests and find files that need to be added to the server
    //skip project ver # of client
    fgets(c_line, 1024, c_man);
    //check for files that are in client but not server (A)
    while(1){
        //get line from client manifest to compare to server
        if(fgets(c_line, 1024, c_man) == NULL){
            //printf("--END OF CLIENT MANIFEST--\n");
            break;
        }
        //get path of file in client manifest
        strcpy(path3, c_line);
        ctoken = strtok(c_line, "\t\n");
        //scan through server and see if its absent
        //skip project ver # of servr
        fgets(s_line, 1024, s_man);
        while(1){
            if(fgets(s_line, 1024, s_man) == NULL){
                //printf("--END OF SERVER MANIFEST--\n");
                break;
            }
            //get path of file in server manifest
            stoken = strtok(s_line, "\t\n");
            //if file paths are the same not being added
            if(strcmp(ctoken, stoken) == 0){
                //update indicator to true 
                in_file = 1;
                break;
            }
        }
        //rewind pointer of server back to beginning
        rewind(s_man);
        //both client and server have the file
        if(in_file == 1){
            //set back to 0
            in_file = 0;
        }
        //else add it to .Update
        else{
            //print path to .Update
            fprintf(commit, "A\t%s", path3);
        }
    }
    //check for files the client changed
    //rewind both client and server mani to check for modified files
    rewind(c_man);
    rewind(s_man);
    //check for modified files (M)
    //skip project ver # of client
    fgets(c_line, 1024, c_man);
    while(1){
        //get line from client manifest to compare to server
        if(fgets(c_line, 1024, c_man) == NULL){
            //printf("--END OF CLIENT MANIFEST--\n");
            break;
        }
        //get path of file in client manifest
        ctoken = strtok(c_line, "\t\n");
        //scan through server and find file to compare hashes
        //skip project ver # of servr
        fgets(s_line, 1024, s_man);
        while(1){
            if(fgets(s_line, 1024, s_man) == NULL){
                //printf("--END OF SERVER MANIFEST--\n");
                break;
            }
            //get path of file in server manifest
            stoken = strtok(s_line, "\t\n");
            //if file paths are the same generate new hash for client and compare to server
            if(strcmp(ctoken, stoken) == 0){
                //open file from client manifest
                rd = open(ctoken, O_RDONLY);
                //get size of file
                curr = lseek(rd, 0, SEEK_CUR);
                fsize = lseek(rd, 0, SEEK_END);
                lseek(rd, curr, SEEK_SET);
                file = (char*)malloc(sizeof(char)*fsize + 1);
                file[fsize] = '\0';
                //read file contents into buffer
                read(rd, file, size);
                //generate hash from file contents
                SHA1(file, fsize, hash);
                for(i=0; i<20; i++){
                    sprintf((char*)&hash2[i*2], "%02x", hash[i]);
                }
                //get and copy version 
                stoken = strtok(NULL, "\t\n");
                strcpy(ver, stoken);
                //get hash
                stoken = strtok(NULL, "\t\n");
                //compare hashes and add to .Update if different
                if(strcmp(stoken, hash) != 0){
                    //print path
                    fprintf(commit, "M\t%s\t", ctoken);
                    //print  version incremented
                    int v = atoi(ver);
                    v++;
                    sprintf(ver, "%d", v);
                    fprintf(commit, "%s\t", ver);
                    //print client hash followed by M
                    for(i = 0; i<20; i++){
                        fprintf(commit, "%02x", hash[i]);
                    }
                }
                free(file);
            }
        }
        //rewind pointer of server back to beginning
        rewind(s_man);
    }
    //close c_man and s_man
    fclose(c_man);
    fclose(s_man);
    fclose(commit);
    close(sock);
    return;
}
//send server .Commit with files to add
void push(char *project_name){ 
    int res, i, size, done = 1, wrt, status = -1, curr, fp;
    char path[1024], *token, u_line[1024], fpath[1024], command[1024], spath[1024] = {'\0'}, man[1024], buff = '\0';
    //path for output .Update
    strcpy(path, "./");
    strcat(path, project_name);
    strcat(path, "/.Commit");
    //get path to manifest
    strcpy(man, "./");
    strcat(man, project_name);
    strcat(man, "/.Manifest");
    //file pointers for client mainfest, server mainfest and output .Update
    FILE *commit, *ovrwrt;
    commit = fopen(path, "r");
    //.Update doesnt exist tell user to run update
    if(commit == NULL){
        printf("No .Commit! Please run commit first!\n");
        return;
    }
    //check if .Update is empty
    fseek(commit, 0, SEEK_END);
    //file is empty delete it and return
    if( 0 == ftell(commit)){
        fclose(commit);
        //delete empty .Update
        remove(path);
        printf("Project hasnt changed!\n");
        return;
    }
    //file isnt empty rewind to print
    rewind(commit);
    int sock = create_socket();
    if(sock == -1){
        printf("Failed to connect to server!\n");
        return;
    }
    //send length of command
    int n = strlen("push");
    send(sock, &n, sizeof(int), 0);
    //send command to server
    send(sock , "push" , strlen("push") , 0 );
    //send server name of project
    send(sock, project_name, strlen(project_name), 0);
    //recieve reponse on if server has project
    read(sock, &res, sizeof(int));
    if(res == -1){
        printf("Failed Push! Server doesn't have project!\n");
        close(sock);
        return;
    }
    //send server size of .Commit
    fseek(commit, 0, SEEK_END);
    size = ftell(commit);
    send(sock, &size, sizeof(int), 0);
    rewind(commit);
    //send server .Commit
    for(i = 0; i <size; i++){
        buff = fgetc(commit);
        send(sock, &buff, sizeof(char), 0);
    }
    //close .Commit
    fclose(commit);
    //send server requested files
    while(done){
        //check if client is requesting file
        read(sock, &done, sizeof(int));
        //if client is not requesting file break
        if(done == 0){
            break;
        }
        //read file path name
        read(sock, fpath, 1023);
        //open file
        fp = open(fpath, O_RDONLY);
        //get file size
        curr = lseek(fp, 0, SEEK_CUR);
        size = lseek(fp, 0, SEEK_END);
        lseek(fp, curr, SEEK_SET);
        //send size of file to client
        send(sock, &size, sizeof(int), 0);
        //send file contents
        status = read(fp, &buff, 1);
        while(status > 0){
            send(sock, &buff, 1, 0);
            status = read(fp, &buff, 1);
        }
        close(fp);
        //read to see if client is done requesting files
        read(sock, &done, sizeof(int));
    }
    //recieve new manifest
    ovrwrt = fopen(man, "w");
    read(sock, &size, sizeof(int));
    for(i=0; i<size; i++){
        read(sock, &buff, 1);
        fputc(buff, ovrwrt);
    }
    fclose(ovrwrt);
    //all done return
    close(sock);
    remove(path);
    return;
}
//get history of project
void history(char *project_name){
    char *man, line[1024], *token;
    int sock = create_socket();
    int res, size;
    FILE *tmpfp;
    if(sock == -1){
        printf("failed to connect to server!\n");
    }
    //send length of command
    int n = strlen("history");
    send(sock, &n, sizeof(int), 0);
    //send command to server
    send(sock , "history" , strlen("history") , 0 );
    //send server name of project
    send(sock, project_name, strlen(project_name), 0);
    //recieve reponse on if the project was created or not
    read(sock, &res, sizeof(int));
    if(res == -1){
        printf("Project doesn't exist!\n");
        close(sock);
        return;
    }
    tmpfp = tmpfile();
    //read file size
    read(sock, &size, sizeof(int));
    man = (char*)malloc(sizeof(char)*size + 1);
    man[size] = '\0';
    //read file into buffer
    read(sock, man, size);
    //print into tmpfile
    fprintf(tmpfp, "%s", man);
    //free man
    free(man);
    rewind(tmpfp);
    while(1){
        if(fgets(line, 1024, tmpfp) == NULL){
            printf("\n--END OF HISTORY--\n");
            break;
        }
        printf("%s", line);
       
    }
    //close tmpfile
    fclose(tmpfp);
    return;
}
//rollback project
void rollback(char *project_name, char *version){
    int sock = create_socket();
    int res, size, i, wrt;
    char buff = '\0';
    char path[1024];
    char command[1024];
    strcpy(path, "./");
    strcat(path, project_name);
    strcat(path, version);
    strcat(path, ".tar.gz");
    if(sock == -1){
        printf("failed to connect to server!\n");
        return;
    }
    //send length of command
    int n = strlen("rollback");
    send(sock, &n, sizeof(int), 0);
    //send command to server
    send(sock , "rollback" , strlen("rollback") , 0 );
    //send server name of project
    n = strlen(project_name);
    send(sock, &n, sizeof(int), 0);
    send(sock, project_name, strlen(project_name), 0);
    n = strlen(version);
    send(sock, &n, sizeof(int), 0);
    send(sock, version, strlen(version), 0);
    //recieve reponse on if server has project
    read(sock, &res, sizeof(int));
    if(res == -1){
        printf("Project doesn't exist!\n");
        close(sock);
        return;
    }
    //remove existing project if there is one
    strcpy(command, "rm -rf ");
    strcat(command, project_name);
    system(command);
    wrt = creat(path, S_IRWXU);
    //get file size
    read(sock, &size, sizeof(int));
    for(i=0; i<size; i++){
        read(sock, &buff, 1);
        write(wrt, &buff, 1);
    }
    close(wrt);
    strcpy(command, "tar -zxf ");
    strcat(command, path);
    //unpack project
    system(command);
    //delete tar
    remove(path);

    return;

}