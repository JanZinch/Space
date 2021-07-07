#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>

#include <stdio.h>
#include <debugapi.h>
#include "resource.h"
#include "interactive.h"
#include "Mouse.h"
#include "meshobj.h"
#include "ConeMesh.h"
//#include "TextWriter.h"

#define BGR 104

#define START_BKG_ANGLE -1.55555f

#define NORMAL 110
#define DIZZY -110

enum Figures {

	PYRAMID = 101, CONUS = 102, TORUS = 103
};


//--------------------------------------------------------------------------------------
// ���������
//--------------------------------------------------------------------------------------

struct Vertex {                             // �������

	XMFLOAT3 Position;						// ���������� ����� � ������������ x, y, z
	XMFLOAT3 Normal;						// ������� �������
	XMFLOAT2 Tex;						    // ���������� �������� tu, tv
};

struct Camera {

	XMFLOAT3 position;
	XMFLOAT3 target;
	float FovAngle;
};


struct DirectLight {

	XMFLOAT4 vLightDirection;				// ����������� ����� (������� ����������)
	XMFLOAT4 vLightColor;					// ���� ����������
};

struct ConstantBufferMatrixes {                      // ����� ��� ������� (������ ���������)

	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
};

struct ConstantBufferLight {

	XMFLOAT4 vLightDirection[2];				// ����������� �����
	XMFLOAT4 vLightColor[2];					// ���� ���������
	XMFLOAT4 vOutputColor;						// �������� ���� (��� ������� PSSolid)
	XMFLOAT4 vTime;
};


//--------------------------------------------------------------------------------------
// ���������� ����������
//--------------------------------------------------------------------------------------
HINSTANCE               g_hInst = NULL;                                          // ����������
HWND                    g_hWnd = NULL;                                           // ����

D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;                     // ����������: ���������� ��� ����������� ���������
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;                 // ������ DirectX, �������������� ����������� 

//--------------------����������-Direct3D------------------

ID3D11Device*           g_pd3dDevice = NULL;                                     // ��������� �������� ��������
ID3D11DeviceContext*    g_pImmediateContext = NULL;                              // ��������� ������ ������. ���������� (��������)
IDXGISwapChain*         g_pSwapChain = NULL;                                     // ��� ������ (��������, ������)
ID3D11RenderTargetView* g_pRenderTargetView = NULL;                              // ������ �����

ID3D11VertexShader*     g_pVertexShader = NULL;                                 // ��������� ������
ID3D11PixelShader*		g_pPixelShader = NULL;									// ���������� ������
ID3D11PixelShader*      g_pPixelShaderSolid = NULL;								// ���������� ������ ��� ���������� �����
ID3D11PixelShader*		g_pPixelShaderBackground = NULL;						// ���������� ������ ��� ����
ID3D11PixelShader*		g_pPixelShaderEffect = NULL;							// ���������� ������ ������

ID3D11InputLayout*		g_pVertexLayout = NULL;									// �������� ������� ������
ID3D11Buffer*           g_pVertexBuffer = NULL;									// ����� ������
ID3D11Buffer*           g_pIndexBuffer = NULL;                                  // ��������� �����
ID3D11Buffer*           g_pCBMatrixes = NULL;									// ����������� ����� � ����������� � ��������
ID3D11Buffer*           g_pCBLight = NULL;										// ����������� ����� � ����������� � �����

ID3D11Texture2D*        g_pDepthStencil = NULL;                                 // �������� ������ �������
ID3D11DepthStencilView* g_pDepthStencilView = NULL;								// ������ ����, ����� ������

ID3D11ShaderResourceView* g_pTextureRV = NULL;									// ������ ��������
ID3D11SamplerState*       g_pSamplerLinear = NULL;								// ��������� ��������� ��������

ID3D11ShaderResourceView* g_pTextureSB = NULL;									// ������ ��������
ID3D11SamplerState*       g_pSamplerSkyBox = NULL;								// ��������� ��������� ��������


LPDIRECTINPUT8			DIObject;                                               // ������ DirectInput
Keyboard				kb;														// �����-�������� ��� LPDIRECTINPUTDEVICE8 (����������)
Mouse                   ms;

//---------------------------------------------------------


XMMATRIX                g_World;
XMMATRIX                g_View;
XMMATRIX                g_Projection;

FLOAT					playTime = 0.0f;												// ����������-�����

DirectLight lights[2];															// ��������� ������������� �����

MeshFromObj*		    g_pMesh = NULL;
ConeMesh*				g_Mesh = NULL;


byte dr_mode = PYRAMID;
byte sh_mode = NORMAL;


Vertex vertices[] = {

	// �������(x, y, z), �������(n(vector)), ��������(u, v)

	{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },    // ���
	{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

	{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

	{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
	{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f),  XMFLOAT2(0.0f, 1.0f) },

	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

	{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },



	{ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },      // ��������
	{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

	{ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

	{ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

	{ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

	{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },

	// ���

	{ XMFLOAT3(2.0f, -1.0f, -5.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
	{ XMFLOAT3(2.0f, 1.0f, -5.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3(-2.0f, -1.0f, -5.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
	{ XMFLOAT3(-2.0f, 1.0f, -5.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },


};

WORD indices[] = {

		3,1,0,							// ���
		2,1,3,

		6,4,5,
		7,4,6,

		11,9,8,
		10,9,11,

		14,12,13,
		15,12,14,

		19,17,16,
		18,17,19,

		22,20,21,
		23,20,22,

		// ��������
		24, 25, 26,
		27, 28, 29,

		30,31,32,
		33,34,35,

		36,37, 38,
		38, 37, 39,

		// ���

		40, 41, 42,
		42, 41, 43

};


//--------------------------------------------------------------------------------------
// ���������� �������
//--------------------------------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
HRESULT InitDevice();
HRESULT InitGeometry();                                       // �������� ������, �������� ��������
HRESULT InitMatrixes();                                       // ������������� ������
void UpdateLight();											// ���������� ����������� �����
void UpdateMatrix(UINT nLightIndex, float, float);						// ������� ���������� ������
void Render();													// ���������
void CleanupDevice();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);        // ���� ��������� ���������
HRESULT InitDirectInput(HINSTANCE hInstance);                // ������������� DirectInput
HRESULT CompileShaderFromFile(WCHAR*, LPCSTR, LPCSTR, ID3DBlob**);    // ������� ��� �������� ��������



//--------------------------------------------------------------------------------------
// ������� ������� ���������. ���������� ��� �������������, � ����� �����������
// ���� ��������� ���������
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{

	OutputDebugStringA("O+\n");

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(InitWindow(hInstance, nCmdShow))) {                // �������� ���� ����������
		
		OutputDebugStringA("O+\n");
		return 0;
	}

	if (FAILED(InitDevice()))                                   // �������� �������� DirectX
	{
		OutputDebugStringA("O+\n");
		CleanupDevice();
		return 0;
	}

	if (FAILED(InitGeometry())) {

		OutputDebugStringA("O+\n");
		CleanupDevice();
		return 0;
	}

	if (FAILED(InitDirectInput(g_hInst))) {
	
		OutputDebugStringA("O+\n");
		CleanupDevice();
		return 0;
	}

	if (!kb.InitKeyboard(DIObject, g_hWnd)) {
		
		OutputDebugStringA("O+\n");
		CleanupDevice();
		return 0;
	}

	if (!ms.InitMouse(DIObject, g_hWnd)) {

		OutputDebugStringA("O+\n");
		CleanupDevice();
		return 0;
	}

	// ���� ��������� ���������
	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Render();
		}
	}

	CleanupDevice();

	return (int)msg.wParam;
}

HRESULT InitDirectInput(HINSTANCE hInstance) {

	if (DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&DIObject, NULL) != DI_OK) {

		MessageBox(0, L"�� ������� ���������������� DirectInput.", L"������", 0);

		return E_FAIL;
	}
	else {
	
		return S_OK;
	}
}


//--------------------------------------------------------------------------------------
// ��������� ��������� ��������� Windows
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}


//--------------------------------------------------------------------------------------
// �������� ����
//--------------------------------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"Article1";
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);

	if (!RegisterClassEx(&wcex)) { return E_FAIL; }
		
	// Create window
	g_hInst = hInstance;
	RECT rc = { 0, 0, 1080, 580 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(L"Article1", L"Space", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 
		CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

	if (!g_hWnd) { return E_FAIL; }

	ShowWindow(g_hWnd, nCmdShow);
	ShowCursor(false);
	return S_OK;
}


//--------------------------------------------------------------------------------------
// ������������� Direct3D ����������
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};

	UINT numDriverTypes = ARRAYSIZE(driverTypes);			// �������� ��������� ���������� ���������

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};								// ������ ������ DirectX
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;                                 // (���������) ���� �����
	ZeroMemory(&sd, sizeof(sd));			                 // �������
	sd.BufferCount = 1;                                      // ����� �������
	sd.BufferDesc.Width = width;                             // ������ ������
	sd.BufferDesc.Height = height;                           // ������ ������
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;       // ������ �������
	sd.BufferDesc.RefreshRate.Numerator = 60;                // ������� ���������� ������
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;         // ���������� ������
	sd.OutputWindow = g_hWnd;                                 // �������� � ����
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;                                        // �� ������������� �����

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];

		hr = D3D11CreateDeviceAndSwapChain(NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);	
		
		if (SUCCEEDED(hr))                  // ���� ���������� ������� �������
			break;
	}
	if (FAILED(hr))
		return hr;

	// �������� ������-�������

	// RenderTargetOutput - �������� �����
	// RenderTargetView - ������ �����

	ID3D11Texture2D* pBackBuffer = NULL;             // ������ ������� �������
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
		return hr;

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);		// �� ����������� �������� ������� ����������� ���������
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

	// ��������� � �������� ������ ������
	// ������� ��������-�������� ������ ������
	D3D11_TEXTURE2D_DESC descDepth;					 // ��������� � �����������
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = width;							// ������ � ������ ��������
	descDepth.Height = height;
	descDepth.MipLevels = 1;							// ������� ������������
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;   // ������ (������ �������)
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;		// ��� - ����� ������
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	hr = g_pd3dDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencil);		// ��� ������ ����������� ���������-�������� ������� ������ ��������
	if (FAILED(hr))	return hr;

	// �������� ������ ������� ������ ������
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;							// ������ ��� � ��������
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	// ��� ������ ����������� ���������-�������� � �������� ������� ������ ������ ������
	hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);

	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);   // ���������� ������ ������� ������ � ������ ������ ������ � ��������� ����������

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;                                        // ���������� ������� ������� ����
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;                                             // ������� ������ �������
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;                                                // ���������� �������� ������ ����
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);					// ����������� � ��������� ���������� ��������

	return S_OK;
}

//--------------------------------------------------------------------------------------
// ��������������� ������� ��� �������������� �������� DX11
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		if (pErrorBlob != NULL) {

			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		}

		if (pErrorBlob) pErrorBlob->Release();
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}




//--------------------------------------------------------------------------------------
// �������� ������ ������, �������� (shaders) � �������� ������� ������ (input layout)
//--------------------------------------------------------------------------------------

HRESULT InitGeometry() {

	HRESULT hr = S_OK;

	ID3DBlob* pVSBlob = NULL;    // ��������������� ������ - ������ ����� � ����������� ������	
	hr = CompileShaderFromFile(L"Shaders.fx", "VS", "vs_4_0", &pVSBlob);	// ���������� ���������� ������� �� �����: �������� �� ����� ".fx" �������-������� "VS" ������ "vs_4_0" � ������ pVSBlob
	if (FAILED(hr)) {

		MessageBox(NULL, L"���������� �������������� ���� FX. ����������, ��������� ������ ��������� �� �����, ���������� ���� FX.", L"������", MB_OK);
		return hr;
	}

	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader);         // �������� ���������� �������

	if (FAILED(hr)) {

		pVSBlob->Release();
		return hr;
	}

	// ����������� ������� ������
	D3D11_INPUT_ELEMENT_DESC layout[] = {

		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },    /* ������������� ���, ������������� ������, ������, �������� ���� (0-15), ����� ������ ������ � ������ ������ (��������), ����� ��������� ����� (�� �����), InstanceDataStepRate (�� �����) */
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },		// 8 = sizeof(TEXCOORD) ���� - ��������
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }		// 12 = sizeof(POSITION) ���� - ��������
	};

	UINT numElements = ARRAYSIZE(layout);
	hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &g_pVertexLayout);			// �������� ������� ������
	pVSBlob->Release();
	if (FAILED(hr)) { return hr; }

	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);								// ����������� ������� ������

	// ���������� ����������� ������� ��� ��������� �������� ���� �� �����
	ID3DBlob* pPSBlob = NULL;
	hr = CompileShaderFromFile(L"Shaders.fx", "PS", "ps_4_0", &pPSBlob);					

	if (FAILED(hr)) {

		MessageBox(NULL, L"���������� �������������� ���� FX. ����������, ��������� ������ ��������� �� �����, ���������� ���� FX.", L"������", MB_OK);
		return hr;
	}
	// �������� ����������� �������
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader);				
	pPSBlob->Release();
	if (FAILED(hr)) { return hr; }


	// ���������� ����������� ������� ��� ���������� ����� �� �����
	pPSBlob = NULL;
	hr = CompileShaderFromFile(L"Shaders.fx", "PSSolid", "ps_4_0", &pPSBlob);

	if (FAILED(hr)){

		MessageBox(NULL, L"���������� �������������� ���� FX. ����������, ��������� ������ ��������� �� �����, ���������� ���� FX.", L"������", MB_OK);
		return hr;
	}
	// �������� ����������� �������
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShaderSolid);
	pPSBlob->Release();
	if (FAILED(hr)) { return hr; }


	// ���������� ����������� ������� ��� ���� �� �����
	pPSBlob = NULL;
	hr = CompileShaderFromFile(L"Shaders.fx", "PSBkg", "ps_4_0", &pPSBlob);

	if (FAILED(hr)) {

		MessageBox(NULL, L"���������� �������������� ���� FX. ����������, ��������� ������ ��������� �� �����, ���������� ���� FX.", L"������", MB_OK);
		return hr;
	}
	// �������� ����������� �������
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShaderBackground);
	pPSBlob->Release();
	if (FAILED(hr)) { return hr; }


	// ���������� ����������� ������� ��� �������
	pPSBlob = NULL;
	hr = CompileShaderFromFile(L"Shaders.fx", "PSEffect", "ps_4_0", &pPSBlob);

	if (FAILED(hr)) {

		MessageBox(NULL, L"���������� �������������� ���� FX. ����������, ��������� ������ ��������� �� �����, ���������� ���� FX.", L"������", MB_OK);
		return hr;
	}
	// �������� ����������� �������
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShaderEffect);
	pPSBlob->Release();
	if (FAILED(hr)) { return hr; }


	D3D11_BUFFER_DESC bd;                           // ���������, ����������� ����������� ����� (� ����� ������ - ����� ������, D3D11_BUFFER... - ��������� �������)
	ZeroMemory(&bd, sizeof(bd));                    // ������� ��
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(vertices);                // ������ ������ = ������ ����� ������� * ���-�� ������
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;        // ��� ������ - ����� ������
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;				// ���������, ���������� ������ ������
	ZeroMemory(&InitData, sizeof(InitData));		// ������� ��
	InitData.pSysMem = vertices;					// ��������� �� ���� 3 �������

	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);      // ����� ������ g_pd3dDevice ������� ������ ������ ������ ID3D11Buffer

	if (FAILED(hr)) return hr;

	// ��������� ������ ������:

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// �������� ���������� ������
	// ������ ��� �������� - ������� ������ (�� �������������� �������) ��� �����. ������������ (��������)

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(indices);															// sizeof(��� ��������) * ���-�� ������, ����������� ���� ������������� � �����
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = indices;                                                                 // ��������� �� ������ ��������
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer);
	if (FAILED(hr)) { return hr; }

	g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);				// ��������� ���������� ������	
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);			// ��������� ���� ����������

	// �������� ������ �������� ������ �������������� ��� �������
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBufferMatrixes);        // ������ ������ = ������� ���������
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;            // ��� - ����������� �����
	bd.CPUAccessFlags = 0;
	hr = g_pd3dDevice->CreateBuffer(&bd, NULL, &g_pCBMatrixes);
	if (FAILED(hr)) { return hr; }

	// �������� ������ �������� ���������� ����� ��� �������
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBufferLight);          // ������ ������ = ������� ���������
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;           // ��� - ����������� �����
	bd.CPUAccessFlags = 0;
	hr = g_pd3dDevice->CreateBuffer(&bd, NULL, &g_pCBLight);
	if (FAILED(hr)) { return hr; }

	// ���� ������ �� ����������, ���������������� ������� ���� � �������� ����� �����
	// � ���� ������ ������� ��� ������ � ��������� �� �����������

	// �������� �������� �� �����
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"Floors.png", NULL, NULL, &g_pTextureRV, NULL);
	if (FAILED(hr)) {

		MessageBox(NULL, L"���������� ��������� ��������. ����������, ��������� ������ ��������� �� �����, ���������� ���� PNG ��� DDS.", L"������", MB_OK); 
		return hr; 
	}

	// �������� �������� �� �����
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"Sky.jpg", NULL, NULL, &g_pTextureSB, NULL);
	if (FAILED(hr)) {

		MessageBox(NULL, L"���������� ��������� ��������. ����������, ��������� ������ ��������� �� �����, ���������� ���� PNG ��� DDS.", L"������", MB_OK);
		return hr;
	}

	// �������� ������ (��������) ��������
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;      // ��� ����������
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;         // ������ ����������
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// ������� ��������� ������ ���������������
	hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerLinear);
	if (FAILED(hr)) { return hr; }

	InitMatrixes();

	//�������� ��������
	g_pMesh = new MeshFromObj(g_pd3dDevice, g_pImmediateContext, "torus.obj");	
	g_Mesh = new ConeMesh(g_pd3dDevice, g_pImmediateContext);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// ������������� ������
//--------------------------------------------------------------------------------------
HRESULT InitMatrixes() {

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;								// ������ ���� 
	UINT height = rc.bottom - rc.top;								// ������ ����

	g_World = XMMatrixIdentity();									// ������������� ������� ����

	g_View = XMMatrixLookAtLH(XMVectorSet(0.0f, 1.0f, -10.0f, 0.0f), XMVectorSet(0.0f, 0.5f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

	g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f);   // ������������� ������� �������� (������ ���� ���������(pi/4), "������������" �������, ������ ������� ������� (���������� �� ������), ����� ���. ���.)

	return S_OK;
}


void UpdateTime() {

	// ���������� ����������-�������

	if (playTime > 500000) {
	
		playTime = 0;	
	}

	if (g_driverType == D3D_DRIVER_TYPE_REFERENCE) {

		playTime += (float)XM_PI * 0.0125f;
	}
	else {

		static DWORD dwTimeStart = 0;
		DWORD dwTimeCur = GetTickCount();

		if (dwTimeStart == 0) { dwTimeStart = dwTimeCur; }

		playTime = (dwTimeCur - dwTimeStart) / 1000.0f;
	}

}


//--------------------------------------------------------------------------------------
// ��������� ����������� �����
//--------------------------------------------------------------------------------------

void UpdateLight(){

	static float angle = 0;
	static float red_pos = 0.0f;
	static bool red_state = false;
	static FLOAT deltaTime = 0.0f;

	if (kb.GetButtonState(DIK_R)) {

		if (playTime - deltaTime > 0.5f) {
		
			deltaTime = playTime;

			if (!red_state) {

				red_state = true;
				red_pos++;
			}
			else {

				red_state = false;
				red_pos--;
			}
			
		}
	}

	// ������ ��������� ���������� ���������� �����

	lights[0].vLightDirection = XMFLOAT4(2.0f, 0.0f, 2.0f, 1.0f);	
	lights[1].vLightDirection = XMFLOAT4(red_pos * 1.6, 0.0f, red_pos * 1.6, 1.0f);

	// ������ ���� ���������� �����, � ��� �� �� ����� ��������
	lights[0].vLightColor = XMFLOAT4(1.0f, 1.0f, 0.5f, 1.0f);
	lights[1].vLightColor = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

	// ��� ������ ������������� ������������ ������ �������� �����
	XMMATRIX mRotate = XMMatrixRotationY(-2.0f * playTime);
	XMVECTOR vLightDir = XMLoadFloat4(&lights[1].vLightDirection);
	vLightDir = XMVector3Transform(vLightDir, mRotate);
	XMStoreFloat4(&lights[1].vLightDirection, vLightDir);

	if (kb.GetButtonState(DIK_A)) {

		angle += 0.0005f;

		if (angle > 120) {

			angle = 0;
		}
	}
	else if (kb.GetButtonState(DIK_D)) {

		angle -= 0.0005f;

		if (angle < -120) {

			angle = 0;
		}
	}

	// ��� ������ ������������� ������������ ������ �������� �����
	mRotate = XMMatrixRotationY(0.5 * angle);
	vLightDir = XMLoadFloat4(&lights[0].vLightDirection);
	vLightDir = XMVector3Transform(vLightDir, mRotate);
	XMStoreFloat4(&lights[0].vLightDirection, vLightDir);

}

//--------------------------------------------------------------------------------------
// ������������� ������� ��� �������� ��������� ����� (0-1) ��� ���� (MX_SETWORLD)
//--------------------------------------------------------------------------------------

void UpdateMatrix(UINT object_code, float orbit = 0, float bkg_angle = 0){

	// ��������� �������� �������

	if (object_code == PYRAMID || object_code == CONUS) {

		static XMFLOAT2 model_angle = { 0.0f, 0.0f };

		if (kb.GetButtonState(DIK_K)) {

			model_angle.x += 0.001;	
		}
		else if (kb.GetButtonState(DIK_I)) {

			model_angle.x -= 0.001;
		}
		else if (kb.GetButtonState(DIK_J)) {

			model_angle.y += 0.001;
		}
		else if (kb.GetButtonState(DIK_L)) {

			model_angle.y -= 0.001;
		}
	
		if (object_code == PYRAMID) {
		
			g_World = XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixTranslation(0.0f, 0.0f, 0.0f) * XMMatrixRotationX(model_angle.x) *  XMMatrixRotationY(model_angle.y);
		}
		else {
		
			g_World = XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixTranslation(0.0f, -1.0f, 0.0f) * XMMatrixRotationX(model_angle.x) *  XMMatrixRotationY(model_angle.y);
		}	
		
		object_code = 0;
	}
	else if (object_code == BGR) {
	
		
		g_World = XMMatrixScaling(35.0f, 35.0f, 1.0f) * XMMatrixTranslation(0.0f, 0.5f, -70.0f) * XMMatrixRotationY(-1 * bkg_angle + 10.99f) * XMMatrixTranslation(0.0f, 0.5f, orbit);   //40
	
	}	
	else if (object_code < 2) {

		// ���� ������ ��������� �����: ���������� ������� � ����� � ��������� � 5 ���
		g_World = XMMatrixTranslationFromVector(5.0f * XMLoadFloat4(&lights[object_code].vLightDirection));
		XMMATRIX mLightScale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
		g_World = mLightScale * g_World;
	}
	else {

		object_code = 0;
	}

	// ���������� ����������� ������������ ������
	ConstantBufferMatrixes cbm;									// ��������� ���������
	cbm.mWorld = XMMatrixTranspose(g_World);				// ��������� � ���� �������
	cbm.mView = XMMatrixTranspose(g_View);
	cbm.mProjection = XMMatrixTranspose(g_Projection);
	g_pImmediateContext->UpdateSubresource(g_pCBMatrixes, 0, NULL, &cbm, 0, 0);


	static INT count = 36;
	if (orbit != 0) count = -1 * orbit;	

	ConstantBufferLight cbl;
	cbl.vLightDirection[0] = lights[0].vLightDirection;				// ��������� ������ � �����
	cbl.vLightDirection[1] = lights[1].vLightDirection;
	cbl.vLightColor[0] = lights[0].vLightColor;
	cbl.vLightColor[1] = lights[1].vLightColor;
	cbl.vOutputColor = lights[object_code].vLightColor;	
	cbl.vTime = XMFLOAT4(playTime, count, 0, 0);					//cbl.vTime.x = t;
	g_pImmediateContext->UpdateSubresource(g_pCBLight, 0, NULL, &cbl, 0, 0);

}


//--------------------------------------------------------------------------------------
// ������
//--------------------------------------------------------------------------------------
void Render() {

	float ClearColor[4] = { 0.0f, 0.1f, 0.3f, 1.0f };												// ����
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);                    // ������� ������� ������� (������-�������) �������� ������
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);    // ������� ������ �������

	UpdateTime();

	static float orbit = 10.0f;
	static XMFLOAT3 at = { 0.0f, 1.0f, 0.0f };
	char buf[80];

	// ��������� ����������� ������ ������ � ��������
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);			// ��������� ���� ����������


	kb.GetKeyboardState();
	UpdateLight();																				// ��������� ���������
	ms.GetMouseState();


	static float bkg_angle = START_BKG_ANGLE;

	if (kb.GetButtonState(DIK_LBRACKET) || ms.GetButtonState(LEFT_BUTTON)) {

		bkg_angle += 0.0005f;

		if (bkg_angle > 360) {

			bkg_angle -= 360;
		}

	}
	else if (kb.GetButtonState(DIK_RBRACKET) || ms.GetButtonState(RIGHT_BUTTON)) {

		bkg_angle -= 0.0005f;

		if (bkg_angle < -360) {

			bkg_angle += 360;
		}
	}

	at.x = cos(bkg_angle);
	at.z = sin(bkg_angle);

	if (ms.GetOffset(DI_Z) > 0 && orbit > 4.0f) {
	
		orbit -= 0.5;
	}
	else if (ms.GetOffset(DI_Z) < 0 && orbit < 50.0f) {
	
		orbit += 0.5;
	}
	
	static FLOAT deltaTime = 0.0f;

	if (kb.GetButtonState(DIK_F)) {
	
		if (playTime - deltaTime > 0.5f) {

			deltaTime = playTime;

			if (dr_mode != TORUS) {
			
				dr_mode++;
			}
			else {
			
				dr_mode = PYRAMID;	
			}		
		}
		
	}

	if (kb.GetButtonState(DIK_E)) {

		if (playTime - deltaTime > 0.5f) {

			deltaTime = playTime;
			sh_mode *= -1;	
		}
	}

	
	XMVECTOR Eye = XMVectorSet(0.0f, 1.0f, orbit, 0.0f);						// ��������� ������           
	XMVECTOR At = XMVectorSet(at.x, at.y, at.z + orbit, 0.0f);		// ��������� �����, �� ������� ���������� ������
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);						// ��������� �����
	g_View = XMMatrixLookAtLH(Eye, At, Up);									// ������������� ������� ����

	// �������� ����������� �����
	ConstantBufferMatrixes cbm;									// ������� ��������� ��������� � ��������� � ��� �������
	cbm.mWorld = XMMatrixTranspose(g_World);
	cbm.mView = XMMatrixTranspose(g_View);
	cbm.mProjection = XMMatrixTranspose(g_Projection);
						
	// ������ ����������� ��������

	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);				// ������������� ������� � ����������� ������
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBMatrixes);		// (������ ������ (b0 ��� b1), ... , ������ ������)
	g_pImmediateContext->PSSetConstantBuffers(1, 1, &g_pCBLight);
	
	UpdateMatrix(BGR, orbit, bkg_angle);
	g_pImmediateContext->PSSetShader(g_pPixelShaderBackground, NULL, 0);
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureSB);			// ��������� �������� (t0)
	g_pImmediateContext->DrawIndexed(6, 54, 0);								// ������ ��� (6 ������)
	

	// ������ ��� ��������� �����
	g_pImmediateContext->PSSetShader(g_pPixelShaderSolid, NULL, 0);			// ������������� ����� ���������� ������

	for (int i = 0; i < 2; i++){

		UpdateMatrix(i);													// ������������� ������� ���� ��������� �����
		g_pImmediateContext->DrawIndexed(36, 0, 0);							// ������ � ������ ������ 36 ������
	}

	if (sh_mode == NORMAL) {
	
		g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
		g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);			// ��������� �������� (t0)	
		g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);			// ��������� ������� (������) (s0)
	}
	else {
	
		g_pImmediateContext->PSSetShader(g_pPixelShaderEffect, NULL, 0);	
	}	

	if (dr_mode == PYRAMID) {
	
		UpdateMatrix(PYRAMID);												// ��������� ������� ������������ ����			
		g_pImmediateContext->DrawIndexed(18, 36, 0);							// ������ � ������ ������ 36 ������
	}
	else if (dr_mode == CONUS) {
	
		UpdateMatrix(CONUS);
		g_Mesh->Draw();
	}
	else if (dr_mode == TORUS) {
	
		UpdateMatrix(PYRAMID);
		g_pMesh->Draw();
	}

	g_pSwapChain->Present(0, 0);											// ����� �� ����� ����������� ������-�������
}


//--------------------------------------------------------------------------------------
// ������� ���������� (���� ��������� ��������)
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
	
	if (DIObject) { DIObject->Release(); }
	
	kb.Release();
	ms.Release();

	if (g_pImmediateContext) g_pImmediateContext->ClearState();      // ���������� ��������� ����������

	// �������� ����������� Direct3D � �������, �������� ��������

	if(g_pSamplerLinear) g_pSamplerLinear->Release();
	if(g_pTextureRV) g_pTextureRV->Release();

	if (g_pCBMatrixes) g_pCBMatrixes->Release();
	if (g_pCBLight) g_pCBLight->Release();
	if (g_pVertexBuffer) g_pVertexBuffer->Release();
	if (g_pIndexBuffer) g_pIndexBuffer->Release();

	if (g_pVertexLayout) g_pVertexLayout->Release();
	if (g_pVertexShader) g_pVertexShader->Release();
	if (g_pPixelShaderEffect) g_pPixelShaderEffect->Release();
	if (g_pPixelShaderBackground) g_pPixelShaderBackground->Release();
	if (g_pPixelShaderSolid) g_pPixelShaderSolid->Release();
	if (g_pPixelShader) g_pPixelShader->Release();


	if (g_pDepthStencil) g_pDepthStencil->Release();
	if (g_pDepthStencilView) g_pDepthStencilView->Release();

	if (g_pRenderTargetView) g_pRenderTargetView->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pd3dDevice) g_pd3dDevice->Release();
}


