#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

char *format1 = "dd/mm/yyyy";
char *format2 = "dd/mm/yy";
char *format3 = "mm/dd/yyyy";
char *format4 = "mm/dd/yy";

void *client_thread(void *);

int main() 
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) 
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5)) 
    {
        perror("listen() failed");
        return 1;
    }

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            perror("accept() failed");
            continue;
        }
        printf("New client connected: %d\n", client);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, &client);
        pthread_detach(thread_id);
    }
    
    close(listener);    

    return 0;
}

void *client_thread(void *param)
{
    int client = *(int *)param;
    char buf[256];

    while (1)
    {   
        time_t t = time(NULL);
        struct tm tm= *localtime(&t);
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0){
            printf("Client %d disconnected.\n", client);
            close(client);
            pthread_exit(NULL);
        }
        buf[ret] = 0;
        printf("Received from %d: %s\n", client, buf);
                char cmd[32],format[32],tmp[256];

                int count = sscanf(buf, "%s%s%s",cmd,format,tmp);
                if (count == 2  ){
                    if (strcmp(cmd,"GET_TIME") == 0 )
                    {
                        if (strcmp(format,format1) == 0)
                        {
                            strftime(buf, sizeof(buf), "%d/%m/%Y", &tm);
                            sprintf(buf,"%s\n",buf);
                            send(client, buf, strlen(buf), 0);  
                        }
                        else if (strcmp(format,format2) == 0)
                        {
                            strftime(buf, sizeof(buf), "%d/%m/%y", &tm);
                            sprintf(buf,"%s\n",buf);
                            send(client, buf, strlen(buf), 0);  
                        }
                         else if (strcmp(format,format3) == 0)
                         {
                            strftime(buf, sizeof(buf), "%m/%d/%Y", &tm);
                            sprintf(buf,"%s\n",buf);
                            send(client, buf, strlen(buf), 0);  
                        } 
                        else if (strcmp(format,format4) == 0)
                        {
                            strftime(buf, sizeof(buf), "%m/%d/%y", &tm);
                            sprintf(buf,"%s\n",buf);
                            send(client, buf, strlen(buf), 0);  
                        }
                        else{
                            char *msg = "Nhap sai format yeu cau nhap dung format\n";
                            send(client, msg, strlen(msg), 0);  
                        }
                    }
                    else {
                        char *msg = "Nhap sai yeu cau nhap dung format\n";
                        send(client, msg, strlen(msg), 0);
                    }
    }
    
}
}