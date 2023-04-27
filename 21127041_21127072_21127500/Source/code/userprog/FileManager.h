#include "main.h"
#include "synchconsole.h"
#include "machine.h"
#include "post.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MACHINE kernel->machine
#define FILESYSTEM kernel->fileSystem
#define INTERRUPT kernel->interrupt
#define INPUT kernel->synchConsoleIn
#define OUTPUT kernel->synchConsoleOut

typedef int OpenFileId;

// Define for file type
#define STANDARD_MODE 0
#define READONLY_MODE 1
#define SOCKET_MODE 2

#define MAX_STRING_SIZE 256
#define PORT 8081

/*
    Class File duoc dung de quan li cac file dang duoc mo va cac socket
    File se luu FilePointer, file descriptor, type(doc, doc-ghi, socket)
    File ID la 1 so nguyen duong
    duoc tich hop boi gia tri type va file descriptor voi cong thuc:
    ! ID = type*100 + FilePointer->ID()
    ! Cac gia tri type duoc dinh nghia o cac dong: 21,22,23 cua file nay

    => Voi cong thuc nay, ham se tra ve 1 so co 3 chu so voi chu so hang tram la type file
    va 2 chu so con lai la file id duoc tao ra boi Nachos
    !ID se giup phan biet giua file thong thuong va socket-> Giup hoan thanh phan advanced
*/

//! Ultility

int Min(int a, int b)
{
    return a < b ? a : b;
}

int Max(int a, int b)
{
    return a > b ? a : b;
}

//!-----------------------

struct Socket
{
    int socketID;
    ~Socket()
    {
        if (socketID > -1)
            CloseSocket(socketID);
        socketID = -1;
    }
};

struct NormalFile
{
    char *name;
    OpenFile *FilePointer;
    int currentOffset;
    ~NormalFile()
    {
        currentOffset = -1;
    }
};

class File
{
private:
    Socket _Socket;
    NormalFile _File;
    OpenFileId id;
    int type;

public:
    File()
    {
        this->id = -1;
        this->type = -1;
        this->_File.currentOffset = -1;
        this->_File.FilePointer = NULL;
    }

    ~File()
    {
        this->id = -1;
        this->type = -1;
    }

    int Type() { return this->type; }
    OpenFileId ID() { return this->id; } //!   MODE*100+FD = ID
    int getSocketID() { return this->_Socket.socketID; }
    char *Name() { return this->_File.name; }
    int Offset() { return this->_File.currentOffset; }

    int openFile(char *name, int type)
    {
        this->_File.FilePointer = FILESYSTEM->Open(name);
        int fid = this->_File.FilePointer->GetFileID();
        if (this->_File.FilePointer != NULL)
        {
            this->_File.name = name;
            this->type = type;
            this->id = (type * 100) + fid;
            this->_File.currentOffset = 0; // Filesystem Object sets Offset = 0 as default
            return id;
        }
        else
            return -1;
    }

    int openSocket()
    {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
        {
            perror("Error creating socket");
            return -1;
        }
        this->_Socket.socketID = sockfd;
        this->type = SOCKET_MODE;
        this->id = SOCKET_MODE * 100 + this->_Socket.socketID;
        return this->_Socket.socketID;
    }

    int connectSocket(char *ip, int port)
    {
        // Create a new sockaddr_in struct to represent the remote host
        struct sockaddr_in remote_addr;
        remote_addr.sin_family = AF_INET;            // Use IPv4 protocol
        remote_addr.sin_port = htons(port);          // Convert port to network byte order
        remote_addr.sin_addr.s_addr = inet_addr(ip); // Convert IP address to network byte order

        // Try to connect to the remote host using the existing socket ID
        if (connect(this->_Socket.socketID, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0)
        {
            perror("Error connecting to remote host");
            CloseSocket(this->_Socket.socketID); // Close socket on failure
            return -1;                           // Return -1 on failure
        }

        cerr << "Connected successfully\n";
        return this->_Socket.socketID; // Return socket file descriptor on success
    }

    int ReadFile(char *buffer, int charCount)
    {
        int result = -1;
        if (this->type == SOCKET_MODE)
        {
            result = _SocketReceive(buffer, charCount, this->_Socket.socketID);
            return result;
        }
        else
        {
            if (!this->_File.FilePointer)
            {
                cout << "File doesn't exist!\n";
                return -1;
            }
            result = _File.FilePointer->Read(buffer, charCount);
            if (result < charCount)
                return -2;
        }
        return result;
    }

    int WriteFile(char *buffer, int charCount)
    {
        int result;
        if (this->type == SOCKET_MODE)
        {
            result = _SocketSend(buffer, charCount, this->_Socket.socketID);
            return result;
        }
        else
        {
            if (!this->_File.FilePointer)
            {
                cout << "File doesn't exist!\n";
                return -1;
            }
            if (this->type == READONLY_MODE)
            {
                cout << "INVALID MODE!\n";
                return -1;
            }
            result = this->_File.FilePointer->WriteAt(buffer, charCount, _File.FilePointer->Length() + 1);
        }
        if (result != charCount)
            return -2;
        cout << "Written success!\n";
        return result;
    }

    int seekFile(int position)
    {
        if (this->type == SOCKET_MODE)
            return -1;
        if (position >= _File.FilePointer->Length())
            return -1;
        if (position == -1)
            return _File.FilePointer->Seek(_File.FilePointer->Length());
        return _File.FilePointer->Seek(position);
    }
    int CloseFile()
    {
        if (this->type == -1)
            return -1;
        if (this->type == SOCKET_MODE)
            this->_Socket.~Socket();
        else if (this->_File.FilePointer)
        {
            _File.FilePointer->~OpenFile();
            _File.FilePointer = NULL;
            this->type = -1;
            this->id = -1;
        }
    }

protected:
    int _SocketSend(char *buffer, int charCount, int fileId)
    {
        int shortRetval = -1;
        struct timeval tv;
        tv.tv_sec = 20; /* 20 Secs Timeout */
        tv.tv_usec = 0;
        if (setsockopt(fileId, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv)) < 0)
        {
            printf("Time Out\n");
            return -1;
        }
        shortRetval = send(fileId, buffer, charCount, 0);
        return shortRetval;
    }

    // receive the data from the server
    int _SocketReceive(char *buffer, int charCount, int fileId)
    {
        int shortRetval = -1;
        //* Set thoi gian cho la 20 mili-seconds
        struct timeval tv;
        tv.tv_sec = 20; /* 20 miliSecs Timeout */
        tv.tv_usec = 0;
        //* Ham check time out theo thoi gian tv_sec neu qua 20 milisecond nhung khong nhan duoc phan hoi
        //* thi in ra "Time Out"
        if (setsockopt(fileId, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv)) < 0)
        {
            printf("Time Out\n");
            return -1;
        }
        shortRetval = recv(fileId, buffer, charCount, 0);
        cout << "Response: " << buffer << endl;
        return shortRetval;
    }
};

class Table
{
private:
    const int MaxSize = 20;
    int currentSize;
    File *table;

public:
    Table()
    {
        this->table = new File[20];
        currentSize = 0;
    }
    ~Table()
    {
        delete[] table;
    }

    OpenFileId isOpened(OpenFileId id)
    {
        for (int i = 0; i < MaxSize; i++)
        {
            if (table[i].ID() == id)
                return table[i].ID();
        }
        return -1;
    }

    OpenFileId isOpened(char *name, int type)
    {
        for (int i = 0; i < MaxSize; i++)
            if (table[i].Name())
            {
                if (strcasecmp(name, table[i].Name()) == 0 && table[i].Type() == type)
                    return table[i].ID();
            }
        return -1;
    }

    int closeFile(OpenFileId id)
    {
        int i = isOpened(id); // Check if it's opened
        if (i < 0)
            return -1;
        if (table[i].Type() != SOCKET_MODE)
        {
            table[i].CloseFile();
            currentSize--;
        }
        else
        {
            close(table[i].getSocketID());
        }
        return 0;
    }

    OpenFileId open(char *name, int type)
    {
        if (currentSize == MaxSize)
            return -1; // Mang day
        for (int i = 0; i < MaxSize; i++)
        {
            if (table[i].ID() == -1)
            {
                if (type != SOCKET_MODE)
                    this->table[i].openFile(name, type);
                else
                    this->table[i].openSocket();

                if (this->table[i].ID() != -1)
                {
                    currentSize++;
                    return this->table[i].ID();
                }
                else
                    return -1;
            }
        }
        return -1; // Full
    }

    bool isEmpty() { return currentSize == 0; }

    bool isFull() { return currentSize == MaxSize; }

    File *getFile(OpenFileId id)
    {
        for (int i = 0; i < MaxSize; i++)
        {
            if (table[i].ID() == id)
                return &table[i];
        }
        return NULL;
    }
};
