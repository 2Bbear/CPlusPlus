#pragma once
#include <stdarg.h>
#include <string>
std::string format_string(const std::string fmt, ...) {
	int size = ((int)fmt.size()) * 2;
	std::string buffer;
	va_list ap;
	while (1) {
		buffer.resize(size);
		va_start(ap, fmt);
		int n = vsnprintf((char*)buffer.data(), size, fmt.c_str(), ap);
		va_end(ap);
		if (n > -1 && n < size) {
			buffer.resize(n);
			return buffer;
		}
		if (n > -1)
			size = n + 1;
		else
			size *= 2;
	}
	return buffer;
}

/*
int main()
{
	std::string test_message = format_string("%d %lf %s %#x\n", 10, 10.5, "test message", 12345678);
	printf(test_message.c_str());
	system("pause");
}*/
