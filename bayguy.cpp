#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

#include <vector>

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
		while (VirtualQueryEx(victimHandle, (void*)basePtr, &memInfo, sizeof(MEMORY_BASIC_INFORMATION)))
		{
			char* localCopyContent = (char*)malloc(memInfo.RegionSize);

			SIZE_T bytesRead = 0;
			int data = 100;
			if (ReadProcessMemory(victimHandle,	memInfo.BaseAddress, localCopyContent, memInfo.RegionSize, &bytesRead))
			{
				for (SIZE_T i = 0; i < bytesRead; i++)
				{
					if ((int)localCopyContent[i] == data)
					{
						std::cout << (void*)&localCopyContent[i] << std::endl;
						WriteProcessMemory(victimHandle, &localCopyContent, &data, sizeof(data), nullptr);
					}
				}
				free(localCopyContent);
			}
			else
			{
				std::cout << "Cant read process memory!" << std::endl;
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