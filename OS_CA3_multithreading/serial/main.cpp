#include <iostream>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <bits/stdc++.h>

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

void getPixlesFromBMP24(int end, int rows, int cols, char *fileReadBuffer)
{
  int count = 1;
  int extra = cols % 4;
  for (int i = 0 ; i < rows ; i++)
  {
    count += extra;
    for (int j = cols - 1 ; j >= 0 ; j--)
      for (int k = 0 ; k < 3; k++)
      {
        pixel[i][j][k] = fileReadBuffer[end - count]; 
        count += 1;
      }
  }
}

void writeOutBmp24(char *fileBuffer, const char *nameOfFileToCreate, int bufferSize)
{
  std::ofstream write(nameOfFileToCreate);
  if (!write)
  {
    cout << "Failed to write " << nameOfFileToCreate << endl;
    return;
  }
  int count = 1;
  int extra = cols % 4;
  for (int i = 0; i < rows; i++)
  {
    count += extra;
    for (int j = cols - 1; j >= 0; j--)
      for (int k = 0; k < 3; k++)
      {
        fileBuffer[bufferSize - count] = pixel[i][j][k];
        count += 1;
      }
  }
  write.write(fileBuffer, bufferSize);
}

bool check(int i, int j)
{
  if(i-1 > 0 && j - 1 > 0 && i+1 < rows && j + 1 < cols)
    return true;
  else 
    return false;
}

void smoothFilter()
{
  for (int i = 0; i < rows; i++)
    for (int j = 0; j < cols; j++)
      for (int k = 0; k < 3; k++)
          if(check(i, j))
            filterPixel[i][j][k] = (pixel[i-1][j][k] + pixel[i][j-1][k] + pixel[i-1][j-1][k] + 
                                    pixel[i+1][j][k] + pixel[i][j+1][k] + pixel[i+1][j+1][k] + 
                                    pixel[i-1][j+1][k] + pixel[i+1][j-1][k] + pixel[i][j][k])/9;
  pixel = filterPixel;
}

void sepiaFilter()
{
  for (int i = 0; i < rows; i++)
    for (int j = 0; j < cols; j++)
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
  pixel = filterPixel;
}

void washedoutFilter()
{
  vector<int> mean(3);
  for(int k = 0 ; k < 3 ; k++)
  {
    for (int i = 0; i < rows; i++)
      for (int j = cols - 1; j >= 0; j--)
        mean[k] += pixel[i][j][k];
    mean[k] /= rows*cols;
  }
  for (int i = 0; i < rows; i++)
    for (int j = cols - 1; j >= 0; j--)
      for(int k = 0 ; k < 3 ; k++)
        filterPixel[i][j][k] = 0.4*pixel[i][j][k] + 0.6*mean[k];
  pixel = filterPixel;
}

void xFilter()
{
  for (int i = 0; i < rows; i++)
    for (int j = cols - 1; j >= 0; j--)
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
}

int main(int argc, char *argv[])
{
  char *fileBuffer;
  int bufferSize;
  char *fileName = argv[1];
  if (!fillAndAllocate(fileBuffer, fileName, rows, cols, bufferSize))
  {
    cout << "File read error" << endl;
    return 1;
  }  

  auto m1 = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
  getPixlesFromBMP24(bufferSize, rows, cols, fileBuffer);
  smoothFilter();
  sepiaFilter();
  washedoutFilter();
  xFilter();
  writeOutBmp24(fileBuffer,"output.bmp", bufferSize);
  auto m2 = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

  cout << m2 - m1 << endl;
  return 0;
}