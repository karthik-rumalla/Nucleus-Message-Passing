#ifndef BUFFER_H
#define BUFFER_H

#include "copyright.h"
#include "list.h"
// #include "process.h"
// #include "thread.h"
#include <string>
#include "message.h"

class Buffer {

public:
    string receiverName;
    string senderName;
    int bufferId;
    List<Message *> *msgList = new List<Message *>;
    string answer;

    string getReceiverName();
    string getSenderName();
    int getBufferId();
    string getAnswer();
    List<Message *> * getMsgList();

    void setReceiverName(string s);
    void setSenderName(string s);
    void setBufferId(int i);
    void setAnswer(string s);
    void appendMessage(Message* s);

    bool hasMessageForMe(char *name);
    Message * popMessageForMe(char *name);
    bool hasAnswerForMe(char *name);
    Message * popAnswerForMe(char *name);

    bool shouldKillBuffer();
};

#endif