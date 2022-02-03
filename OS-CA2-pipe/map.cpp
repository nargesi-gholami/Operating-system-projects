#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <vector>
#include <sstream>
#include <fstream>

#define SIZE 18

using namespace std;


int convert_to_int(char* buff) 
{
    int num = 0;
    int z = 1;
    for(int i = strlen(buff) -1 ; i >= 0; i--)
    {
        num += (buff[i]-'0')*z;
        z *= 10;
    }
    return num;
}

vector<string> make_key(char* addr)
{

    vector<vector<string>> content;
	vector<string> row;
	string line, word;

    fstream file(addr, ios::in);
	if(file.is_open())
	{
		while(getline(file, line))
		{
			row.clear();
			stringstream str(line);
			while(getline(str, word, ','))
				row.push_back(word);          
			content.push_back(row);
		}
	}        
    return row;
}

int main(int argc, char*argv[])
{
    char *addr = (char*)malloc(SIZE);
    int address = convert_to_int(argv[1]);

    if(read(address, addr, SIZE) == -1) 
        printf("Error in reading");
    close(address);

    vector<string> row = make_key(addr);
    vector<string> word; 
    vector<int> num;

    for(int i = 0 ; i < row.size() ; i++)
    {
        string w = row[i];
        int seen = 0;
        for(int j = 0 ; j < word.size() ; j++)
        {
            if(word[j] == w)
            {
                num[j] ++;
                seen = 1;
            }
        }
        if(!seen)
        {
            word.push_back(w);
            num.push_back(1);
        }
    }
    

    string final = "";
    
    for(int k = 0 ; k < word.size() ; k++)
        final += word[k] + " " + to_string(num[k]) +" ";
    
    
    char key[1024]; 
    strcpy(key, final.c_str());

    int named_pipe_fd = convert_to_int(argv[2]);
    write(named_pipe_fd, key, strlen(key));

    return 0;
}