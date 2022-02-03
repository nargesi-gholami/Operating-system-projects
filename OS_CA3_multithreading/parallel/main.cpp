#include <iostream>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <bits/stdc++.h>

#include <pthread.h>
#include <stdlib.h>
#include <cstdint>

#include <chrono>
#include <sys/time.h>
#include <ctime>
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

using namespace std;

using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;

#pragma pack(1)
//#pragma once

typedef int LONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;

typedef struct tagBITMAPFILEHEADER
{
  WORD bfType;
  DWORD bfSize;
  WORD bfReserved1;
  WORD bfReserved2;
  DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
  DWORD biSize;
  LONG biWidth;
  LONG biHeight;
  WORD biPlanes;
  WORD biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG biXPelsPerMeter;
  LONG biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

#define size 512
vector<vector<vector<unsigned char>>> pixel(size, vector<vector<unsigned char>>(size, vector<unsigned char>(3)));
vector<vector<vector<unsigned char>>> filterPixel(size, vector<vector<unsigned char>>(size, vector<unsigned char>(3)));

int rows;
int cols;
char *fileBuffer;
int bufferSize;
const char *nameOfFileToCreate;

vector<int> mean(3);

bool fillAndAllocate(char *&buffer, const char *fileName, int &rows, int &cols, int &bufferSize)
{
  std::ifstream file(fileName);

  if (file)
  {
    file.seekg(0, std::ios::end);
    std::streampos length = file.tellg();
    file.seekg(0, std::ios::beg);

    buffer = new char[length];
    file.read(&buffer[0], length);

    PBITMAPFILEHEADER file_header;
    PBITMAPINFOHEADER info_header;

    file_header = (PBITMAPFILEHEADER)(&buffer[0]);
    info_header = (PBITMAPINFOHEADER)(&buffer[0] + sizeof(BITMAPFILEHEADER));
    rows = info_header->biHeight;
    cols = info_header->biWidth;
    bufferSize = file_header->bfSize;
    return 1;
  }
  else
  {
    cout << "File" << fileName << " doesn't exist!" << endl;
    return 0;
  }
}

void* getPixlesFromBMP24(void* input)
{
  intptr_t n = (intptr_t)input;
  int startRow, endRow;

  if(n == 0)
    startRow = 0;
  else if(n == 1)
    startRow = int(size / 4);
  else if(n == 2)
    startRow = int(size / 2);
  else if (n == 3)
    startRow = int(3 * size / 4);
  
  endRow =  startRow + int(size/4);
  int extra = cols % 4;

  int count = 1;
  int end = bufferSize- int(3*size*size*n / 4);
  for (int i = startRow; i < endRow; i++)
  {
    count += extra;
    for (int j = cols-1; j >= 0; j--)
      for (int k = 0; k < 3; k++)
      {
        pixel[i][j][k] = fileBuffer[end - count]; 
        count += 1;
      }
  }
  pthread_exit(0);
}

void* writeOutBmp24(void* input)
{
  std::ofstream write(nameOfFileToCreate);
  if (!write)
  {
    cout << "Failed to write " << nameOfFileToCreate << endl;
    pthread_exit(0);
  }

  intptr_t n = (intptr_t)input;
  int startRow, endRow;

  if(n == 0)
    startRow = 0;
  else if(n == 1)
    startRow = int(size / 4);
  else if(n == 2)
    startRow = int(size / 2);
  else if (n == 3)
    startRow = int(3 * size / 4);

  endRow =  startRow + int(size/4);

  int count = 1;
  int extra = cols % 4;
  int end = bufferSize- int(3*size*size*n / 4);
  for (int i = startRow; i < endRow; i++)
  {
    count += extra;
    for (int j = cols - 1; j >= 0; j--)
      for (int k = 0; k < 3; k++)
      {
        fileBuffer[end - count] = pixel[i][j][k];
        count += 1;
      }
  }
  write.write(fileBuffer, bufferSize);
  pthread_exit(0);
}

bool check(int i, int j)
{
  if(i-1 > 0 && j - 1 > 0 && i+1 < rows && j + 1 < cols)
    return true;
  else 
    return false;
}
/////////////////////////////////////
void* smoothFilter(void* input)
{
  intptr_t k = (intptr_t)input;
  int startCol, endCol;
  int startRow, endRow;

  startCol = k < 2 ? 0 : int(size / 2);
  endCol = k < 2 ? int(size / 2) : size;
  startRow = k % 2 ? int(size / 2) : 0;
  endRow =  k % 2 ? size : int(size / 2);

  for (int i = startRow; i < endRow; i++)
    for (int j = startCol; j < endCol; j++)
      for (int k = 0; k < 3; k++)
        if(check(i, j))
          filterPixel[i][j][k] = (pixel[i-1][j][k] + pixel[i][j-1][k] + pixel[i-1][j-1][k] + 
                                    pixel[i+1][j][k] + pixel[i][j+1][k] + pixel[i+1][j+1][k] + 
                                    pixel[i-1][j+1][k] + pixel[i+1][j-1][k] + pixel[i][j][k])/9;

  for (int i = startRow; i < endRow; i++)
    for (int j = startCol; j < endCol; j++)
      for (int k = 0; k < 3; k++)
        pixel[i][j][k] = filterPixel[i][j][k];
  pthread_exit(0);
}

////////////////////////////////////

void* sepiaFilter(void* input)
{
  intptr_t k = (intptr_t)input;
  int startCol, endCol;
  int startRow, endRow;
  startCol = k < 2 ? 0 : int(size / 2);
  endCol = k < 2 ? int(size / 2) : size;
  startRow = k % 2 ? int(size / 2) : 0;
  endRow =  k % 2 ? size : int(size / 2);

  for (int i = startRow; i < endRow; i++)
    for (int j = startCol; j < endCol; j++)
    {
      if(0.393*pixel[i][j][0] + 0.769*pixel[i][j][1] + 0.189*pixel[i][j][2] < 255)
        filterPixel[i][j][0] = 0.393*pixel[i][j][0] + 0.769*pixel[i][j][1] + 0.189*pixel[i][j][2];
      else
        filterPixel[i][j][0] =  255;
      if(0.349*pixel[i][j][0] + 0.686*pixel[i][j][1] + 0.168*pixel[i][j][2] < 255)
        filterPixel[i][j][1] = 0.349*pixel[i][j][0] + 0.686*pixel[i][j][1] + 0.168*pixel[i][j][2];
      else
        filterPixel[i][j][1] = 255;
      if(0.272*pixel[i][j][0] + 0.534*pixel[i][j][1] + 0.131*pixel[i][j][2] < 255)
        filterPixel[i][j][2] = 0.272*pixel[i][j][0] + 0.534*pixel[i][j][1] + 0.131*pixel[i][j][2]  ;
      else        
        filterPixel[i][j][2] = 255;
    }
  for (int i = startRow; i < endRow; i++)
    for (int j = startCol; j < endCol; j++)
      for (int k = 0; k < 3; k++)
        pixel[i][j][k] = filterPixel[i][j][k];
  pthread_exit(0);
}

void calMean()
{
  for(int k = 0 ; k < 3 ; k++)
  {
    for (int i = 0; i < rows; i++)
      for (int j = cols - 1; j >= 0; j--)
        mean[k] += pixel[i][j][k];
    mean[k] /= rows*cols;
  }
}

void* washedoutFilter(void* input)
{
  intptr_t k = (intptr_t)input;
  int startCol, endCol;
  int startRow, endRow;
  startCol = k < 2 ? 0 : int(size / 2);
  endCol = k < 2 ? int(size / 2) : size;
  startRow = k % 2 ? int(size / 2) : 0;
  endRow =  k % 2 ? size : int(size / 2);

  for (int i = startRow; i < endRow; i++)
    for (int j = startCol; j < endCol; j++)
      for(int k = 0 ; k < 3 ; k++)
        filterPixel[i][j][k] = 0.4*pixel[i][j][k] + 0.6*mean[k];

  for (int i = startRow; i < endRow; i++)
    for (int j = startCol; j < endCol; j++)
      for (int k = 0; k < 3; k++)
        pixel[i][j][k] = filterPixel[i][j][k];
  pthread_exit(0);
}

void* xFilter(void* input)
{
  intptr_t k = (intptr_t)input;
  int startCol, endCol;
  int startRow, endRow;

  startCol = k < 2 ? 0 : int(size / 2);
  endCol = k < 2 ? int(size / 2) : size;
  startRow = k % 2 ? int(size / 2) : 0;
  endRow =  k % 2 ? size : int(size / 2);

  for (int i = startRow; i < endRow; i++)
    for (int j = startCol; j < endCol; j++)
      for(int k = 0 ; k < 3 ; k++)
      {
        if(i == j && check(i, j))
        {
          pixel[i][j][k] = 255;
          pixel[i+1][j][k] = 255;
          pixel[i][j+1][k] = 255;
          pixel[rows-i][j][k] = 255;
          pixel[rows-i-1][j][k] = 255;
          pixel[rows-i][j+1][k] = 255;
        }
      }
  pthread_exit(0);
}

int main(int argc, char *argv[])
{
    char *fileName = argv[1];
    if (!fillAndAllocate(fileBuffer, fileName, rows, cols, bufferSize))
    {
        cout << "File read error" << endl;
        return 1;
    }  
    auto m1 = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    //read image
    pthread_t readThread[4];
    for (int i = 0 ; i < 4 ; i++)
    {
      int threadnumber = (intptr_t) i;
      pthread_create( &readThread[i], NULL, getPixlesFromBMP24, (void*)(intptr_t)threadnumber);
    }
    for (int i = 0 ; i < 4 ; i++)
      pthread_join( readThread[i], NULL);

    //smoothing
    pthread_t smoothThread[4];
    for (int i = 0 ; i < 4 ; i++)
    {
      int threadnumber = (intptr_t) i;
      pthread_create( &smoothThread[i], NULL, smoothFilter, (void*)(intptr_t)threadnumber);
    }
    for (int i = 0 ; i < 4 ; i++)
      pthread_join( smoothThread[i], NULL);

  //sepia
    pthread_t sepiaThread[4];
    for (int i = 0 ; i < 4 ; i++)
    {
      int threadnumber = (intptr_t) i;
      pthread_create( &sepiaThread[i], NULL, sepiaFilter, (void*)(intptr_t)threadnumber);
    }
    for (int i = 0 ; i < 4 ; i++)
      pthread_join( sepiaThread[i], NULL);
  //washed out 
    calMean();
    pthread_t washThread[4];
    for (int i = 0 ; i < 4 ; i++)
    {
      int threadnumber = (intptr_t) i;
      pthread_create( &washThread[i], NULL, washedoutFilter, (void*)(intptr_t)threadnumber);
    }
    for (int i = 0 ; i < 4 ; i++)
      pthread_join( washThread[i], NULL);

  //x filter
    pthread_t xThread[4];
    for (int i = 0 ; i < 4 ; i++)
    {
      int threadnumber = (intptr_t) i;
      pthread_create( &xThread[i], NULL, xFilter, (void*)(intptr_t)threadnumber);
    }
    for (int i = 0 ; i < 4 ; i++)
      pthread_join( xThread[i], NULL);
  //write
  nameOfFileToCreate = "output.bmp";
  pthread_t writeThread[4];
    for (int i = 0 ; i < 4 ; i++)
    {
      int threadnumber = (intptr_t) i;
      pthread_create( &writeThread[i], NULL, writeOutBmp24, (void*)(intptr_t)threadnumber);
    }
    for (int i = 0 ; i < 4 ; i++)
      pthread_join( writeThread[i], NULL);


    auto m2 = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
  //print time
    cout << m2 - m1 << endl;
    return 0;
}