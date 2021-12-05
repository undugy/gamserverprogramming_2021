#pragma once
#include"header.h"
class DB
{
public:
	DB();
	~DB();
public:
	 SQLHENV henv;
	 SQLHDBC hdbc;
	 SQLHSTMT hstmt = 0;
	 SQLLEN cb_id = 0, cb_hp = 0,cb_maxhp, cb_level = 0,cb_exp;
private:
	SQLRETURN retcode;
	void InitDB();
	void Save_Data();
	void get_login_data();
	void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);
};

