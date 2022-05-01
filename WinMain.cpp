#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN // ������ ��� ���Ͽ� �������� ������� ���� ��� ���ϵ��� ���ܽ�����.
#define WIN32_EXTRA_LEAN

#include "include/glad/glad.h"
#include <windows.h>
#include <iostream>
#include "Application.h"int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int);  // ������ ���� �Լ�.
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); // ������ ���ν��� �Լ�.

/*
	�α��� ���� �ϱ� ���� ����� ��忡���� ǥ�� Win32 â�� �α׸� ���� ���� �ܼ� â �� ���� �������� ��.
	#pragma ��ó���� �����ڸ� �̿��ؼ�, ����� ��忡���� �ܼ� ���� �ý��ۿ� �����ϰ�,
	������ ��忡���� �������� ���� �ý��ۿ� �����ϵ��� ��.
*/
#if _DEBUG
#pragma comment( linker, "/subsystem:console" )
int main(int argc, const char** argv) {
	return WinMain(GetModuleHandle(NULL), NULL,
		GetCommandLineA(), SW_SHOWDEFAULT);
}
#else
#pragma comment( linker, "/subsystem:windows" )
#endif

#pragma comment(lib, "opengl32.lib") // opengl ���̺귯�� ����

#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int*);

typedef const char*(WINAPI* PFNWGLGETEXTENSIONSSTRINGEXTPROC) (void);
typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC) (int);
typedef int (WINAPI* PFNWGLGETSWAPINTERVALEXTPROC) (void);

Application* gApplication = 0;
GLuint gVertexArrayObject = 0;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	// ���ø����̼� �ν��Ͻ� ����.
	gApplication = new Application();

	// ������ Ŭ���� ���� �� ���.
	WNDCLASSEX wndclass;
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wndclass.lpszMenuName = 0;
	wndclass.lpszClassName = L"Win32 Game Window";
	RegisterClassEx(&wndclass);

	// ������ ������ ���.
	int screenWidth = GetSystemMetrics(SM_CXSCREEN); // â�� ������ ������� ����� ������ ȭ���� �ʺ�� ���̸� ����.
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int clientWidth = 800;
	int clientHeight = 600;

	RECT windowRect;
	SetRect(&windowRect,
		(screenWidth / 2) - (clientWidth / 2),
		(screenHeight / 2) - (clientHeight / 2),
		(screenWidth / 2) + (clientWidth / 2),
		(screenHeight / 2) + (clientHeight / 2));

	// ������ ��Ÿ�� ����.
	DWORD style = (WS_OVERLAPPED | WS_CAPTION |
		WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
	// | WS_THICKFRAME to resize

	// ������ ũ�� ���� �� ������ ����.
	AdjustWindowRectEx(&windowRect, style, FALSE, 0);
	HWND hwnd = CreateWindowEx(0, wndclass.lpszClassName, L"Game Window", style,
		windowRect.left, windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top, NULL, NULL,
		hInstance, szCmdLine);
	HDC hdc = GetDC(hwnd);

	// �ùٸ� �ȼ� ���� �����ϰ�, �������� ����̽� ���ؽ�Ʈ�� ����.
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW
		| PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 32;
	pfd.cStencilBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;
	int pixelFormat = ChoosePixelFormat(hdc, &pfd);
	SetPixelFormat(hdc, pixelFormat, &pfd);

	// �ӽ� OpenGL Context�� �����ϰ�, �ֽ� ���ؽ�Ʈ ������ ���� wglCreateContextAttribsARB�� ���� �����͸� ����.
	HGLRC tempRC = wglCreateContext(hdc);
	wglMakeCurrent(hdc, tempRC);
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

	// wglCreateContextAttribsARB �Լ��� ȣ���Ͽ� OpenGL 3.3 Core context ����.
	const int attribList[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
		WGL_CONTEXT_FLAGS_ARB, 0,
		WGL_CONTEXT_PROFILE_MASK_ARB,
		WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0, };
	HGLRC hglrc = wglCreateContextAttribsARB(hdc, 0, attribList);

	// ���� OpenGL Context�� �и��ϰ� �� Context�� ��ü.
	wglMakeCurrent(NULL, NULL); // �����忡�� ���� ������ ���ؽ�Ʈ�� �и�.
	wglDeleteContext(tempRC);   // ������ ���� �ӽ� OpenGL Context�� ����.
	wglMakeCurrent(hdc, hglrc); // ��� ������� Context�� ������ ���ؽ�Ʈ�� �������.

	// glad ���� OpenGL 3.3 Core �Լ����� �ҷ���.
	if (!gladLoadGL()) {
		std::cout << "Could not initialize GLAD\n";
	}
	else {
		std::cout << "OpenGL Version " <<
			GLVersion.major << "." << GLVersion.minor <<
			"\n";
	}

	// wglGetExtensionStringEXT�� ���� WGL_EXT_swap_control ���� ���θ� ����.
	PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
	bool swapControlSupported = strstr(_wglGetExtensionsStringEXT(),"WGL_EXT_swap_control") != 0;

	// WGL_EXT_swap_control �ҷ�����.
	int vsynch = 0;
	if (swapControlSupported)
	{
		PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
		PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
		if (wglSwapIntervalEXT(1))
		{
			std::cout << "Enabled vsynch\n";
			vsynch = wglGetSwapIntervalEXT();
		}
		else
		{
			std::cout << "Could not enable vsynch\n";
		}
	}
	else 
	{ // !swapControlSupported
		std::cout << "WGL_EXT_swap_control not supported\n";
	}

	// VAO�� �����ϰ�, ���ε� ����.
	glGenVertexArrays(1, &gVertexArrayObject);
	glBindVertexArray(gVertexArrayObject);

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);
	gApplication->Initialize();

	DWORD lastTick = GetTickCount();
	MSG msg;
	while (true) 
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
		{
			if (msg.message == WM_QUIT) 
			{
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// ������ �ð� ���ϱ�
		DWORD thisTick = GetTickCount();
		float deltaTime = float(thisTick - lastTick) * 0.001f;
		lastTick = thisTick;
		
		// application ������Ʈ
		if (gApplication != 0) 
		{
			gApplication->Update(deltaTime);
		}
		
		// OpenGL�� ���� ������
		if (gApplication != 0) 
		{
			RECT clientRect;
			GetClientRect(hwnd, &clientRect);
			clientWidth = clientRect.right - clientRect.left;
			clientHeight = clientRect.bottom - clientRect.top;
			glViewport(0, 0, clientWidth, clientHeight);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glPointSize(5.0f);
			glBindVertexArray(gVertexArrayObject);

			glClearColor(0.5f, 0.6f, 0.7f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			float aspect = (float)clientWidth / (float)clientHeight;
			gApplication->Render(aspect);
		}
		
		// �� ���� ����
		if (gApplication != 0) 
		{
			SwapBuffers(hdc);
			if (vsynch != 0) 
			{
				glFinish();
			}
		}
	} // End of game loop

	// ����
	if (gApplication != 0) {
		std::cout << "Expected application to be null on exit\n";
		delete gApplication;
	}
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_CLOSE:
		// application�� �ݰ�, destroy �޽����� ��������, �����츦 ������.
		if (gApplication != 0)
		{
			gApplication->Shutdown();
			gApplication = 0;
			DestroyWindow(hwnd);
		}
		else
		{
			std::cout << "Already shut down!\n";
		}
		break;
	case WM_DESTROY:
		// destroy �޽����� ������, �������� OpenGL �ڿ����� ������.
		if (gVertexArrayObject != 0)
		{
			HDC hdc = GetDC(hwnd);
			HGLRC hglrc = wglGetCurrentContext();

			// VAO�� ����.
			glBindVertexArray(0);
			glDeleteVertexArrays(1, &gVertexArrayObject);
			gVertexArrayObject = 0;

			// OpenGL ���ؽ�Ʈ�� ����
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(hglrc);
			ReleaseDC(hwnd, hdc);

			PostQuitMessage(0);
		}
		else
		{
			std::cout << "Got multiple destroy messages\n";
		}
		break;
	case WM_PAINT:
	case WM_ERASEBKGND:
		return 0;
	}

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}