#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>

#define TRUE 1

int connectServer(int port) {
    int fd;
    struct sockaddr_in server_address;
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
    
    server_address.sin_family = AF_INET; 
    server_address.sin_port = htons(port); 
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)  // checking for errors
        printf("Error in connecting to server\n");

    return fd;
}

int convert_int(char buff[]) 
{
    int zarib = 1, integer_number = 0;
    for(int i = 4 ; i >= 1; i--)
    {
        integer_number += (buff[i] - '0')*zarib;
        zarib *= 10;
    }
    return integer_number;
}

void alarm_handler(int sig)
{
    printf("Your time is over. You can't answer anymore.\n");
}

int creat_room(int port ,int question_turn, int fd)
{
    int sock, broadcast = 1, opt = 1;
    char buffer[1024] = {0};
    char question[100] = {0};
    char answer1[100] = {0};
    char answer2[100] = {0};
    char best_answer[100];
    struct sockaddr_in bc_address;


    sock = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bc_address.sin_family = AF_INET; 
    bc_address.sin_port = htons(port); 
    bc_address.sin_addr.s_addr = inet_addr("255.255.255.255");

    bind(sock, (struct sockaddr *)&bc_address, sizeof(bc_address));        

    for(int turn = 1 ; turn < 4 ; turn++)
    {
        if(question_turn == turn)
        {
            write(1, "Ask your question \n\n", 18);
            memset(question, 0, 100);
            read(0, question, 100);
            int a = sendto(sock, question, strlen(question), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));
            recv(sock, question, 1024, 0);
            
            recv(sock, answer1, 1024, 0);
            if(!strcmp(answer1, "pass\n"))
                write(1, "The client doesn't know the answer\n", 36);
            else
                printf("The first answer is: %s \n", answer1);
            
            recv(sock, answer2, 1024, 0);
            if(!strcmp(answer2, "pass\n"))
                write(1, "The client doesn't know the answer\n", 36);
            else
                printf("The second answer is: %s \n", answer2);
            
            write(1, "In my opinion the best answer is for client(first or second): \n", 64);
            memset(best_answer, 0, 5);
            read(0, best_answer, 5);

            if((best_answer[0]-'0') == 1)
            {
                write(1, answer1, strlen(answer1));
                int a = sendto(sock, answer1, strlen(answer1), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));
                recv(sock, buffer, 1024, 0);
                sprintf(best_answer, "The best answer is answer %c \n", '1');
            }
            else if((best_answer[0]-'0') == 2)
            {
                write(1, answer2, strlen(answer2));
                int a = sendto(sock, answer2, strlen(answer2), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));
                recv(sock, buffer, 1024, 0);
                sprintf(best_answer, "The best answer is answer %c \n", '2');
            }
            else
            {
                printf("We don't have this answer id\n");
                int a = sendto(sock, "The best answer not determined\n", 32, 0,(struct sockaddr *)&bc_address, sizeof(bc_address));
                recv(sock, buffer, 1024, 0);
            }

            char send_to_server[1024];
            memset(send_to_server, 0, 1024);
            sprintf(send_to_server, "%s%s%s%s", question, answer1, answer2, best_answer);
            send(fd, send_to_server, strlen(send_to_server), 0);      
        }
        else
        {
            write(1,"Read the question then answer \n\n", 30);
            memset(buffer, 0, 1024);
            recv(sock, buffer, 1024, 0);
            printf("The question is: %s\n", buffer);

            signal(SIGALRM, alarm_handler);
            siginterrupt(SIGALRM, 1);
            for(int k = 0 ; k < 3 ; k++)
            {
                if(k == 2)
                {
                    memset(buffer, 0, 1024);
                    recv(sock, buffer, 1024, 0);
                    printf("The best answer is:%s ", buffer);
                    break;
                }
                if((turn + k) % 3 + 1 == question_turn)
                {
                    write(1,"You have only one minute to answer question: ", 45);
                    alarm(60);
                    memset(buffer, 0, 1024);
                    read(0, buffer, 1024);
                    int a = sendto(sock, buffer, strlen(buffer), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));
                    recv(sock, buffer, 1024, 0);
                    alarm(0);
                }
                else
                {
                    write(1,"Wait for others to answer\n", 26);
                    recv(sock, buffer, 1024, 0);
                }
            }
        }
    }
    close(sock);
    return TRUE;
}

int main(int argc, char const *argv[])
{
    int fd, portNum;
    char buff[1024] = {0};

    fd = connectServer(atoi(argv[1]));

    while (TRUE) 
    {
        read(0, buff, 1024);
        send(fd, buff, strlen(buff), 0);
        recv(fd, buff, 1024, 0);
        int exit_permit = creat_room(convert_int(buff), buff[0]-'0', fd);
        memset(buff, 0, 1024);
        if(exit_permit)
        {
            close(fd);
            break;
        }
    }
    
    return 0;
}