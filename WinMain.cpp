#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN // 윈도우 헤더 파일에 딸려오는 사용하지 않을 헤더 파일들을 제외시켜줌.
#define WIN32_EXTRA_LEAN

#include "include/glad/glad.h"
#include <windows.h>
#include <iostream>
#include "Application.h"int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int);  // 윈도우 진입 함수.
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); // 윈도우 프로시져 함수.

/*
	로깅을 쉽게 하기 위해 디버그 모드에서는 표준 Win32 창과 로그를 보기 위한 콘솔 창 두 개가 열리도록 함.
	#pragma 전처리기 지시자를 이용해서, 디버그 모드에서는 콘솔 서브 시스템에 연결하고,
	릴리즈 모드에서는 윈도우즈 서브 시스템에 연결하도록 함.
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

#pragma comment(lib, "opengl32.lib") // opengl 라이브러리 연결

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
	// 애플리케이션 인스턴스 생성.
	gApplication = new Application();

	// 윈도우 클래스 생성 및 등록.
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

	// 윈도우 사이즈 계산.
	int screenWidth = GetSystemMetrics(SM_CXSCREEN); // 창이 열리면 모니터의 가운데에 오도록 화면의 너비와 높이를 구함.
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int clientWidth = 800;
	int clientHeight = 600;

	RECT windowRect;
	SetRect(&windowRect,
		(screenWidth / 2) - (clientWidth / 2),
		(screenHeight / 2) - (clientHeight / 2),
		(screenWidth / 2) + (clientWidth / 2),
		(screenHeight / 2) + (clientHeight / 2));

	// 윈도우 스타일 설정.
	DWORD style = (WS_OVERLAPPED | WS_CAPTION |
		WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
	// | WS_THICKFRAME to resize

	// 윈도우 크기 설정 및 윈도우 생성.
	AdjustWindowRectEx(&windowRect, style, FALSE, 0);
	HWND hwnd = CreateWindowEx(0, wndclass.lpszClassName, L"Game Window", style,
		windowRect.left, windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top, NULL, NULL,
		hInstance, szCmdLine);
	HDC hdc = GetDC(hwnd);

	// 올바른 픽셀 포맷 정의하고, 윈도우의 디바이스 컨텍스트에 적용.
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

	// 임시 OpenGL Context를 생성하고, 최신 컨텍스트 생성을 위한 wglCreateContextAttribsARB에 대한 포인터를 얻어옴.
	HGLRC tempRC = wglCreateContext(hdc);
	wglMakeCurrent(hdc, tempRC);
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

	// wglCreateContextAttribsARB 함수를 호출하여 OpenGL 3.3 Core context 생성.
	const int attribList[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
		WGL_CONTEXT_FLAGS_ARB, 0,
		WGL_CONTEXT_PROFILE_MASK_ARB,
		WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0, };
	HGLRC hglrc = wglCreateContextAttribsARB(hdc, 0, attribList);

	// 기존 OpenGL Context를 분리하고 새 Context로 교체.
	wglMakeCurrent(NULL, NULL); // 스레드에서 현재 렌더링 컨텍스트를 분리.
	wglDeleteContext(tempRC);   // 이전에 만든 임시 OpenGL Context를 제거.
	wglMakeCurrent(hdc, hglrc); // 방금 만들어준 Context를 렌더링 컨텍스트로 만들어줌.

	// glad 통해 OpenGL 3.3 Core 함수들을 불러옴.
	if (!gladLoadGL()) {
		std::cout << "Could not initialize GLAD\n";
	}
	else {
		std::cout << "OpenGL Version " <<
			GLVersion.major << "." << GLVersion.minor <<
			"\n";
	}

	// wglGetExtensionStringEXT를 통해 WGL_EXT_swap_control 지원 여부를 쿼리.
	PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
	bool swapControlSupported = strstr(_wglGetExtensionsStringEXT(),"WGL_EXT_swap_control") != 0;

	// WGL_EXT_swap_control 불러오기.
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

	// VAO를 생성하고, 바인딩 해줌.
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
		// 프레임 시간 구하기
		DWORD thisTick = GetTickCount();
		float deltaTime = float(thisTick - lastTick) * 0.001f;
		lastTick = thisTick;
		
		// application 업데이트
		if (gApplication != 0) 
		{
			gApplication->Update(deltaTime);
		}
		
		// OpenGL을 통한 렌더링
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
		
		// 백 버퍼 스왑
		if (gApplication != 0) 
		{
			SwapBuffers(hdc);
			if (vsynch != 0) 
			{
				glFinish();
			}
		}
	} // End of game loop

	// 종료
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
		// application을 닫고, destroy 메시지를 내보내고, 윈도우를 제거함.
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
		// destroy 메시지를 받으면, 윈도우의 OpenGL 자원들을 해제함.
		if (gVertexArrayObject != 0)
		{
			HDC hdc = GetDC(hwnd);
			HGLRC hglrc = wglGetCurrentContext();

			// VAO를 삭제.
			glBindVertexArray(0);
			glDeleteVertexArrays(1, &gVertexArrayObject);
			gVertexArrayObject = 0;

			// OpenGL 컨텍스트를 제거
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