#pragma once

template <typename T>
class Option
{
private:
	T value;
	T valueDisplay; // copy of value for user input
	std::function<bool(T)> isInputValid;
public:


	explicit Option(T defaultValue, std::function<bool(T)> inputValidator) 
		: isInputValid(inputValidator), value(defaultValue), valueDisplay(defaultValue)
	{}

	eventpp::CallbackList<void(T& newValue)> valueChangedEvent;

	void SetValue()
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

	T& GetValue()
	{
		return value;
	}

	T& GetValueDisplay()
	{
		return valueDisplay;
	}

};

