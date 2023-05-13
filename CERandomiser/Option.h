#pragma once

template <typename T>
class Option
{
private:
	T value;
	T valueDisplay; // copy of value for user input
	T defaultValue; // used in case we need to reset
	std::function<bool(T)> isInputValid;
	std::string mOptionName;
public:

	// bool options must be named
	explicit Option(bool defaultValue, std::function<bool(bool)> inputValidator, std::string optionName)
		: isInputValid(inputValidator), value(defaultValue), valueDisplay(defaultValue), mOptionName(optionName)
	{}

	explicit Option(T defaultValue, std::function<bool(T)> inputValidator)
		: isInputValid(inputValidator), value(defaultValue), valueDisplay(defaultValue), mOptionName("Unnamed option")
	{}


	eventpp::CallbackList<void(T& newValue)> valueChangedEvent;

	void UpdateValueWithInput()
	{
		if (isInputValid(valueDisplay))
		{
			value = valueDisplay;
			valueChangedEvent(value);
		}
		else
		{
			valueDisplay = value; // reset valueDisplay back to stored value
		}
	}

	// Called by RuntimeExceptionHandler if associated code throws an exception
	void resetToDefaultValue()
	{
		value = defaultValue;
		valueDisplay = value;
		valueChangedEvent(value);
	}

	T& GetValue()
	{
		return value;
	}

	T& GetValueDisplay()
	{
		return valueDisplay;
	}

	std::string getOptionName() { return mOptionName; } // only used for user facing error messages

};

