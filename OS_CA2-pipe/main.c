#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define POSITION 12

#define FIFO_ADDRESS "/tmp/myfifo"

#define DIRECTION "./testcases/"
#define ADDRESS "./testcases/0.csv"

int count_file()
{
    
    int file_count = 0;
    DIR * dirp;
    struct dirent * entry;
    dirp = opendir(DIRECTION); 
    while ((entry = readdir(dirp)) != NULL) 
        if (entry->d_type == DT_REG)
            file_count++;
    closedir(dirp);
    return file_count;
}

int main()
{
    int file_count = count_file();
    char addr[] = ADDRESS;
    int size = 0;

    int fd[2];
    if(pipe(fd) == -1)
        printf("An error occured with opening the pupe\n");

    int fifo = mkfifo(FIFO_ADDRESS, 0777);
    int pipe_fd = open(FIFO_ADDRESS, O_RDWR);
    
    for(int i = 1 ; i <= file_count ; i++)
    {
        pid_t id = fork();
        if(id == 0)
        {
            char buffer[2], buffer2[3];
            sprintf(buffer, "%d", fd[0]);
            sprintf(buffer2, "%d", pipe_fd);
            char addr[] = "./map.out";
            char *args[]={&addr[0], buffer, buffer2, NULL};
		    execvp(args[0], args);
            close(fd[1]);
        }
        else
        {
            addr[POSITION] = i + '0';
            if(write(fd[1], &addr, sizeof(addr)) == -1) 
                printf("Error in writing");
        }   
        close(id); 
    }
    pid_t child_pid;
    int s = 0;
    while ((child_pid = wait(&s)) > 0);

    pid_t id2 = fork();
    if(id2 == 0)
    {
        char buffer[2], buffer2[3];
        sprintf(buffer, "%d", fd[1]);
        sprintf(buffer2, "%d", pipe_fd);
        char addr[] = "./reduce.out";
        char *args[]={&addr[0], buffer2, buffer, NULL};
	    execvp(args[0], args);
    }
    else
    {
        pid_t child_pid;
        int s = 0;
        while ((child_pid = wait(&s)) > 0);
    }   

    char text_[2000];
    if(read(fd[0], text_, 2000) == -1) 
        printf("Error in reading");

    int output_fd = open("output.csv", O_RDWR );
    if(write(output_fd, &text_, strlen(text_)) == -1)
        printf("Error in writing \n");

    close(fd[0]);
    close(fd[1]);
    close(pipe_fd);
    return 0;
}