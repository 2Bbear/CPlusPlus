﻿// ConcurrentProgramProject.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//
#include"stdafx.h"
#include "unique_mutexTest.h"
#include "constexpr_Test.h"
#include "conditionVariableBlockTest.h"
#include "TaskTest.h"
#include "Memory_order_relexedTest.h"
#include "Memory_order_acquireTest.h"
#include "Memory_order_seq_cstTest.h"
#include "MemoryOrderPractice.h"
int main()
{
	Memory_order_relexedTest::Doit();
}
