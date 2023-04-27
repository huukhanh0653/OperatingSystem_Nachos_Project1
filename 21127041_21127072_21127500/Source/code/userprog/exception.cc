// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"

#define MaxFileNameLength 32
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions
//	is in machine.h.
//----------------------------------------------------------------------

//! Utilities

void IncPC()
{
	// Similar to: register[PrevPCReg] = register[PCReg].
	kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	// Similar to: register[PCReg] = register[NextPCReg].
	kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(NextPCReg));

	// Similar to: register[NextPCReg] = PCReg right after.
	kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(NextPCReg) + 4);
}

char *User2System(int virtAddr, int limit)
{
	int i; // index
	int oneChar;
	char *kernelBuf = NULL;

	kernelBuf = new char[limit + 1]; // need for terminal string
	if (kernelBuf == NULL)
		return kernelBuf;
	memset(kernelBuf, 0, limit + 1);

	// printf("\n Filename u2s:");
	for (i = 0; i < limit; i++)
	{
		MACHINE->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;
		// printf("%c",kernelBuf[i]);
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}

// Copy memory zone from System space to User space

int System2User(int virtAddr, int len, char *buffer)
{
	if (len < 0)
		return -1;
	if (len == 0)
		return len;
	int i = 0;
	int oneChar = 0;
	do
	{
		oneChar = (int)buffer[i];
		MACHINE->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < len && oneChar != 0);
	return i;
}

//! Syscall Handlers

void HandleHalt()
{
	DEBUG(dbgSys, "\n Shutdown, initiated by user program.\n");
	SysHalt();
	ASSERTNOTREACHED();
}

// Handle syscall add
void HandleAdd()
{
	DEBUG(dbgSys, "\n Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");

	/* Process SysAdd System call*/
	int result;
	result = SysAdd(/*op1*/ (int)kernel->machine->ReadRegister(4), /*op2*/ (int)kernel->machine->ReadRegister(5));
	DEBUG(dbgSys, "\n Add returning with " << result << "\n");

	/* Prepare Result */
	kernel->machine->WriteRegister(2, (int)result);
	cout << "Add: " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << " = " << result << "\n";

	return IncPC();
}

// Handle syscall create file
void HandleCreate()
{
	int virtAddr;
	char *filename;
	DEBUG('a', "\n SC_CreateFile call ...");
	DEBUG('a', "\n Reading virtual address of filename");

	virtAddr = kernel->machine->ReadRegister(4);
	DEBUG('a', "\n Reading filename.");

	// Sao chep khong gian bo nho User sang System, voi do dang toi da la (32 + 1) bytes
	filename = User2System(virtAddr, MaxFileNameLength + 1);

	if (SysCreate(filename))
		kernel->machine->WriteRegister(2, 0);
	else
		kernel->machine->WriteRegister(2, -1);

	delete filename;
	return IncPC();
}

// Handle syscall delete file
void HandleDelete()
{
	int virtAddr = kernel->machine->ReadRegister(4);		   // Lay dia chi cua tham so name tu thanh ghi so 4
	char *filename = User2System(virtAddr, MaxFileNameLength); // Copy chuoi tu vung nho User Space sang System Space voi bo dem name dai MaxFileLength

	kernel->machine->WriteRegister(2, SysRemove(filename));

	return IncPC();
}

// Handle syscall open file
void HandleOpen()
{
	int virtAddr = kernel->machine->ReadRegister(4);		   // Lay dia chi cua tham so name tu thanh ghi so 4
	int type = kernel->machine->ReadRegister(5);			   // Lay tham so type tu thanh ghi so 5
	char *filename = User2System(virtAddr, MaxFileNameLength); // Copy chuoi tu vung nho User Space sang System Space voi bo dem name dai MaxFileLength

	kernel->machine->WriteRegister(2, SysOpen(filename, type));

	delete[] filename;
	return IncPC();
}

// Handle syscall close file
void HandleClose()
{
	// Input id cua file(OpenFileID)
	//  Output: 0: thanh cong, -1 that bai
	int id = kernel->machine->ReadRegister(4); // Lay tham so id tu thanh ghi so 4

	kernel->machine->WriteRegister(2, SysClose(id));

	return IncPC();
}
void HandleRead()
{
	int virtAddr = kernel->machine->ReadRegister(4);  // Lay dia chi cua tham so buffer tu thanh ghi so 4
	int charCount = kernel->machine->ReadRegister(5); // Lay tham so charCount tu thanh ghi so 5
	char *buffer = User2System(virtAddr, charCount);  // Copy chuoi tu vung nho User Space sang System Space voi bo dem buffer dai charCount
	int fileId = kernel->machine->ReadRegister(6);	  // Lay tham so id tu thanh ghi so 6

	DEBUG(dbgFile, "Read " << charCount << " chars from file " << fileId << "\n");

	kernel->machine->WriteRegister(2, SysRead(buffer, charCount, fileId));

	System2User(virtAddr, charCount, buffer); // Copy chuoi tu vung nho System Space sang User Space voi bo dem buffer dai charCount

	delete[] buffer;
	return IncPC();
}

void HandleWrite()
{
	int virtAddr = kernel->machine->ReadRegister(4);  // Lay dia chi cua tham so buffer tu thanh ghi so 4
	int charCount = kernel->machine->ReadRegister(5); // Lay tham so charCount tu thanh ghi so 5
	char *buffer = User2System(virtAddr, charCount);  // Copy chuoi tu vung nho User Space sang System Space voi bo dem buffer dai charCount
	int fileId = kernel->machine->ReadRegister(6);	  // Lay tham so id tu thanh ghi so 6

	kernel->machine->WriteRegister(2, SysWrite(buffer, charCount, fileId));

	System2User(virtAddr, charCount, buffer); // Copy chuoi tu vung nho System Space sang User Space voi bo dem buffer dai charCount

	delete[] buffer;
	return IncPC();
}

void HandleSeek()
{
	int seekPos = kernel->machine->ReadRegister(4); // Lay tham so pos tu thanh ghi so 4
	int fileId = kernel->machine->ReadRegister(5);	// Lay tham so id tu thanh ghi so 5

	kernel->machine->WriteRegister(2, SysSeek(seekPos, fileId));
	return IncPC();
}

void HandleConnect()
{
	int arg1 = kernel->machine->ReadRegister(4); // socketid
	int arg2 = kernel->machine->ReadRegister(5); // ip
	int arg3 = kernel->machine->ReadRegister(6); // port

	// convert the IP address from user space to kernel space
	char *kernelIP = new char[MAX_STRING_SIZE];
	copyStringFromMachine(arg2, kernelIP, MAX_STRING_SIZE);

	// call the connect function from network utility library
	int result = SysConnect(arg1, kernelIP, arg3);
	// set the return value
	kernel->machine->WriteRegister(2, result);

	delete[] kernelIP;
	return IncPC();
}

void HandleSocketTCP()
{
	kernel->machine->WriteRegister(2, SysSocketTCP());
	return IncPC();
}

// Send the data to the server and set the timeout of 20 seconds
//! For advanced part, we don't use this function
int SocketSend(char *buffer, int charCount, int fileId)
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
//! For advanced part, we don't use this function
int SocketReceive(char *buffer, int charCount, int fileId)
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
	shortRetval = recv(fileId, buffer, charCount, 0);
	cout << "Response: " << buffer << endl;
	IncPC();
	return shortRetval;
}

//----------------------------------------------------------------------
void ExceptionHandler(ExceptionType which)
{
	int type = kernel->machine->ReadRegister(2);

	DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

	switch (which)
	{
	case SyscallException:
		switch (type)
		{
		case SC_Halt:
		{
			DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

			SysHalt();
			ASSERTNOTREACHED();
			break;
		}
		case SC_Add:
		{
			DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");

			/* Process SysAdd Systemcall*/
			int result;
			result = SysAdd(/* int op1 */ (int)kernel->machine->ReadRegister(4),
							/* int op2 */ (int)kernel->machine->ReadRegister(5));

			DEBUG(dbgSys, "Add returning with " << result << "\n");

			/* Prepare Result */
			kernel->machine->WriteRegister(2, (int)result);

			IncPC();
			ASSERTNOTREACHED();

			break;
		}

		case SC_Create:
		{
			HandleCreate();
			break;
		}
		case SC_Remove:
		{
			HandleDelete();
			break;
		}

		case SC_Open:
		{
			HandleOpen();
			break;
		}

		case SC_Read:
		{
			HandleRead();
			break;
		}
		case SC_Write:
		{
			HandleWrite();
			break;
		}

		case SC_Seek:
		{
			HandleSeek();
			break;
		}

		case SC_Close:
		{
			HandleClose();
			break;
		}

		case SC_Connect:
		{
			HandleConnect();
			break;
		}

		case SC_SocketTCP:
		{
			HandleSocketTCP();
			break;
		}

		case SC_Send:
		{
			//! For advanced part, we don't use this case
			int idbuffer = kernel->machine->ReadRegister(4);
			int len = kernel->machine->ReadRegister(5);
			int addrSocketid = kernel->machine->ReadRegister(6);

			int socketid = SysGetIdSocket(addrSocketid);
			char *buffer = new char[len];
			buffer = User2System(idbuffer, len);
			SocketSend(buffer, len, socketid);
			IncPC();
			break;
		}

		case SC_Receive:
		{
			//! For advanced part, we don't use this case
			int idbuffer = kernel->machine->ReadRegister(4);
			int len = kernel->machine->ReadRegister(5);
			int addrSocketid = kernel->machine->ReadRegister(6);
			cout << "Address Socket Id: " << addrSocketid << endl;
			int socketid = SysGetIdSocket(addrSocketid);
			cout << "Socket Id: " << socketid << endl;
			char *buffer = new char[len];
			// buffer = User2System(idbuffer, 100);
			SocketReceive(buffer, len, socketid);
			System2User(idbuffer, len, buffer);
			IncPC();
			break;
		}

		default:
			cerr << "Unexpected system call " << type << "\n";
			break;
		}
		break;
	default:
		cerr << "Unexpected user mode exception" << (int)which << "\n";
		break;
	}
}
