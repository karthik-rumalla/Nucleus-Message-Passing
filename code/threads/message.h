#ifndef MESSAGE_H
#define MESSAGE_H

#include "copyright.h"
#include <string>

class Message {
public:
	std::string msg;
	std::string sender;
	std::string receiver;
	bool isAnswer;
	int answerType;

	void PrintMessage();
};

#endif