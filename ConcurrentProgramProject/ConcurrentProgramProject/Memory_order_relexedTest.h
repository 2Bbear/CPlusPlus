#pragma once
namespace Memory_order_relexedTest
{
	/*
	수정 순서가 바뀔 수 있는 코드
	최적화를 끄고 테스트 하면 cpu 순서 재정렬 현상을 관람할 수 있습니다.
	만약 x= 0, y=0이 뜬다면 cpu가 순서를 재정렬했다는 것을 추측할 수 있습니다.
	왜냐면 단일 스레드 관점에서 볼때 x나 y값이 들어가려면 먼저 1이 들어가야 하기 때문입니다.
	그럼에도 0, 0이 되는 경우는 찾기 힘들었다....
	*/
	void Doit();
	/*
	순서 재배치가 일어나도 무방한 코드

	어차피 counter를 증가시킬 것이기 때문에, 전체 총 스레드 수만 맞다면
	수정 순서는 모두 일관적이기 때문에 예상한 결과 값을 얻을 수 있다.
	이경우 40000이 나올 것이다. 스레드가 4개니까.
	*/
	void Doit2();

};

