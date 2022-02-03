#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>


int setupServer(int port) {
    struct sockaddr_in address;
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    
    listen(server_fd, 4);

    return server_fd;
}

int acceptClient(int server_fd) {
    int client_fd;
    struct sockaddr_in client_address;
    int address_len = sizeof(client_address);
    client_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*) &address_len);

    return client_fd;
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, max_sd;
    char buffer[1024] = {0};
    int is_in_room[1024] = {0};
    fd_set master_set, working_set;
    
    server_fd = setupServer(atoi(argv[1]));

    FD_ZERO(&master_set);
    max_sd = server_fd;
    FD_SET(server_fd, &master_set);

    write(1, "Server is running\n", 18);

    printf("Client!\nif you want computer room enter -> 1\nmechanic room -> 2\nelectronic room -> 3\ncivil room ->4\n");
    int comp_room_arr[3], elec_room_arr[3], civil_room_arr[3], mech_room_arr[3];
    int mech_room = 0, comp_room = 0, elec_room = 0, civil_room = 0;
    int room_port = 1;
    while (1) {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);
        for (int i = 0; i <= max_sd; i++) 
        {
            if (FD_ISSET(i, &working_set)) 
            {                
                if (i == server_fd) 
                {  // new clinet
                    new_socket = acceptClient(server_fd);
                    FD_SET(new_socket, &master_set);
                    if (new_socket > max_sd)
                        max_sd = new_socket;
                    printf("New client connected. fd = %d\n", new_socket);
                }
                
                else { // client sending msg
                    int bytes_received;
                    bytes_received = recv(i , buffer, 1024, 0);
                    
                    if (bytes_received == 0) 
                    { // EOF
                        printf("client fd = %d closed\n", i);
                        is_in_room[i] = 0;
                        close(i);
                        FD_CLR(i, &master_set);
                        continue;
                    }

                    else
                    {
                        if(is_in_room[i])
                        {
                            int file_fd;
                            file_fd = open("file.txt", O_APPEND | O_RDWR);
                            write(file_fd, buffer, strlen(buffer));
                            close(file_fd);
                        }
                        else if(buffer[0] == '1' && !is_in_room[i])
                        {
                            comp_room_arr[comp_room] = i;
                            comp_room ++;
                            printf("You are registerd in computer room\n");
                            if(comp_room == 3)
                            {
                                comp_room = 0;
                                for(int j = 0 ; j < 3 ; j++)
                                {
                                    sprintf(buffer, "%d%d", j+1, 8080+room_port);
                                    send(comp_room_arr[j], buffer , strlen(buffer), 0);
                                    is_in_room[comp_room_arr[j]] = 1;
                                }
                                room_port ++;
                                memset(comp_room_arr, 0, 3);
                                buffer[0] = '1';
                            }
                        }
                        else if(buffer[0] == '2' && !is_in_room[i])
                        {
                            mech_room_arr[mech_room] = i;
                            mech_room ++;
                            printf("You are registered in mechanic room\n");
                            if(mech_room == 3)
                            {
                                mech_room = 0;
                                for(int j = 0 ; j < 3 ; j++)
                                {
                                    sprintf(buffer, "%d%d", j+1, 8080+room_port);
                                    send(mech_room_arr[j], buffer , strlen(buffer), 0);
                                    is_in_room[mech_room_arr[j]] = 1;
                                }
                                room_port ++;
                                memset(mech_room_arr, 0, 3);
                            }
                        }
                        else if(buffer[0] == '3' && !is_in_room[i])
                        {
                            elec_room_arr[elec_room] = i;
                            elec_room ++;
                            printf("You are registered in electronic room\n");
                            if(elec_room == 3)
                            {
                                elec_room = 0;
                                for(int j = 0 ; j < 3 ; j++)
                                {
                                    sprintf(buffer,"%d%d", j+1, 8080+room_port);
                                    send(elec_room_arr[j], buffer , strlen(buffer), 0);
                                    is_in_room[elec_room_arr[j]] = 1;
                                }
                                room_port++;
                                memset(elec_room_arr, 0, 3);
                            }
                        }
                        else if(buffer[0] == '4' && !is_in_room[i])
                        {
                            civil_room_arr[civil_room] = i;
                            civil_room ++;
                            printf("You are registered in civil room\n");
                            if(civil_room == 3)
                            {
                                civil_room = 0;
                                for(int j = 0 ; j < 3 ; j++)
                                {
                                    sprintf(buffer, "%d%d", j+1, 8080+room_port);
                                    send(civil_room_arr[j], buffer , strlen(buffer), 0);
                                    is_in_room[civil_room_arr[j]] = 1;
                                }
                                memset(civil_room_arr, 0, 3);
                                room_port++;
                            }
                        }
                        else
                        {
                            write(1,"You are entered in incorrect number\n", 40);
                        }
                    }
                    if(!is_in_room[i])
                        printf("client %d: %s\n", i, buffer);
                    memset(buffer, 0, 1024);
                }
            }
        }
    }

    return 0;
}