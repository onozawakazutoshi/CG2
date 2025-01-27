#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#define DIRECTINPUT_VERSION    0x0800
#include <dinput.h>


#pragma  comment(lib,"dinput8.lib")
#pragma  comment(lib,"dxguid.lib")


class Input
{
public:
	void Initialize(HINSTANCE hinstance_,HWND hwnd_);
	void Updat();

	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	bool PushKey(BYTE keyNumber);

	bool TriggerKey(BYTE keyNumber);

private:
	HRESULT result;
	HINSTANCE hinstance;
	HWND hwnd;
	BYTE key[256] = {};
	BYTE keyPre[256] = {};
	ComPtr<IDirectInputDevice8> keyboard;
	ComPtr<IDirectInput8> directInput = nullptr;


};

