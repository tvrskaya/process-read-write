#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

#include <vector>

char* FindPattern(char* src, size_t srcLen, const char* pattern, size_t patternLen)
{
	char* cur = src;
	size_t curPos = 0;
	while (curPos < srcLen)
	{
		if (memcmp(cur, pattern, patternLen) == 0)
		{
			return cur;
		}
		curPos++;
		cur = &src[curPos];
	}
	return nullptr;
}

int main(void)
{
	HANDLE h;
	PROCESSENTRY32 singleProcess;
	h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	singleProcess.dwSize = sizeof(PROCESSENTRY32);

	const char* name = "victim.exe";
	DWORD pid;
	bool success = false;
	Process32First(h, &singleProcess);
	do
	{
		if (wcscmp(singleProcess.szExeFile, L"victim.exe") == 0)
		{
			pid = singleProcess.th32ProcessID;
			CloseHandle(h);
			success = true;
		}
	} while (Process32Next(h, &singleProcess));

	if (!success)
	{
		std::cout << "Cant find process id of " << name << std::endl;
		CloseHandle(h);
		return -1;
	}
	std::cout << "Find the victim! This lucky guy is: " << name << " with process id: " << pid << std::endl;
	std::cin.get();

	HANDLE victimHandle = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
	if (victimHandle)
	{
		MEMORY_BASIC_INFORMATION memInfo;
		char* basePtr = (char*)0x0;
		const char* pattern = "victim";
		const char* newData = "NowBadguy";
		while (VirtualQueryEx(victimHandle, (void*)basePtr, &memInfo, sizeof(MEMORY_BASIC_INFORMATION)))
		{
			if (memInfo.State == MEM_COMMIT && memInfo.Protect == PAGE_READWRITE)
			{
				char* localCopyContent = (char*)malloc(memInfo.RegionSize);
				char* remoteMemRegionPtr = (char*)memInfo.BaseAddress;
				SIZE_T bytesRead = 0;
				if (ReadProcessMemory(victimHandle, memInfo.BaseAddress, localCopyContent, memInfo.RegionSize, &bytesRead))
				{
					char* match = FindPattern(localCopyContent, memInfo.RegionSize, pattern, sizeof(pattern));
					if (match)
					{
						uint64_t diff = (uint64_t)match - (uint64_t)localCopyContent;
						char* processPtr = remoteMemRegionPtr + diff;
						std::cout << "Match: " << (char*)match << std::endl;
						std::cout << "Ptr: " << processPtr << std::endl;
						SIZE_T bytesWrite = 0;
						if (WriteProcessMemory(victimHandle, processPtr, newData, sizeof(newData), &bytesWrite))
						{
							std::cout << "Write complete. Bytes write: " << bytesWrite << std::endl;
						}
					}
					free(localCopyContent);
				}
				else
				{
					std::cout << "Cant read process memory!" << std::endl;
				}
			}
			basePtr = (char*)memInfo.BaseAddress + memInfo.RegionSize;
		}
		CloseHandle(victimHandle);
	}
	else
	{
		std::cout << "Fail to open process!" << std::endl;
		return -1;
	}

	return 0;
}