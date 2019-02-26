#include "buffer.h"
#include <stdio.h>

string
Buffer::getReceiverName() {
	return receiverName;
}

string
Buffer::getSenderName() {
	return senderName;
}

int
Buffer::getBufferId() {
	return bufferId;
}

string
Buffer::getAnswer() {
	return answer;
}

List<Message *>*
Buffer::getMsgList() {
	return msgList;
}

void
Buffer::setReceiverName(string recName) {
	receiverName = recName;
}
void
Buffer::setSenderName(string sendName) {
	senderName = sendName;
}

void
Buffer::setBufferId(int bufId) {
	bufferId = bufId;
}
void
Buffer::setAnswer(string ans) {
	answer = ans;
}
void
Buffer::appendMessage(Message* message) {
	msgList -> Append(message);
}


bool Buffer::hasMessageForMe(char *name) {
	cout << "before string constr" << endl;
	string receiver(name);
	cout << "after string constructor hmfm" << endl;
	ListIterator<Message*> *it = new ListIterator<Message*>(msgList);
	for (it -> Item() ; !it -> IsDone() ; it -> Next()) {
		if (it -> Item() -> receiver == receiver && !it->Item()->isAnswer) {
			return TRUE;
		}
	}
	return FALSE;
}

Message * Buffer::popMessageForMe(char *name) {
	string receiver(name);
	ListIterator<Message*> *it = new ListIterator<Message*>(msgList);
	for (it -> Item() ; !it -> IsDone() ; it -> Next()) {
		if (it -> Item() -> receiver == receiver && !it->Item()->isAnswer) {
			msgList->Remove(it->Item());
			return it->Item();
		}
	}
	return NULL;
}

bool Buffer::hasAnswerForMe(char *name) {
	string receiver(name);
	ListIterator<Message*> *it = new ListIterator<Message*>(msgList);
	for (it -> Item() ; !it -> IsDone() ; it -> Next()) {
		if (it -> Item() -> receiver == receiver && it->Item()->isAnswer) {
			return TRUE;
		}
	}
	return FALSE;
}

Message * Buffer::popAnswerForMe(char *name) {
	string receiver(name);
	ListIterator<Message*> *it = new ListIterator<Message*>(msgList);
	for (it -> Item() ; !it -> IsDone() ; it -> Next()) {
		if (it -> Item() -> receiver == receiver && it->Item()->isAnswer) {
			msgList->Remove(it->Item());
			return it->Item();
		}
	}
	return NULL;
}

bool Buffer::shouldKillBuffer() {
	return TRUE;
}