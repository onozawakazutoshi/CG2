#include "Input.h"
#include "assert.h"

using namespace Microsoft::WRL;

void Input::Initialize(HINSTANCE hinstance_, HWND hwnd_)
{
	hinstance = hinstance_;
	hwnd = hwnd_;

	result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(result));

	
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));

	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));

	result = keyboard->SetCooperativeLevel(hwnd,DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
}

void Input::Updat()
{
	
	memcpy(keyPre, key, sizeof(key));

	keyboard->Acquire();
	
	keyboard->GetDeviceState(sizeof(key), key);

	
}

bool Input::PushKey(BYTE keyNumber)
{
	if (key[keyNumber]) {
		return true;
	}
	return false;
}

bool Input::TriggerKey(BYTE keyNumber)
{
	if (keyPre[keyNumber] && !key[keyNumber]) {
		return true;
	}
	return false;
}
