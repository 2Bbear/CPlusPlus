#pragma once
namespace TaskTest
{
	/*
	단순한 퓨쳐 사용과 스레드 사용의 비교
	*/
	void Doit();
	/*
	퓨처의 조급한 계산법과 느긋한 계산법 비교
	*/
	void Doit2();

	/*
	fire ,forget 퓨처 사용 테스트
	*/
	void Doit3();

	/*
	퓨처가 네 개인 스칼라곱
	근데 안되는데?
	*/
	void Doit4();

	/*
	std::packaged_task의 동시성 테스트
	0에서 10000까지 합을 계산한다. 결과는 50005000 
	이때 네 개의 packaged_task가 하나하나 별도의 스레드로 실행된다.
	연동된 퓨처는 최종 결과를 나타내는 데 사용된다.
	*/
	void Doit5();
};

