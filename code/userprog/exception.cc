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
#include "thread.h"
#include <string>
#include "buffer.h"
#include "message.h"
#include <map>
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

map<int, Thread* > umap;
map<int, Thread*>:: iterator itr;
map<string, Thread*>:: iterator itrThread;
int value;

void ForkTest1(int id)
{
	printf("ForkTest1 is called, its PID is %d\n", id);
	for (int i = 0; i < 3; i++)
	{
		printf("ForkTest1 is in loop %d\n", i);
		for (int j = 0; j < 100; j++)
			kernel->interrupt->OneTick();
	}
	Exit_POS(id);
}

void ForkTest2(int id)
{
	printf("ForkTest2 is called, its PID is %d\n", id);
	for (int i = 0; i < 3; i++)
	{
		printf("ForkTest2 is in loop %d\n", i);
		for (int j = 0; j < 100; j++)
			kernel->interrupt->OneTick();
	}
	Exit_POS(id);
}

void ForkTest3(int id)
{
	printf("ForkTest3 is called, its PID is %d\n", id);
	for (int i = 0; i < 3; i++)
	{
		printf("ForkTest3 is in loop %d\n", i);
		for (int j = 0; j < 100; j++)
			kernel->interrupt->OneTick();
	}
	Exit_POS(id);
}

void
Exit_POS(int id)
{
	IntStatus oldLevel;
	for (itr = umap.begin(); itr != umap.end(); itr++) {
		if ((itr -> first) == id ) {
			oldLevel = kernel -> interrupt->SetLevel(IntOff);

			cout << "WAKING UP: " << itr -> second -> getName() << endl;

			kernel-> scheduler -> ReadyToRun(itr -> second);

			(void) kernel -> interrupt->SetLevel(oldLevel);
		}
	}
}


void
HandleSendMessage(int rec, int msg, int bufferId) {
	string receiver = "";
	string message = "";

	// populating rec
	while (true) {
		bool check = kernel->machine -> ReadMem(rec, 1, &value);
		if (check) {
			if ((char)value != NULL) {
				// cout << (char)value;
				receiver.push_back((char)value);
				rec += 1;
			}
			else {
				break;
			}
		}
		else {
			break;
		}
	}

// populating msg
	while (true) {
		bool check = kernel->machine -> ReadMem(msg, 1, &value);
		if (check) {
			if ((char)value != NULL) {
				// cout << (char)value;
				message.push_back((char)value);
				msg += 1;
			}
			else {
				break;
			}
		}
		else {
			break;
		}
	}
	cout << "********* SendMessage in Thread "
	     << kernel->currentThread->getName() << " ***************" << endl;

	Message *m = new Message;
	string sender(kernel->currentThread->getName());
	m->sender = sender;
	m->receiver = receiver;
	m->msg = message;

	cout << "Sending message from thread: " << m->sender << endl;

	if (kernel->threadMap[receiver] != NULL) {
		// bufferId = kernel->currentThread->buffermap[sender];
		Buffer *buffer = NULL;
		if (!kernel -> threadMap[receiver] -> bufferpool -> IsEmpty()) {
			ListIterator<Buffer*> *bufItr =
			    new ListIterator<Buffer*>(kernel -> threadMap[receiver] -> bufferpool);
			for (bufItr -> Item() ; !bufItr -> IsDone() ; bufItr -> Next()) {
				if (bufItr -> Item() -> bufferId == bufferId) {
					buffer = bufItr->Item();
					break;
				}
			}
		}
		// if (bufferId == -1)
		// 	bufferId = kernel -> bitmap -> FindAndSet();
		// if (bufferId == -1) {

		// }

		if (buffer == NULL) {
			buffer = new Buffer();
			buffer -> setReceiverName(receiver);
			char* sName = kernel -> currentThread -> getName();
			std::string str(sName);
			buffer -> setSenderName(str);
			buffer -> setBufferId(bufferId);
			cout << "Set bufferId " << bufferId << endl;

			kernel -> threadMap[sender] -> bufferpool -> Append(buffer);
			kernel -> threadMap[receiver] -> bufferpool -> Append(buffer);
			kernel -> threadMap[receiver] -> buffermap[str] = bufferId;
		}
		buffer->appendMessage(m);

		if (kernel->threadMap[receiver]->status == BLOCKED) {
			IntStatus oldLevel = kernel -> interrupt->SetLevel(IntOff);
			kernel->scheduler->ReadyToRun(kernel->threadMap[receiver]);
			(void) kernel -> interrupt->SetLevel(oldLevel);
		}
		cout << "********** Successfully sent Message ***********" << endl << endl;

		kernel->machine->WriteRegister(2, bufferId);
	} else {
		cout << "Target process doesn't exist! Can't send message!!!" << endl;
		cout << "Initializing cleanup of buffer..." << bufferId << endl;
		kernel->bitmap->Clear(bufferId);
		kernel->machine->WriteRegister(2, -1);
	}
}

void
PrintMessage(string s) {
	cout << s << ", ";
}

void
HandleWaitMessage(int send, int msg, int bufferId) {
	IntStatus oldLevel;
	Thread *invokingThread = kernel -> currentThread;
	cout << invokingThread->getName() << " waiting for message." << endl;
	// populating sender
	string sender = "";
	while (true) {
		bool check = kernel->machine -> ReadMem(send, 1, &value);
		if (check) {
			if ((char)value != NULL) {
				// cout << (char)value;
				sender.push_back((char)value);
				send += 1;
			}
			else {
				break;
			}
		}
		else {
			break;
		}
	}
	Buffer *buffer = NULL;
	while (true) {
		bufferId = kernel->currentThread->buffermap[sender];
		ListIterator<Buffer*> *it = new ListIterator<Buffer*>(kernel -> currentThread -> bufferpool);
		for (; !it -> IsDone() ; it -> Next()) {
			if (it -> Item() -> bufferId == bufferId) {
				buffer = it->Item();
				break;
			}
		}
		if (buffer != NULL) break;
		else {
			oldLevel = kernel -> interrupt->SetLevel(IntOff);
			cout << kernel->currentThread->getName() << " is sleeping because buffer doesn't exist." << endl;
			invokingThread->setStatus(BLOCKED);
			invokingThread -> Sleep(FALSE);
			(void) kernel -> interrupt->SetLevel(oldLevel);
		}
	}

	while (!buffer->hasMessageForMe(invokingThread->getName())) {
		cout << "doesn't have message for me " << invokingThread->getName() << endl;
		oldLevel = kernel -> interrupt->SetLevel(IntOff);
		invokingThread->setStatus(BLOCKED);
		invokingThread -> Sleep(FALSE);
		(void) kernel -> interrupt->SetLevel(oldLevel);
		cout << invokingThread->getName() << endl;
	}

	cout << "********** Message received *************" << endl;
	buffer->popMessageForMe(kernel->currentThread->getName())->PrintMessage();
	// invokingThread -> unanswered -> Append(received);
	kernel -> machine -> WriteRegister(2, bufferId);
	cout << "********** End Message Handling ************" << endl;
}

void SendAnswer(int result, int answer, int bufferId) {
	string ans = "";
	while (true) {
		bool check = kernel->machine -> ReadMem(answer, 1, &value);
		if (check) {
			if ((char)value != NULL) {
				// cout << (char)value;
				ans.push_back((char)value);
				answer += 1;
			}
			else {
				break;
			}
		}
		else {
			break;
		}
	}

	cout << "**************** Sending Answer: " << ans << " *****************" << endl;
	ListIterator<Buffer*> *it = new ListIterator<Buffer*>(kernel -> currentThread -> bufferpool);
	Buffer *buffer = NULL;
	for (; !it -> IsDone() ; it -> Next()) {
		if (it -> Item() -> bufferId == bufferId) {
			buffer = it->Item();
			break;
		}
	}
	if (buffer != NULL) {
		//todo create message object and push to buffer
		Message *m = new Message();
		string sender(kernel->currentThread->getName());
		m->sender = sender;
		m->isAnswer = TRUE;
		m->answerType = 1;
		string receiver(buffer->getSenderName() == sender
		                ? buffer->getReceiverName() : buffer->getSenderName());
		cout << "receiver: " << receiver << endl;
		m->receiver = receiver;
		m->msg = ans;
		buffer->appendMessage(m);
		// kernel -> threadMap[buffer -> getSenderName()] -> bufferpool -> Append(buffer);
		// cout << "Sent answer " << buffer->getAnswer() << " to " << buffer->getSenderName() << endl;

		if (kernel->threadMap[m->receiver] != NULL) {
			if (kernel->threadMap[m->receiver]->status == BLOCKED) {
				IntStatus oldLevel = kernel -> interrupt->SetLevel(IntOff);
				kernel->scheduler->ReadyToRun(kernel->threadMap[m->receiver]);
				// kernel -> currentThread -> unanswered -> Remove(buffer);
				(void) kernel -> interrupt->SetLevel(oldLevel);
			}
		} else {
			cout << m->receiver << " has finished. Initializing buffer cleanup..." << endl;
		}
	} else {
		cout << "Buffer not found, not sending answer!" << endl;
	}

	cout << "************ End Send Answer *************" << endl;
}

void SendDummyAnswer(string receiver, string sender, Buffer *buffer) {
	Message *m = new Message();
	m->sender = sender;
	m->isAnswer = TRUE;
	m->answerType = 1;
	m->receiver = receiver;
	m->msg = string("dummy");

	if (kernel->threadMap[m->receiver] != NULL) {
		buffer->appendMessage(m);
		if (kernel->threadMap[m->receiver]->status == BLOCKED) {
			IntStatus oldLevel = kernel -> interrupt->SetLevel(IntOff);
			kernel->scheduler->ReadyToRun(kernel->threadMap[m->receiver]);
			// kernel -> currentThread -> unanswered -> Remove(buffer);
			(void) kernel -> interrupt->SetLevel(oldLevel);
		}
	} else {
		cout << m->receiver << " has finished. No dummy answer..." << endl;
	}
}

void WaitAnswer(int result, int answer, int bufferId) {
	cout << kernel->currentThread->getName() << " waiting for answer. " << bufferId << endl;
	if (bufferId == -1) {
		cout << "bufferId is -1. Did previous message fail?" << endl;
		IntStatus oldLevel = kernel -> interrupt->SetLevel(IntOff);
		cout << kernel->currentThread->getName() << " yielding..." << endl << endl;
		kernel->currentThread->Yield();
		kernel -> interrupt->SetLevel(oldLevel);
	}
	Buffer *buffer = NULL;
	while (true) {
		ListIterator<Buffer*> *bufItr = new ListIterator<Buffer*>(kernel -> currentThread -> bufferpool);
		for (; !bufItr -> IsDone() ; bufItr -> Next()) {
			if (bufItr -> Item() -> bufferId == bufferId) {
				buffer = bufItr->Item();
				break;
			}
		}
		if (buffer != NULL) break;
		else {
			IntStatus oldLevel = kernel -> interrupt->SetLevel(IntOff);
			cout << kernel->currentThread->getName() << " sleeping..." << endl << endl;
			kernel->currentThread->setStatus(BLOCKED);
			kernel->currentThread->Sleep(FALSE);
			kernel -> interrupt->SetLevel(oldLevel);
		}
	}

	cout << "after infinite while waitanswer" << endl;
	while (!buffer->hasAnswerForMe(kernel->currentThread->getName())) {
		IntStatus oldLevel = kernel -> interrupt->SetLevel(IntOff);
		cout << kernel->currentThread->getName() << " sleeping..." << endl;
		kernel->currentThread->setStatus(BLOCKED);
		kernel->currentThread->Sleep(FALSE);
		kernel -> interrupt->SetLevel(oldLevel);
	}
	cout << "********** Answer Received ************" << endl;

	cout << "Received answer: ";
	buffer->popAnswerForMe(kernel->currentThread->getName())->PrintMessage();

	cout << "************ End Wait Answer **************" << endl;
}

void
ExceptionHandler(ExceptionType which)
{
	int type = kernel->machine->ReadRegister(2);

	DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

	switch (which) {
	case SyscallException:
		switch (type) {
		case SC_Halt:
			DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

			SysHalt();

			ASSERTNOTREACHED();
			break;

		case SC_Add:
			DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4)
			      << " + " << kernel->machine->ReadRegister(5) << "\n");

			/* Process SysAdd Systemcall*/
			int result;
			result = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),
			                             /* int op2 */(int)kernel->machine->ReadRegister(5));

			DEBUG(dbgSys, "Add returning with " << result << "\n");
			/* Prepare Result */
			kernel->machine->WriteRegister(2, (int)result);

			/* Modify return point */
			{
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			}

			return;

			ASSERTNOTREACHED();

			break;

		case SC_FORK_POS:
		{
			//  int res = SysFork_POS(1);
			//  cout<<res<<endl;

			// ASSERTNOTREACHED();
			int val = (int)kernel->machine->ReadRegister(4);
			Thread *childThread = new Thread("childThread");

			int ID = childThread -> getID();

			cout << "child ID: " << ID << endl;

			if (val == 1) {
				cout << "Entered value is: " << val << endl;
				childThread -> Fork((VoidFunctionPtr) ForkTest1, (void *) ID);
			}
			if (val == 2) {
				cout << "Entered value is: " << val << endl;
				childThread -> Fork((VoidFunctionPtr) ForkTest2, (void *) ID);
			}
			if (val == 3) {
				cout << "Entered value is: " << val << endl;
				childThread -> Fork((VoidFunctionPtr) ForkTest3, (void *) ID);
			}

			kernel->machine->WriteRegister(2, (int)ID);

			/* Modify return point */
			{
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			}

		}
		return ;
		break;


		case SC_WAIT_POS:
		{
			//SysWait_POS(1);
			//ASSERTNOTREACHED();
			IntStatus oldLevel;

			int childID = (int)kernel->machine->ReadRegister(4);

			umap.insert({childID, kernel -> currentThread});

			oldLevel = kernel -> interrupt->SetLevel(IntOff);

			cout << "SLEEPING: " << kernel -> currentThread -> getName() << endl;

			kernel -> currentThread -> Sleep(FALSE);	// ReadyToRun assumes that interrupts are disabled!

			cout << "RESUMING: " << kernel -> currentThread -> getName() << endl;

			(void) kernel -> interrupt->SetLevel(oldLevel);

			{
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			}
		}
		return ;
		break;

		case SC_Exit:
		{
			cout << "System Exit is called in " << kernel->currentThread->getName() << endl;
			string me(kernel -> currentThread -> getName());
			kernel->threadMap.erase(kernel -> currentThread -> getName());
			ListIterator<Buffer*> *it = new ListIterator<Buffer*>(kernel->currentThread->bufferpool);
			for (; !it->IsDone(); it->Next()) {
				Buffer *b = it->Item();
				string other(b->receiverName == me ? b->senderName : b->receiverName);
				ListIterator<Message*> *mit = new ListIterator<Message*>(b->getMsgList());
				for (; !mit->IsDone(); mit->Next()) {
					if (mit->Item()->receiver == me) {
						Message *msg = mit->Item();
						b->getMsgList()->Remove(msg);

						if (!msg->isAnswer) {
							SendDummyAnswer(msg->sender, msg->receiver, b);
						}
					}
				}
				if (b->getMsgList()->IsEmpty()) { // should only happen when both are dead.
					cout << "Buffer empty: " << b->receiverName << " " << b->senderName << endl;
					kernel->currentThread->bufferpool->Remove(b);
					kernel->threadMap[other]->bufferpool->Remove(b);
					cout << "Before clearing bitmap " << b->bufferId << endl;
					kernel->bitmap->Clear(b->bufferId);
					cout << endl << endl << "Deleting Buffer!!!!!!!" << endl << endl;
					delete b;
				}
			}
			kernel -> currentThread -> Finish();
			return;
		}

		case SC_Write:
		{
			cout << endl;
			int locStart = (int)kernel->machine->ReadRegister(4);
			//cout<<"Start Location: "<<locStart<<endl;
			int noOfCharsToRead = (int)kernel->machine->ReadRegister(5);
			//cout<<"No of charecter to read: "<<noOfCharsToRead<<endl;
			int size = locStart + noOfCharsToRead;
			//cout<<"Size is: "<<size<<endl;
			bool check;
			//cout<<endl<<endl;
			while (locStart < size) {

				check = kernel->machine -> ReadMem(locStart, 1, &value);
				//cout<<endl;
				//cout<<"Value is: "<<value<<" and charecter is: "<<(char)value<<endl;
				cout << (char)value;

				if (check) {
					locStart += 1;
					//cout<<"Incremented Location start: "<<locStart<<endl;
				}
			}
			cout << endl;
			{
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			}
		}

		return;
		break;

		case SC_SendMessage:
		{
			int receiver = (int)kernel->machine->ReadRegister(4);

			int message = (int)kernel->machine->ReadRegister(5);

			int tempBufferId = (int)kernel->machine->ReadRegister(6);

			if (tempBufferId == -1) {

				int bufferId = kernel -> bitmap -> FindAndSet();

				if ( bufferId != -1) {
					HandleSendMessage(receiver , message , bufferId);
					// kernel->machine->WriteRegister(2, bufferId);
				} else {
					cout << "No space in bitmap!" << endl;
					kernel->machine->WriteRegister(2, -1);
				}
			} else {

				HandleSendMessage(receiver , message , tempBufferId);
			}

			/* set previous programm counter (debugging only)*/
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

			/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

			/* set next programm counter for brach execution */
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
		}
		return;

		case SC_WaitMessage:
		{
			int sender = (int)kernel->machine->ReadRegister(4);

			int message = (int)kernel->machine->ReadRegister(5);

			int tempBufferId = (int)kernel->machine->ReadRegister(6);

			HandleWaitMessage(sender , message , tempBufferId);

			{
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			}


		}
		return;

		case SC_SendAnswer: {
			// cout << "Entered Send Answer" << endl;
			// kernel->currentThread->Yield();
			int result = (int)kernel->machine->ReadRegister(4);

			int message = (int)kernel->machine->ReadRegister(5);

			int tempBufferId = (int)kernel->machine->ReadRegister(6);

			SendAnswer(result , message , tempBufferId);

			{
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			}
		}
		return;


		case SC_WaitAnswer: {
			// cout << "Entered Wait Answer" << endl;
			// kernel->currentThread->Yield();
			int result = (int)kernel->machine->ReadRegister(4);

			int message = (int)kernel->machine->ReadRegister(5);

			int tempBufferId = (int)kernel->machine->ReadRegister(6);

			WaitAnswer(result , message , tempBufferId);

			{
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			}
			return;
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
	cout << "yo" << endl;
	ASSERTNOTREACHED();
}

