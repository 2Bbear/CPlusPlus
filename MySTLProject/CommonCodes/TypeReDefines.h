#pragma once
#include <mutex>
#include <atomic>


//자주 사용할 타입 재정의
using BYTE = unsigned char;
using int8 = __int8;
using int15 = __int16;
using int32 = __int32;
using int64 = __int64;
using uint8 = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;


//mutex 버전이나 atomic 새 버전을 넣고 싶다면 이 부분을 수정하여 전체적으로 반영할 수 있도록 한다.
template<typename T>
using Atomic = std::atomic<T>;
using Mutex = std::mutex;
using CondVar = std::condition_variable;
using UniqueLock = std::unique_lock<std::mutex>;
using LockGuard = std::lock_guard<std::mutex>;