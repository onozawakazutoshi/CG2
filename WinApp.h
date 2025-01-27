#pragma once
#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#define DIRECTINPUT_VERSION    0x0800
#include <dinput.h>
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"


#include <Windows.h>
#include <winuser.h>
#include <cstdint>
#include <string>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <cmath>

#pragma  comment(lib,"dinput8.lib")
#pragma  comment(lib,"dxguid.lib")

class WinApp
{
public:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	void Initialize();
	void Update();

	
};

