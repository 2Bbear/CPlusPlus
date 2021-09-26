#pragma once
#include <stack>
#include <map>
#include <vector>

/*--------------------
	DeadLockProfiler

# 사용 목적 #####################
라이브로 코드를 내보내기 전에 디버그 모드에서 데드락 상황이 발생하는지 판단하는 용도다

# 원리 #########################
그래프로 

# 사용 방법 #####################
DeadLockProfiler* GDeadLockProfiler;을 전역에 선언하고

락을 잡을때
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif

락을 풀때
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif
#############################################


#문제점 #
보통 데드락 이슈는 타이밍 이슈다. 이러한 이슈를 찾는 가장 원초적인 방법은 장시간 테스트를 돌리는 방법이다.
하지만 이 데드락 프로파일러를 하면 dfs로 인한 성능저하도 어마어마할 뿐더러, 디버그 상태에서 찾아야 하기 때문에 실제 릴리즈 버전과 차이가 생길 수 있다.
혹여 릴리즈 최적화 모드로 빌드했다 하더라도 찾는데 시간이 오래 걸릴 것이다.

---------------------*/

class DeadLockProfiler
{
public:
	/// @brief 락을 잡는 함수
	/// @param name 
	void PushLock(const char* name);
	/// @brief 락을 빼는 함수
	/// @param name 
	void PopLock(const char* name);
	/// @brief 락 사이클이 일어나는지 판단하는 함수
	void CheckCycle();

private:
	/// @brief 깊이 탐색용 함수
	/// @param index 
	void Dfs(int32 index);

private:
	/// @brief 키와 락의 번호를 관리하는 맵
	unordered_map<const char*, int32>	_nameToId;
	/// @brief id랑 이름을 맵핑하는 맵
	unordered_map<int32, const char*>	_idToName;
	/// @brief 락이 실행되는지 추적하는 용도
	stack<int32>						_lockStack;
	/// @brief 어떤 락이 어떤 락을 잡았는지 확인하는 용도, 즉 간선에 해당하는 정보를 담는 맵이다.
	map<int32, set<int32>>				_lockHistory;

	Mutex _lock; 

private:
	vector<int32>	_discoveredOrder; // 노드가 발견된 순서를 기록하는 배열
	int32			_discoveredCount = 0; // 노드가 발견된 순서
	vector<bool>	_finished; // Dfs(i)가 종료 되었는지 여부
	vector<int32>	_parent;
};

