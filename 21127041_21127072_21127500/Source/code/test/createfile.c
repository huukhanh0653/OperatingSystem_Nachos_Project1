#include "syscall.h"
#include "Utilities.h"

int main()
{
    char fileName[80];
    int length=50;

    Write("----------CREATE FILE----------\n",100,1);

    Write("\n(use ctrl+D to end typing)\n",50,1);
    Write("Enter file's name: ",50,1);
    Read(fileName, length,0); // read file name from console

    if (fileName[0]=='\0'){ // default method of createfile: create a file name "newfile.txt"
        if (Create("newfile.txt") == 0) {
            Write("\nFile ",10,1);
            Write("newfile.txt",20,1);
            Write("Created successfully!\n\n",50,1);
        } else
            Write("Create file failed\n\n",30,1);
    }

    else if (Create(fileName) == 0) { // console method of createfile: create file with the name that you just enter from keyboard
        Write("\nFile ",10,1);
        Write(fileName,length,1);
        Write("Created successfully!\n\n",50,1);
    } else
        Write("Create file failed\n\n",30,1);

    Halt();
}