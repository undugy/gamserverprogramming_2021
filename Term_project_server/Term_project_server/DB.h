#pragma once
#include"header.h"
#include<tchar.h>
#include"Player.h"
class DB
{
private:
	static DB* m_pInst;
public:
	DB();
	~DB();
public:
	static DB* GetInst()
	{
		if (!m_pInst)
			m_pInst = new DB;
		return m_pInst;
	}

	static void DestroyInst()
	{
		if (m_pInst)
		{
			delete m_pInst;
			m_pInst = NULL;
		}
	}
private:
	 SQLHENV henv;
	 SQLHDBC hdbc;
	 
	 SQLWCHAR p_id[MAX_NAME_SIZE + 1];
	 SQLSMALLINT p_x, p_y, p_level, p_hp, p_maxhp;
	 SQLINTEGER  p_exp;
	 SQLLEN cb_x, cb_y, cb_id, cb_level, cb_hp, cb_maxhp, cb_exp;
	 Player* cl = NULL;
public:
	 mutex db_l;
	 
private:
	SQLRETURN retcode;
	void InitDB();
	void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);
public:
	void Save_Data(int c_id);
	void get_login_data(int c_id,char*name);
	
};

