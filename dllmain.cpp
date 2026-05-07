#pragma once
// 头文件

#include "cryptbase.h"
#include "mono.h"

#include <iostream>

//添加控制台窗口
void InitConsole()
{
	AllocConsole();
	SetConsoleTitleA("YxMod Debug Console");

	// 重定向
	FILE* f;
	if (freopen_s(&f, "CONOUT$", "w", stdout) == 0)
		setvbuf(stdout, NULL, _IONBF, 0);
	if (freopen_s(&f, "CONOUT$", "w", stderr) == 0)
		setvbuf(stderr, NULL, _IONBF, 0);

	// 仅设置 GBK 兼容模式（Unity Mono 最兼容）
	SetConsoleOutputCP(CP_UTF8);
}


DWORD WINAPI ThreadProc(LPVOID lpThreadParameter)
{
	// 初始化控制台
	InitConsole();

	//默认游戏根目录，命名空间，类名，方法名
	Run_CS_method("\\YxMod4.dll", "Doorstop", "Entrypoint", "Start");
	return 0;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, PVOID pvReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);

		if (Load() && Init())
		{
			TCHAR szAppName[MAX_PATH] = TEXT("human.exe");//请修改宿主进程名
			TCHAR szCurName[MAX_PATH];

			GetModuleFileName(NULL, szCurName, MAX_PATH);
			PathStripPath(szCurName);

			//是否判断宿主进程名
			if (StrCmpI(szCurName, szAppName) == 0)
			{
				//线程跑
				HANDLE hThread = CreateThread(NULL, NULL, ThreadProc, NULL, NULL, NULL);
				if (hThread)
{
					CloseHandle(hThread);
				}

				//直跑，会阻塞2333
				//Run_CS_method("\\ScriptManager.dll", "Doorstop", "Entrypoint", "Start");


				// 定时器跑，不阻塞，加载较晚
				//g_uTimerID = SetTimer(NULL, 0, 100, TimerProc);

			}
		}
	}
	else if (dwReason == DLL_PROCESS_DETACH)
    {
		//if (g_uTimerID) KillTimer(NULL, g_uTimerID);
		Free();
		FreeConsole();
    }

    return TRUE;
}

