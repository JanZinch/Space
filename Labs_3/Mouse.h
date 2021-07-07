#pragma once
#include <dinput.h>

#pragma comment ( lib , "dinput8.lib" )
#pragma comment ( lib , "dxguid.lib" )

#define			DI_X		0
#define			DI_Y		1
#define			DI_Z		2

#define LEFT_BUTTON 0
#define RIGHT_BUTTON 1
#define MIDDLE_BUTTON 3


class Mouse {

private:

	static DIMOUSESTATE2 STD_DIMOUSEINPUT2;

	LPDIRECTINPUTDEVICE8 DIMouse;

public:

	bool InitMouse(LPDIRECTINPUT8 &, HWND);
	void GetMouseState(void);
	bool GetButtonState(int);
	int	GetOffset(int);
	void Release(void);
	~Mouse() {}
};

