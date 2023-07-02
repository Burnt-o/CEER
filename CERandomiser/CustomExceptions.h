#pragma once




class CEERExceptionBase : public std::exception
{
private:
	std::string message = "error message not set";
	std::string sourceLocation;
	std::string stackTrace;

	void MandatoryLogging()
	{
		PLOG_ERROR << this->sourceLocation << std::endl << this->message << std::endl << this->stackTrace;
	}

public:
	explicit CEERExceptionBase(const char* msg,
		const std::source_location location = std::source_location::current())
		: message(msg), sourceLocation(std::format("@ {}::{}:{}", location.file_name(), location.function_name(), location.line()).c_str())
	{
		std::stringstream buffer;
		buffer << boost::stacktrace::stacktrace();
		stackTrace = buffer.str();
		MandatoryLogging();
	}

	explicit CEERExceptionBase(std::string msg,
		const std::source_location location = std::source_location::current())
		: message(msg), sourceLocation(std::format("@ {}::{}:{}", location.file_name(), location.function_name(), location.line()).c_str())
	{
		std::stringstream buffer;
		buffer << boost::stacktrace::stacktrace();
		stackTrace = buffer.str();
		MandatoryLogging();
	}

	std::string what()
	{
		return this->message;
	}

	std::string source()
	{
		return this->sourceLocation;
	}

	std::string trace()
	{
		return this->stackTrace;
	}
	void prepend(std::string pre)
	{
		this->message = std::string(pre + this->message);
	}

};





// Classes used for exceptions that we can gracefully handle
// Thrown during CEER/singleton inits if something goes horribly wrong. Error to be shown to user then CEER will shutdown
class InitException : public CEERExceptionBase { using CEERExceptionBase::CEERExceptionBase; };


// Runtime exceptions 
// These will always be passed to the RuntimeExceptionHandler
class CEERRuntimeException : public CEERExceptionBase { using CEERExceptionBase::CEERExceptionBase; };

// thrown on Serialisation/deserialisation failures
// Also passed to RuntimeExceptionHandler
class SerialisationException : public CEERExceptionBase { using CEERExceptionBase::CEERExceptionBase; };
