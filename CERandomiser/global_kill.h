#pragma once


class global_kill
{
private:
	bool mKillFlag;

	// singleton
	static global_kill& get() {
		static global_kill instance;
		return instance;
	}

	global_kill() = default;
	~global_kill() = default;
public:

	static void kill_me()
	{
		global_kill::get().mKillFlag = true;
	}
	static bool is_kill_set()
	{
		return global_kill::get().mKillFlag;
	}
};