#pragma once

//#define throw_except(x) throw expected_exception(std::format("{} @ {}:{}", x, __FILE__, __LINE__)

// Class used for exceptions that are the result of us checking for various bad conditions etc, 
// where we can usually gracefully handle the exception by just not doing the thing we were trying to do

class expected_exception : public std::exception
{
private:
	 const char* message = nullptr;

public:
	explicit expected_exception(const char*  msg, 
		const std::source_location location = std::source_location::current()) 
		: message(std::format("{} @ {}::{}:{}", msg, location.file_name(), location.function_name(), location.line()).c_str()) {}

	explicit expected_exception(std::string msg,
		const std::source_location location = std::source_location::current()) 
		: message(std::format("{} @ {}::{}:{}", msg, location.file_name(), location.function_name(), location.line()).c_str()) {}

	 const char* what()
	{
		return this->message;
	}

	 void prepend(std::string pre)
	 {
		 this->message = std::string(pre + this->message).c_str();
	 }

};
