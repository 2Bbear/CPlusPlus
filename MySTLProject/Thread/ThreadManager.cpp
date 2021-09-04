#include <mutex>

#include "ThreadManager.h"

#include "CoreMacro.h"

ThreadManager::ThreadManager()
{
}

ThreadManager::~ThreadManager()
{
}

void ThreadManager::Launch(std::function<void(void)> callback)
{
}

void ThreadManager::Join()
{
}
