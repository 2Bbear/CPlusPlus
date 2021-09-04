#pragma once
/*
인위적으로 크래쉬를 발생시켜서 덤프할때 사용하는 코드
*/
#define CRASH(cause){					\
	uint32_t* crash = nullptr;			\
	__analysis_assume(crash != nullptr);\
	* crash = 0xDEADBEEF;				\
}

//조건에 맞추면 Crash를 일으키는 코드
#define ASSERT_CRASH(expr)				\
{										\
	if (!(expr))						\
	{									\
		CRASH("ASSERT_CRASH");			\
		__analysis_assume(expr);		\
	}									\
}				