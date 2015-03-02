#pragma comment(lib, "ws2_32.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <Windows.h>

#include <iostream>

#define CLASS_NAME "EchoServer"
#define WM_NETWORK (WM_USER + 1)
#define BUF_SIZE 1024
#define PORT 9001

LRESULT CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void OnAccept(SOCKET sock, HWND hWnd);
void OnRead(SOCKET sock);
void OnClose(SOCKET sock);

HWND InitWindow(HINSTANCE hInstnace);
void CleanupWindow();
SOCKET InitServerSocket(HWND hWnd, unsigned short port);
void CleanupServerSocket(SOCKET serverSock);

int WINAPI WinMain(HINSTANCE hInstnace, HINSTANCE, LPSTR lpCmdLine, int nShowCmd)
{
	HWND hWnd = InitWindow(hInstnace);
	if (!hWnd)
		return -1;

	SOCKET serverSock = InitServerSocket(hWnd, PORT);
	if (serverSock == INVALID_SOCKET)
	{
		CleanupWindow();
		return -1;
	}

	printf("Echo Server ON\n");

	MSG msg;
	while (GetMessage(&msg, hWnd, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CleanupServerSocket(serverSock);
	CleanupWindow();

	return 0;
}

//
//

LRESULT CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_NETWORK:
	{
		SOCKET sock = wParam;
		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_ACCEPT:
			OnAccept(sock, hWnd);
			break;
		case FD_READ:
			OnRead(sock);
			break;
		case FD_CLOSE:
			OnClose(sock);
			break;
		}
		return 0;
	}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void OnAccept(SOCKET sock, HWND hWnd)
{
	sockaddr_in addr = { 0, };
	int addrSize = sizeof(addr);
	SOCKET clientSock = accept(sock, (sockaddr*)&addr, &addrSize);
	if (clientSock == INVALID_SOCKET)
	{
		int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK)
		{
			printf("Accept Error %d\n", error);
			return;
		}

	}
	if (WSAAsyncSelect(clientSock, hWnd, WM_NETWORK, FD_READ | FD_CLOSE) != NO_ERROR)
	{
		printf("WSAAsyncSelect on client socket Error\n");
		closesocket(clientSock);
	}
}
void OnRead(SOCKET sock)
{
	char buf[BUF_SIZE];
	int recvSize = recv(sock, buf, _countof(buf), 0);
	if (recvSize == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK)
		{
			printf("Recv Error %d\n", error);
			closesocket(sock);
			return;
		}
	}
	if (send(sock, buf, recvSize, 0) == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK)
		{
			printf("Send Error %d\n", error);
			closesocket(sock);
			return;
		}
	}
}

void OnClose(SOCKET sock)
{
	closesocket(sock);
}


//
//

HWND InitWindow(HINSTANCE hInstnace)
{
	if (!AllocConsole())
		return NULL;
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);

	WNDCLASS wc = {};
	wc.hInstance = hInstnace;
	wc.lpszClassName = CLASS_NAME;
	wc.lpfnWndProc = &WinProc;
	RegisterClass(&wc);

	HWND hWnd = CreateWindowEx(
		0,
		CLASS_NAME,
		"Server",
		0,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL,
		NULL,
		hInstnace,
		NULL
		);

	return hWnd;
}

void CleanupWindow()
{
	FreeConsole();
}

SOCKET InitServerSocket(HWND hWnd, unsigned short port)
{
	WSAData wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != NO_ERROR)
	{
		printf("WSAStartup Error\n");
		return INVALID_SOCKET;
	}
	
	SOCKET serverSock = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSock == INVALID_SOCKET)
	{
		printf("Socket Error\n");
		return INVALID_SOCKET;
	}

	sockaddr_in addr = { 0, };
	addr.sin_family = PF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = ADDR_ANY;
	if (bind(serverSock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		printf("Bind Error\n");
		closesocket(serverSock);
		return INVALID_SOCKET;
	}

	if (listen(serverSock, SOMAXCONN) == SOCKET_ERROR)
	{
		printf("Listen Error\n");
		closesocket(serverSock);
		return INVALID_SOCKET;
	}

	if (WSAAsyncSelect(serverSock, hWnd, WM_NETWORK, FD_ACCEPT) != NO_ERROR)
	{
		printf("WSAAsyncSelect Error\n");
		closesocket(serverSock);
		return INVALID_SOCKET;
	}

	return serverSock;
}

void CleanupServerSocket(SOCKET serverSock)
{
	closesocket(serverSock);
	WSACleanup();
}
