#pragma once

// Class used for exceptions that are the result of us checking for various bad conditions etc, 
// where we can usually gracefully handle the exception by just not doing the thing we were trying to do

class expected_exception : public std::exception
{
private:
	const char* message;

public:
	expected_exception(const char* const msg) : message(msg) {}
	const char* what()
	{
		return this->message;
	}

};
