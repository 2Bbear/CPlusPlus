#pragma once
#include <memory>
#include <string>
template<typename ... Args>
std::string format_string(const std::string& format, Args ... args)
{
	size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1;
	std::unique_ptr<char[]> buffer(new char[size]);
	snprintf(buffer.get(), size, format.c_str(), args ...);
	return std::string(buffer.get(), buffer.get() + size - 1);
}
/*
int main()
{
	std::string test_message = format_string("%d %lf %s %#x\n", 10, 10.5, "test message", 12345678);
	printf(test_message.c_str());
	system("pause");
}*/
