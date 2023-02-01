#pragma once

#include "..\sdk\misc\Recv.hpp"
#include <unordered_map>

class netg_cfg
{
public:
	std::unordered_map<std::string, RecvTable*> tables;

	int get_offset(const char *tableName, const char *propName); 
	int get_prop(const char *tableName, const char *propName, RecvProp **prop = 0);
	int get_prop(RecvTable *recvTable, const char *propName, RecvProp **prop = 0);

	static netg_cfg& get()
	{
		static netg_cfg instance;
		return instance;
	}

private:
	RecvTable *get_table(const char *tableName);
};