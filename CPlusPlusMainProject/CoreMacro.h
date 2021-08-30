#pragma once
/*
���������� ũ������ �߻����Ѽ� �����Ҷ� ����ϴ� �ڵ�
*/
#define CRASH(cause){					\
	uint32_t* crash = nullptr;			\
	__analysis_assume(crash != nullptr);\
	* crash = 0xDEADBEEF;				\
}

//���ǿ� ���߸� Crash�� ����Ű�� �ڵ�
#define ASSERT_CRASH(expr)				\
{										\
	if (!(expr))						\
	{									\
		CRASH("ASSERT_CRASH");			\
		__analysis_assume(expr);		\
	}									\
}				