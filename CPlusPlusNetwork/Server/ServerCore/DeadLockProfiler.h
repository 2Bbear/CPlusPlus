#pragma once
#include <stack>
#include <map>
#include <vector>

/*--------------------
	DeadLockProfiler

# ��� ���� #####################
���̺�� �ڵ带 �������� ���� ����� ��忡�� ����� ��Ȳ�� �߻��ϴ��� �Ǵ��ϴ� �뵵��

# ���� #########################
�׷����� 

# ��� ��� #####################
DeadLockProfiler* GDeadLockProfiler;�� ������ �����ϰ�

���� ������
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif

���� Ǯ��
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif
#############################################


#������ #
���� ����� �̽��� Ÿ�̹� �̽���. �̷��� �̽��� ã�� ���� �������� ����� ��ð� �׽�Ʈ�� ������ ����̴�.
������ �� ����� �������Ϸ��� �ϸ� dfs�� ���� �������ϵ� ���� �Ӵ���, ����� ���¿��� ã�ƾ� �ϱ� ������ ���� ������ ������ ���̰� ���� �� �ִ�.
Ȥ�� ������ ����ȭ ���� �����ߴ� �ϴ��� ã�µ� �ð��� ���� �ɸ� ���̴�.

---------------------*/

class DeadLockProfiler
{
public:
	/// @brief ���� ��� �Լ�
	/// @param name 
	void PushLock(const char* name);
	/// @brief ���� ���� �Լ�
	/// @param name 
	void PopLock(const char* name);
	/// @brief �� ����Ŭ�� �Ͼ���� �Ǵ��ϴ� �Լ�
	void CheckCycle();

private:
	/// @brief ���� Ž���� �Լ�
	/// @param index 
	void Dfs(int32 index);

private:
	/// @brief Ű�� ���� ��ȣ�� �����ϴ� ��
	unordered_map<const char*, int32>	_nameToId;
	/// @brief id�� �̸��� �����ϴ� ��
	unordered_map<int32, const char*>	_idToName;
	/// @brief ���� ����Ǵ��� �����ϴ� �뵵
	stack<int32>						_lockStack;
	/// @brief � ���� � ���� ��Ҵ��� Ȯ���ϴ� �뵵, �� ������ �ش��ϴ� ������ ��� ���̴�.
	map<int32, set<int32>>				_lockHistory;

	Mutex _lock; 

private:
	vector<int32>	_discoveredOrder; // ��尡 �߰ߵ� ������ ����ϴ� �迭
	int32			_discoveredCount = 0; // ��尡 �߰ߵ� ����
	vector<bool>	_finished; // Dfs(i)�� ���� �Ǿ����� ����
	vector<int32>	_parent;
};

