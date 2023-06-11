#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

pthread_mutex_t Login_mutex = PTHREAD_MUTEX_INITIALIZER;

void *client_thread(void *);

int users[64];      // Mang socket client da dang nhap
char *user_ids[64]; // Mang luu tru id cua client da dang nhap
int num_users = 0;  // So client da dang nhap

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
    addr.sin_port = htons(9090);

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
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
        {
            printf("Client %d disconnected.\n", client);
            close(client);

            pthread_mutex_lock(&Login_mutex);
            for (int l = 0; l < num_users; l++)
                    {
                        if (users[l]==client){
                            users[l]=users[num_users-1];
                            memcpy(&user_ids[l], &user_ids[num_users-1], strlen(user_ids[num_users-1]));
                            num_users--;
                        }
                    }            
            pthread_mutex_unlock(&Login_mutex);
            pthread_exit(NULL);
        }
        buf[ret] = 0;
        printf("Received from %d: %s\n", client, buf);

                    // Kiem tra trang thai dang nhap cua client
                        int j = 0;
                        for (; j < num_users; j++)
                            if (users[j] == client) break;

                        if (j == num_users)
                        {
                            // Chua dang nhap
                            // Xu ly cu phap yeu cau dang nhap
                            char cmd[32], id[32], tmp[32];
                            ret = sscanf(buf, "%s%s%s", cmd, id, tmp);
                            if (ret == 2)
                            {
                                if (strcmp(cmd, "client_id:") == 0)
                                {
                                   

                                    int k = 0;
                                    for (; k < num_users; k++)
                                        if (strcmp(user_ids[k], id) == 0) break;
                                    
                                    if (k < num_users)
                                    {
                                        char *msg = "ID da ton tai. Yeu cau nhap lai.\n";
                                        send(client, msg, strlen(msg), 0);
                                    }
                                    else
                                    {
                                        pthread_mutex_lock(&Login_mutex);
                                        users[num_users] = client;
                                        user_ids[num_users] = malloc(strlen(id) + 1);
                                        strcpy(user_ids[num_users], id);
                                        num_users++;
                                        pthread_mutex_unlock(&Login_mutex);
                                        char *msg = "Dung cu phap. Gui Tin nhan\n";
                                        send(client, msg, strlen(msg), 0);
                                    }                                    
                                }
                                else
                                {
                                    char *msg = "Nhap sai. Yeu cau nhap lai.\n";
                                    send(client, msg, strlen(msg), 0);
                                }
                            }
                            else
                            {
                                char *msg = "Nhap sai. Yeu cau nhap lai.\n";
                                send(client, msg, strlen(msg), 0);
                            }
                        }
                        else {
                            // Da dang nhap
                            char target_id[32];
                            ret = sscanf(buf, "%s", target_id);
                            
                            char sendbuf[512];
                            sprintf(sendbuf, "%s: %s", user_ids[j], buf + strlen(target_id) + 1);

                            if (strcmp(target_id, "all") == 0)
                            {
                                for (int k = 0; k < num_users; k++)
                                    if (users[k] != client)
                                        send(users[k], sendbuf, strlen(sendbuf), 0);
                            }
                            else
                            {
                                for (int k = 0; k < num_users; k++)
                                    if (strcmp(user_ids[k], target_id) == 0)
                                        send(users[k], sendbuf, strlen(sendbuf), 0);
                            }        
                        }
    }

}