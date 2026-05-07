#pragma once
HMODULE m_hMono = NULL; // Unity Mono 模块句柄
//UINT_PTR g_uTimerID = 0;



typedef void* (__cdecl* t_mono_get_root_domain)();
typedef void* (__cdecl* t_mono_thread_attach)(void* domain);
typedef void* (__cdecl* mono_domain_assembly_open_t)(void* domain, const char* path);
typedef void* (__cdecl* mono_assembly_get_image_t)(void* assembly);
typedef void* (__cdecl* mono_class_from_name_t)(void* image, const char* name_space, const char* class_name);
typedef void* (__cdecl* mono_class_get_method_from_name_t)(void* klass, const char* name, int param_count);
typedef void* (__cdecl* mono_runtime_invoke_t)(void* method, void* obj, void** params, void** exc);


t_mono_get_root_domain mono_get_root_domain_ = NULL;
mono_domain_assembly_open_t fn_assembly_open = NULL;
mono_assembly_get_image_t fn_assembly_image = NULL;
mono_class_from_name_t fn_class_from_name = NULL;
mono_class_get_method_from_name_t fn_get_method = NULL;
mono_runtime_invoke_t fn_invoke = NULL;

t_mono_thread_attach mono_thread_attach_ = NULL;


BOOL WINAPI GetMonoAPI()
{
	WaitForInputIdle(GetCurrentProcess(), 5000);
	Sleep(200);



	m_hMono = GetModuleHandleA("mono-2.0-bdwgc.dll");
	if (!m_hMono)
	{
		MessageBox(NULL, TEXT("错误：获取Mono句柄失败"), TEXT("YxMod"), MB_ICONSTOP);
		return FALSE;
	}
	/*HMODULE m_hMono = NULL;
	for (int i = 0; i < 100; i++)
	{
		m_hMono = GetModuleHandleA("mono-2.0-bdwgc.dll");
		if (m_hMono) break;
		Sleep(100);
		printf("[YxMod] dengdai... %d/100\n", i + 1);
	}

	if (!m_hMono)
	{
		printf("[YxMod] 错误：Mono模块未加载，获取句柄失败\n");
		MessageBox(NULL, TEXT("错误：获取Mono句柄失败"), TEXT("YxMod"), MB_ICONSTOP);
		return FALSE;
	}
	printf("[YxMod] 成功获取Mono模块句柄：0x%p\n", m_hMono);*/




	// 从游戏的Mono模块里获取函数地址

	mono_get_root_domain_ = (t_mono_get_root_domain)(GetProcAddress(m_hMono, "mono_get_root_domain"));
	fn_assembly_open = (mono_domain_assembly_open_t)GetProcAddress(m_hMono, "mono_domain_assembly_open");
	fn_assembly_image = (mono_assembly_get_image_t)GetProcAddress(m_hMono, "mono_assembly_get_image");
	fn_class_from_name = (mono_class_from_name_t)GetProcAddress(m_hMono, "mono_class_from_name");
	fn_get_method = (mono_class_get_method_from_name_t)GetProcAddress(m_hMono, "mono_class_get_method_from_name");
	fn_invoke = (mono_runtime_invoke_t)GetProcAddress(m_hMono, "mono_runtime_invoke");
	mono_thread_attach_ = (t_mono_thread_attach)(GetProcAddress(m_hMono, "mono_thread_attach"));


	if (!mono_get_root_domain_ || !mono_thread_attach_ || !fn_assembly_open || !fn_assembly_image || !fn_class_from_name || !fn_get_method || !fn_invoke)
	{
		MessageBox(NULL, TEXT("错误：获取Mono函数失败"), TEXT("YxMod"), MB_ICONSTOP);
		return FALSE;
	}
	return TRUE;
}




VOID Run_method(const char* modPath, const char* namespace_name, const char* class_name, const char* method_name)
{
	void* mono_domain = mono_get_root_domain_();
	if (!mono_domain)
	{
		MessageBox(NULL, TEXT("错误：获取Mono根域失败"), TEXT("YxMod"), MB_ICONSTOP);
		return;
	}

	//线程Attach，仅调用一次
	mono_thread_attach_(mono_domain);


	void* mod_assembly = fn_assembly_open(mono_domain, modPath);
	if (!mod_assembly)
	{
		MessageBox(NULL, TEXT("错误：打开MOD DLL失败"), TEXT("YxMod"), MB_ICONSTOP);
		return;
	}

	void* mod_image = fn_assembly_image(mod_assembly);
	void* mod_class = fn_class_from_name(mod_image, namespace_name, class_name); // 命名空间.类名    MyMod.ModLoader
	void* mod_method = fn_get_method(mod_class, method_name, 0); // 方法名，参数数量0    Load

	if (!mod_method)
	{
		MessageBox(NULL, TEXT("错误：找不到方法"), TEXT("YxMod"), MB_ICONSTOP);
		return;
	}

	//调用C#方法
	fn_invoke(mod_method, NULL, NULL, NULL);


}

VOID Run_CS_method(const char* dll_path, const char* namespace_name, const char* class_name, const char* method_name)
{
	char modPath[MAX_PATH] = { 0 };
	GetCurrentDirectoryA(MAX_PATH, modPath);
	strcat_s(modPath, dll_path);
	// 检查MOD文件是否存在
	if (GetFileAttributesA(modPath) == INVALID_FILE_ATTRIBUTES)
		return;

	if (!GetMonoAPI())
		return;
	Run_method(modPath, namespace_name, class_name, method_name);
}



// ========================================
// 🔥 Timer 回调：在 Unity 主线程消息循环里运行，绝对不阻塞
// ========================================

//VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
	//{
//	if (!m_hMono)
//	{
//		m_hMono = GetModuleHandleA("mono-2.0-bdwgc.dll");
//		return;
//	}
//	else
//	{
//		if (!mono_get_root_domain_) mono_get_root_domain_ = (t_mono_get_root_domain)(GetProcAddress(m_hMono, "mono_get_root_domain"));
//		if (!fn_assembly_open) fn_assembly_open = (mono_domain_assembly_open_t)GetProcAddress(m_hMono, "mono_domain_assembly_open");
//		if (!fn_assembly_image) fn_assembly_image = (mono_assembly_get_image_t)GetProcAddress(m_hMono, "mono_assembly_get_image");
//		if (!fn_class_from_name) fn_class_from_name = (mono_class_from_name_t)GetProcAddress(m_hMono, "mono_class_from_name");
//		if (!fn_get_method) fn_get_method = (mono_class_get_method_from_name_t)GetProcAddress(m_hMono, "mono_class_get_method_from_name");
//		if (!fn_invoke) fn_invoke = (mono_runtime_invoke_t)GetProcAddress(m_hMono, "mono_runtime_invoke");
//		if (!mono_thread_attach_) mono_thread_attach_ = (t_mono_thread_attach)(GetProcAddress(m_hMono, "mono_thread_attach"));
//
//		if (!mono_get_root_domain_ || !mono_thread_attach_ || !fn_assembly_open || !fn_assembly_image || !fn_class_from_name || !fn_get_method || !fn_invoke)
//		{
//			MessageBox(NULL, TEXT("错误：获取Mono函数失败"), TEXT("YxMod"), MB_ICONSTOP);
//			return;
//		}
//		char modPath[MAX_PATH] = { 0 };
//		GetCurrentDirectoryA(MAX_PATH, modPath);
//		strcat_s(modPath, "\\YxMod4.dll");
//		if (GetFileAttributesA(modPath) != INVALID_FILE_ATTRIBUTES)
//		{
//			Run_method(modPath, "Doorstop", "Entrypoint", "Start");
//		}
//
//		KillTimer(NULL, g_uTimerID);
//	}

//}
