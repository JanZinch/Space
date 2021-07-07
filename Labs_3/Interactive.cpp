#include "interactive.h"


bool Keyboard::InitKeyboard(LPDIRECTINPUT8 &DIObject, HWND hWnd) {
	
	Buffer = new char[256];

	if (DIObject->CreateDevice(GUID_SysKeyboard, &DIKeyboard, NULL) != DI_OK) {

		MessageBox(0, L"������ �������� ������� ��� ����������.", L"������", 0);
		return false;
	}

	if (DIKeyboard->SetDataFormat(&c_dfDIKeyboard) != DI_OK) {

		MessageBox(0, L"������ ��������� ������� ����������.", L"������", 0);
		return false;
	}

	if (DIKeyboard->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE) != DI_OK) {

		MessageBox(0, L"������ ��������� Cooperative Level'� ��� ����������.", L"������", 0);
		return false;
	}

	if (DIKeyboard->Acquire() != DI_OK) {

		MessageBox(0, L"������ ��������� ������� � ����������.", L"������.", 0);
		return false;
	}

	return true;
}

bool Keyboard::GetKeyboardState(void) {
	
	if (!Buffer) {
		
		MessageBox(0, L"������ ���������� �� ���������������.", L"������", 0);
		return false;
	}
	if (DIKeyboard->GetDeviceState(256, (LPVOID)Buffer) != DI_OK) {

		MessageBox(0, L"������ ��������� ��������� ����������.", L"������", 0);
		return false;
	}
	return true;
}

bool Keyboard::GetButtonState(char Button) {
	
	if (!Buffer) {
		
		MessageBox(0, L"������ ���������� �� ���������������.", L"������", 0);
		return false;
	}
	return (Buffer[Button] & 0x80);
}

void Keyboard::Release(void) {
	
	if (DIKeyboard) {

		DIKeyboard->Release();
	}
	if (Buffer) {

		delete[] Buffer;
	}
}






