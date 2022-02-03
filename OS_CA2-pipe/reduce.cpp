#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <string.h>
#include <iostream>
#include <sstream>
#include <vector>

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

int main(int argc, char*argv[])
{
    char text_[2000];
    string text;
    int address = convert_to_int(argv[1]);
    if(read(address, text_, 2000) == -1) 
        printf("Error in reading\n");

    text = text_;

    std::stringstream stream(text); 
    string word; 
  
    int num;
    vector<string> words;
    vector<int> nums;
    while(stream >> word)
    {
        int seen = 0;
        stream >> num;
        for(int i = 0 ; i < words.size() ; i++)
        {
            if(word == words[i])
            {
                nums[i] += num;
                seen = 1;
            }
        }
        if(!seen)
        {
            words.push_back(word);
            nums.push_back(num);
        }
    }

    string final = "";
    for(int k = 0 ; k < words.size() ; k++)
        final += words[k] + ": " + to_string(nums[k]) + "\n";
    
    char key[1024]; 
    strcpy(key, final.c_str());

    int named_pipe_fd = convert_to_int(argv[2]);
    write(named_pipe_fd, key, strlen(key));

    close(address);
}