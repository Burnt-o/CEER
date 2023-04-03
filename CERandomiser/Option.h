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

	// Onus is on listeners to check if newValue != oldValue, if it cares (it'll also know the type and how to compare them)
	eventpp::CallbackList<void(T& newValue, T& oldValue)> valueChangedEvent;

	void SetValue()
	{
		if (isInputValid(valueDisplay))
		{
			T oldValue = value;
			value = valueDisplay;
			valueChangedEvent(value, oldValue);
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

