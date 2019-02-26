#include "message.h"
#include <iostream>

void Message::PrintMessage() {
	std::cout << "From: " << sender << " To: " << receiver
	     << " Message: " << msg << std::endl;
}