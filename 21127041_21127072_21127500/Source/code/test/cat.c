#include "syscall.h"
#include "Utilities.h"

#define CONSOLE_INPUT 0
#define CONSOLE_OUTPUT 1

int main() {
    int id;
    char buffer[256];

    id = Open("Text1.txt", 0);
    Read(buffer,256, id);
    Write(buffer,len(buffer),CONSOLE_OUTPUT);
    Close(id);
    Halt();
}