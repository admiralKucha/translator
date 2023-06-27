#pragma once
#include <string>
#include <vector>
#include "Atoms.h"

typedef int Scope;

const Scope GlobalScope = -1;

class SymbolTable 
{
public:
	struct TableRecord 
	{
	public:
		enum class RecordType { unknown, integer, chr };
		enum class RecordKind { unknown, var, func };

		TableRecord(const std::string name, Scope scope = GlobalScope, RecordKind kind = RecordKind::unknown, RecordType type = RecordType::unknown, int len = 0, int init = 0, int offset = -1) : _name{ name }, _kind{ kind }, _type{ type }, _len{ len }, _init{ init }, _scope{ scope }, _offset{ offset } {};
		std::string _name;
		RecordKind _kind;
		RecordType _type;
		int _len;
		int _init;
		Scope _scope;
		int _offset;
	};
	const TableRecord& operator [](const int index) const;
	void chFuncLen(const int index, int newLen);
	std::shared_ptr<MemoryOperand> add(const std::string& name);

	std::shared_ptr<MemoryOperand> addVar(const std::string& name, const Scope scope, const TableRecord::RecordType type, const int init = 0);
	std::shared_ptr<MemoryOperand> addFunc(const std::string& name, const TableRecord::RecordType type, const int len);
	std::shared_ptr<MemoryOperand> checkVar(const Scope scope, const std::string& name);
	std::shared_ptr<MemoryOperand> checkFunc(const std::string& name, int len);
	std::shared_ptr<MemoryOperand> checkMain();

	std::shared_ptr<MemoryOperand> alloc(Scope scope);
	friend std::ostream& operator<<(std::ostream& stream, const SymbolTable& sybmolTable);

protected:
	std::vector<TableRecord> _records;
};
