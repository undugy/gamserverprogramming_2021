#include "DB.h"
extern array<NPC*, MAX_USER + MAX_NPC>clients;
DB* DB::m_pInst = NULL;
DB::DB():hstmt(0)
{
	InitDB();
}

DB::~DB()
{
	SQLCancel(hstmt);
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

void DB::InitDB()
{

	
	//SQLINTEGER p_id;
	//SQLSMALLINT p_level;
	//SQLWCHAR p_Name[NAME_LEN];
	//SQLLEN cbName = 0, cbP_ID = 0, cbP_Level = 0;

	// Allocate environment handle  
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// Set the ODBC version environment attribute  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		// Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source  
				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"2017182020_TermProject_ODBC", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

				// Allocate statement handle  
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					cout << "ODBC Connected\n";
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
				}


			}
		}
	}
}

void DB::Save_Data(int c_id)
{
	cl = (Player*)clients[c_id];
	wchar_t exec[256];
	wchar_t wname[MAX_NAME_SIZE + 1];
	size_t len;
	mbstowcs_s(&len, wname, MAX_NAME_SIZE + 1, cl->name, MAX_NAME_SIZE + 1);
	wsprintf(exec, L"EXEC save_user_status @Param=N'%ls',@Param1=%d,@Param2=%d,@Param3=%d,@Param4=%d,@Param5=%d,@Param6=%d", 
		wname,cl->hp,cl->maxhp,cl->exp,cl->level,cl->x,cl->y);
	wcout << exec << endl;
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)exec, SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		cout << "저장성공\n";
	}
	else
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		SQLCancel(hstmt);
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}
}

void DB::get_login_data(int c_id,char*name)
{
	//setlocale(LC_ALL, "korean");
	cl = (Player*)clients[c_id];
	wchar_t exec[256];
	wchar_t wname[MAX_NAME_SIZE+1];
	size_t len;
	mbstowcs_s(&len, wname, MAX_NAME_SIZE + 1, name, MAX_NAME_SIZE + 1);
	wsprintf(exec, L"EXEC select_if_exist @Param=N'%ls'", wname);
	wcout << exec << endl;
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)exec, SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, p_id, MAX_NAME_SIZE + 1, &cb_id);
		retcode = SQLBindCol(hstmt, 2, SQL_C_SSHORT, &p_hp, 100, &cb_hp);
		retcode = SQLBindCol(hstmt, 3, SQL_C_SSHORT, &p_maxhp, 100, &cb_maxhp);
		retcode = SQLBindCol(hstmt, 4, SQL_C_SLONG, &p_exp, 100, &cb_exp);
		retcode = SQLBindCol(hstmt, 5, SQL_C_SSHORT, &p_level, 100, &cb_level);
		retcode = SQLBindCol(hstmt, 6, SQL_C_SSHORT, &p_x, 100, &cb_x);
		retcode = SQLBindCol(hstmt, 7, SQL_C_SSHORT, &p_y, 100, &cb_y);
		
		
		
		retcode = SQLFetch(hstmt);
		if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
				HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
				cl->x = p_x;
				cl->y = p_y;
				cl->level = p_level;
				cl->hp = p_hp;
				cl->maxhp=p_maxhp;
				strcpy_s(cl->name, name);
				printf("플레이어 초기화");
				

		}
		
	}
	else {
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
	}
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		SQLCancel(hstmt);
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}
}

void DB::HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER iError;
	WCHAR wszMessage[1000];
	WCHAR wszState[SQL_SQLSTATE_SIZE + 1];
	if (RetCode == SQL_INVALID_HANDLE) {
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5)) {
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}
