#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dinput.h>

#pragma  comment(lib,"dinput8.lib")
#pragma  comment(lib,"dxguid.lib")


class Input
{
public:
	void Initialize();
	void Updat();

private:
	HRESULT result;
	HINSTANCE hinstance;
	HWND hwnd;

};

