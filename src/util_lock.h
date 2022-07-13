#pragma once

class LockableBool
{
public:
	void lock() { data = true; }
	void unlock() { data = false; }

	operator bool() { return data; }

private:
	bool data = false;
};
