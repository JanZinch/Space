#include "Mouse.h"

#pragma comment ( lib , "dinput8.lib" )
#pragma comment ( lib , "dxguid.lib" )


DIMOUSESTATE2 Mouse::STD_DIMOUSEINPUT2;

void Mouse::GetMouseState(void) {

	if (!DIMouse) {

		MessageBox(0, L"���� �� ����������������.", L"������", 0);
		exit(0);
	}

	ZeroMemory(&STD_DIMOUSEINPUT2, sizeof(STD_DIMOUSEINPUT2));

	if (DIMouse->GetDeviceState(sizeof(DIMOUSESTATE2), &STD_DIMOUSEINPUT2) != DI_OK) {

		MessageBox(0, L"������ ������ ��������� ����.", L"������", 0);
		exit(0);
	}
}

bool Mouse::GetButtonState(int mb) {

	if (!DIMouse) {

		MessageBox(0, L"���� �� ����������������.", L"������", 0);
		exit(0);
	}

	return (STD_DIMOUSEINPUT2.rgbButtons[mb] & 0x80);
}

int Mouse::GetOffset(int _MODE) {

	if (!DIMouse) {

		MessageBox(0, L"���� �� ����������������.", L"������", 0);
		exit(0);
	}

	switch (_MODE) {

	case DI_X: return STD_DIMOUSEINPUT2.lX; break;
	case DI_Y: return STD_DIMOUSEINPUT2.lY; break;
	case DI_Z: return STD_DIMOUSEINPUT2.lZ; break;
	}

	MessageBox(0, L"Mouse::GetOffset(...) : ������ �������� _MODE.", L"������", 0);
	return 0;
}

bool Mouse::InitMouse(LPDIRECTINPUT8& DIObject, HWND hWnd) {

	//�������� �������
	if (DIObject->CreateDevice(GUID_SysMouse, &DIMouse, NULL) != DI_OK) {

		MessageBox(0, L"������ �������� ������� ��� ����.", L"������", 0);
		return false;
	}

	if (DIMouse->SetDataFormat(&c_dfDIMouse2) != DI_OK) {

		MessageBox(0, L"������ ��������� ������� ����.", L"������", 0);
		return false;
	}

	//��������� ������ ������ ����
	if (DIMouse->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE) != DI_OK) {

		MessageBox(0, L"������ ��������� Cooperative Level'� ��� ����.", L"������", 0);
		return false;
	}

	//�������� ������ � ����
	if (DIMouse->Acquire() != DI_OK) {

		MessageBox(0, L"������ ��������� ������� � ����.", L"������.", 0);
		return false;
	}

	return true;
}


void Mouse::Release(void) {

	if (DIMouse) { DIMouse->Release(); }
}