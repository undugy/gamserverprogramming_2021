#pragma once
#include"header.h"
class SERVER_MAIN
{
public:
	SERVER_MAIN();
	~SERVER_MAIN();

private:
	HANDLE m_h_iocp;
	SOCKET m_s_socket;

public:
	static void error_display(int err_no);

};

