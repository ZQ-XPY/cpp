#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <signal.h>

#define PORT 7000
#define BUFFER_SIZE 1024

/*
 * @function: 进程退出时通知信号
 * @brief	: 
 * @param	: 
 * @return	: 
 */
void signalHandler(int sig)
{
    printf("recv signal:%d\n", sig);
    printf("parent exit\n");
    exit(0);
}

/*
 * @function: 同步阻塞模式
 * @brief	: 
 * @param	: 
 * @return	: 
 */
void client_block( )
{
    // 创建sockfd(默认为主动模式)
    int sock_cli = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);                 // 服务器端口
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 服务器ip inet_addr 地址格式转化, 把点分十进制转为二进制

    // connect 同步阻塞模式
    // 2. 建立到达服务器的链接(三次握手)
    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("connect");
        exit(1);
    }

    char sendbuf[BUFFER_SIZE];
    char recvbuf[BUFFER_SIZE];
    while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
    {
        // send 同步阻塞模式
        send(sock_cli, sendbuf, strlen(sendbuf), 0);
        if (strcmp(sendbuf, "exit\n") == 0)
            break;
        // 同步阻塞模式
        recv(sock_cli, recvbuf, sizeof(recvbuf), 0);
        fputs(recvbuf, stdout);

        memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));
    }

    close(sock_cli);
}

/*
 * @function: 同步非阻塞IO
 * @brief	: 
 * @param	: 
 * @return	: 
 */
void client_dontblock( )
{
    // 创建sockfd(默认为主动模式)
    int sock_cli = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);                 // 服务器端口
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 服务器ip inet_addr 地址格式转化, 把点分十进制转为二进制

    // connect 同步阻塞模式
    // 2. 建立到达服务器的链接(三次握手)
    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("connect");
        exit(1);
    }

    char sendbuf[BUFFER_SIZE];
    char recvbuf[BUFFER_SIZE];
    while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
    {

        if (strcmp(sendbuf, "exit\n") == 0)
            break;

        // send 同步非阻塞模式
        while (send(sock_cli, sendbuf, strlen(sendbuf), MSG_DONTWAIT) == -1)
        {
            sleep(10);
            printf("sleep\n");
        }

        // send 同步非阻塞模式
        while(recv(sock_cli, recvbuf, sizeof(recvbuf), MSG_DONTWAIT) == -1)
        {
            sleep(10);
            printf("sleep\n");
        }
        fputs(recvbuf, stdout);
        memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));
    }

    close(sock_cli);
}

/*
 * @function: 点对点通信
 * @brief	: 
 * @param	: 
 * @return	: 
 */
void peer2peer( )
{
    int sock_cli = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("connect");
        exit(1);
    }

    pid_t pid;
    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(1);
    }
    else if (pid == 0)  // 子进程接收数据
    {
        char recvbuf[BUFFER_SIZE];
        while (true)
        {
            memset(recvbuf, 0, sizeof(recvbuf));
            int ret = read(sock_cli, recvbuf, sizeof(recvbuf));
            if (ret == -1)
            {
                perror("read");
            }
            else if (ret == 0) // 服务端已关闭
            {
                printf("service closed\n");
                kill(pid, SIGUSR1);
                break;
            }
            fputs(recvbuf, stdout);
        }
        close(sock_cli);
    }
    else // 父进程发送数据
    {
        signal(SIGUSR1, signalHandler); // 处理信号
        char sendbuf[BUFFER_SIZE] = {0};
        while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
        {
            write(sock_cli, sendbuf, sizeof(sendbuf));
            memset(sendbuf, 0, sizeof(sendbuf));
        }
        close(sock_cli);
    }
}

int main(int argc, char *argv[])
{
    // client_block();
    // client_dontblock();
    peer2peer();
    return 0;
}