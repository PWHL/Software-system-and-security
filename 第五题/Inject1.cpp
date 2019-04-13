#include <windows.h>

#include <stdio.h>

#include <tlhelp32.h>



int main() {

	char szDllName[] = "C:\\Users\\ywy\\source\\HookWriteFile1\\Debug\\HookWriteFile1.dll";

	char szExeName[] = "notepad.exe";



	/* Step 1 */

	PROCESSENTRY32 ProcessEntry = {};//做快照时驻留在系统地址空间中的进程列表中的条目

	ProcessEntry.dwSize = sizeof(PROCESSENTRY32);//初始化dwSize

	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);//获取指定进程的快照，以及进程所使用的堆、模块和线程

	bool bRet = Process32First(hProcessSnap, &ProcessEntry);//检索有关系统快照中遇到的第一个进程的信息

	DWORD dwProcessId = 0;

	while (bRet) {

		if (strcmp(szExeName, ProcessEntry.szExeFile) == 0) //找到notepad进程
		{

			dwProcessId = ProcessEntry.th32ProcessID;

			break;

		}

		bRet = Process32Next(hProcessSnap, &ProcessEntry);

	}

	if (0 == dwProcessId) {

		printf("找不到进程\n");

		return 1;

	}



	/* Step 2 */

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);//打开现有的本地进程对象

	if (0 == hProcess) {

		printf("无法打开进程\n");

		return 1;

	}



	/* Step 3 */

	size_t length = strlen(szDllName) + 1;//定义指针能够指向的最大字节数
	//VirtualAllocEx保留，提交或更改指定进程的虚拟地址空间内的内存区域的状态。该函数将其分配的内存初始化为零
	char * pszDllFile = (char *)VirtualAllocEx(hProcess, NULL, length, MEM_COMMIT, PAGE_READWRITE);

	if (0 == pszDllFile) {

		printf("远程空间分配失败\n");

		return 1;

	}



	/* Step 4 */
	//将数据写入指定进程中的内存区域
	if (!WriteProcessMemory(hProcess, (PVOID)pszDllFile, (PVOID)szDllName, length, NULL)) {

		printf("远程空间写入失败\n");

		return 1;

	}



	/* Step 5 */
	//PTHREAD_START_ROUTINE指向通知主机线程已经开始执行的函数  GetProcAddress从指定的动态链接库检索导出的函数或者变量的地址
	PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle("kernel32"), "LoadLibraryA");

	if (0 == pfnThreadRtn) {

		printf("LoadLibraryA函数地址获取失败\n");

		return 1;

	}



	/* Step 6 */
	//创建在另一个进程的虚拟地址空间中运行的线程
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pfnThreadRtn, (PVOID)pszDllFile, 0, NULL);

	if (0 == hThread) {

		printf("远程线程创建失败\n");

		return 1;

	}



	/* Step 7 */

	WaitForSingleObject(hThread, INFINITE);

	printf("远程线程执行完毕!\n");



	VirtualFreeEx(hProcess, (PVOID)pszDllFile, 0, MEM_RELEASE);

	CloseHandle(hThread);

	CloseHandle(hProcess);



	return 0;

}