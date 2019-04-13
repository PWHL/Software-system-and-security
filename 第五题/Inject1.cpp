#include <windows.h>

#include <stdio.h>

#include <tlhelp32.h>



int main() {

	char szDllName[] = "C:\\Users\\ywy\\source\\HookWriteFile1\\Debug\\HookWriteFile1.dll";

	char szExeName[] = "notepad.exe";



	/* Step 1 */

	PROCESSENTRY32 ProcessEntry = {};//������ʱפ����ϵͳ��ַ�ռ��еĽ����б��е���Ŀ

	ProcessEntry.dwSize = sizeof(PROCESSENTRY32);//��ʼ��dwSize

	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);//��ȡָ�����̵Ŀ��գ��Լ�������ʹ�õĶѡ�ģ����߳�

	bool bRet = Process32First(hProcessSnap, &ProcessEntry);//�����й�ϵͳ�����������ĵ�һ�����̵���Ϣ

	DWORD dwProcessId = 0;

	while (bRet) {

		if (strcmp(szExeName, ProcessEntry.szExeFile) == 0) //�ҵ�notepad����
		{

			dwProcessId = ProcessEntry.th32ProcessID;

			break;

		}

		bRet = Process32Next(hProcessSnap, &ProcessEntry);

	}

	if (0 == dwProcessId) {

		printf("�Ҳ�������\n");

		return 1;

	}



	/* Step 2 */

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);//�����еı��ؽ��̶���

	if (0 == hProcess) {

		printf("�޷��򿪽���\n");

		return 1;

	}



	/* Step 3 */

	size_t length = strlen(szDllName) + 1;//����ָ���ܹ�ָ�������ֽ���
	//VirtualAllocEx�������ύ�����ָ�����̵������ַ�ռ��ڵ��ڴ������״̬���ú������������ڴ��ʼ��Ϊ��
	char * pszDllFile = (char *)VirtualAllocEx(hProcess, NULL, length, MEM_COMMIT, PAGE_READWRITE);

	if (0 == pszDllFile) {

		printf("Զ�̿ռ����ʧ��\n");

		return 1;

	}



	/* Step 4 */
	//������д��ָ�������е��ڴ�����
	if (!WriteProcessMemory(hProcess, (PVOID)pszDllFile, (PVOID)szDllName, length, NULL)) {

		printf("Զ�̿ռ�д��ʧ��\n");

		return 1;

	}



	/* Step 5 */
	//PTHREAD_START_ROUTINEָ��֪ͨ�����߳��Ѿ���ʼִ�еĺ���  GetProcAddress��ָ���Ķ�̬���ӿ���������ĺ������߱����ĵ�ַ
	PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle("kernel32"), "LoadLibraryA");

	if (0 == pfnThreadRtn) {

		printf("LoadLibraryA������ַ��ȡʧ��\n");

		return 1;

	}



	/* Step 6 */
	//��������һ�����̵������ַ�ռ������е��߳�
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pfnThreadRtn, (PVOID)pszDllFile, 0, NULL);

	if (0 == hThread) {

		printf("Զ���̴߳���ʧ��\n");

		return 1;

	}



	/* Step 7 */

	WaitForSingleObject(hThread, INFINITE);

	printf("Զ���߳�ִ�����!\n");



	VirtualFreeEx(hProcess, (PVOID)pszDllFile, 0, MEM_RELEASE);

	CloseHandle(hThread);

	CloseHandle(hProcess);



	return 0;

}