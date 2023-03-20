#pragma once


class config_window
{
private:

	static config_window& get() {
		static config_window instance;
		return instance;
	}

	config_window() = default;
	~config_window() = default;

public:
	static void initialize();

};