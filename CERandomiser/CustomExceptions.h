#pragma once






// Classes used for exceptions that we can gracefully handle
// Thrown during CEER/singleton inits if something goes horribly wrong. Error to be shown to user then CEER will shutdown
class InitException : public std::exception
{
private:
	std::string message = "error message not set";

public:
	explicit InitException(const char*  msg, 
		const std::source_location location = std::source_location::current()) 
		: message(std::format("{} @ {}::{}:{}", msg, location.file_name(), location.function_name(), location.line()).c_str()) {}

	explicit InitException(std::string msg,
		const std::source_location location = std::source_location::current()) 
		: message(std::format("{} @ {}::{}:{}", msg, location.file_name(), location.function_name(), location.line()).c_str()) {}

	std::string what()
	{
		return this->message;
	}

	 void prepend(std::string pre)
	 {
		 this->message = std::string(pre + this->message);
	 }

};




// Runtime exceptions 
// These will always be passed to the RuntimeExceptionHandler
class CEERRuntimeException : public std::exception
{
private:
	std::string message = "error message not set";

public:
	explicit CEERRuntimeException(const char* msg, 
		const std::source_location location = std::source_location::current())
		: message(std::format("{} @ {}::{}:{}", msg, location.file_name(), location.function_name(), location.line()).c_str()) 
	{
		std::stringstream buffer;
		buffer << message << std::endl << boost::stacktrace::stacktrace();
		message = buffer.str();
	}

	explicit CEERRuntimeException(std::string msg, 
		const std::source_location location = std::source_location::current())
		: message(std::format("{} @ {}::{}:{}", msg, location.file_name(), location.function_name(), location.line()).c_str()) {
		std::stringstream buffer;
		buffer << message << std::endl << boost::stacktrace::stacktrace();
		message = buffer.str();
	}

	std::string what()
	{
		return this->message;
	}

	void prepend(std::string pre)
	{
		this->message = std::string(pre + this->message);
	}



};