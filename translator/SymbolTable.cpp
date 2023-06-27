#include "SymbolTable.h"
#include <iomanip>
#include <algorithm>

typedef SymbolTable::TableRecord TSrec;

const TSrec& SymbolTable::operator[] (const int index)  const
{
	if (index < 0 || index >= _records.size()) {throw "error";}
	return _records[index];
}

void SymbolTable::chFuncLen(const int index, int newLen) {_records[index]._len = newLen;}

std::shared_ptr<MemoryOperand> SymbolTable::add(const std::string& name) 
{
	auto is_ok = [&name](TSrec& _s) {return _s._name == name; };
	auto result = std::find_if(_records.begin(), _records.end(), is_ok);

	if (result != _records.end()) {return std::make_shared<MemoryOperand>(result - _records.begin(), this);}
	_records.push_back(TSrec(name));
	return std::make_shared<MemoryOperand>(_records.size() - 1, this);
}

std::shared_ptr<MemoryOperand> SymbolTable::alloc(Scope scope) 
{
	_records.push_back(TSrec("", scope, TSrec::RecordKind::var, TSrec::RecordType::integer));
	return std::make_shared<MemoryOperand>(_records.size() - 1, this);
}

std::shared_ptr<MemoryOperand> SymbolTable::addVar(const std::string& name, const Scope scope, const TableRecord::RecordType type, const int init) 
{
	auto is_ok = [&name, scope](TSrec& _s) {return _s._name == name && _s._scope == scope; };
	auto result = std::find_if(_records.begin(), _records.end(), is_ok);

	if (result != _records.end()) {		return nullptr;}

	auto newRecord = TSrec(name, scope, TSrec::RecordKind::var, type, 0, init);
	_records.push_back(newRecord);
	return std::make_shared<MemoryOperand>(_records.size() - 1, this);
}

std::shared_ptr<MemoryOperand> SymbolTable::addFunc(const std::string& name, const TableRecord::RecordType type, const int len) 
{
	auto is_ok = [&name](TSrec& _s) { return _s._name == name && _s._scope == GlobalScope; };
	auto result = std::find_if(_records.begin(), _records.end(), is_ok);

	if (result != _records.end()) {return nullptr;}

	auto newRecord = TSrec(name, GlobalScope, TSrec::RecordKind::func, type, len);
	_records.push_back(newRecord);
	return std::make_shared<MemoryOperand>(_records.size() - 1, this);
}


std::shared_ptr<MemoryOperand> SymbolTable::checkVar(const Scope scope, const std::string& name) {
	auto is_ok = [&name, scope](TSrec& _s) {return _s._name == name && _s._scope == scope; };
	auto result = std::find_if(_records.begin(), _records.end(), is_ok);

	if (result == _records.end() && scope != GlobalScope)
	{
		auto is_ok = [&name](TSrec& _s) {return _s._name == name && _s._scope == GlobalScope; };
		result = std::find_if(_records.begin(), _records.end(), is_ok);
	}

	if (result == _records.end() || result->_kind != TSrec::RecordKind::var) {return nullptr;}

	return std::make_shared<MemoryOperand>(result - _records.begin(), this);
}

std::shared_ptr<MemoryOperand> SymbolTable::checkFunc(const std::string& name, int len)
{
	auto is_ok = [&name](TSrec& _s) {return _s._name == name && _s._scope == GlobalScope; };
	auto result = std::find_if(_records.begin(), _records.end(), is_ok);

	if (result == _records.end() || result->_kind != TSrec::RecordKind::func || result->_len != len) {return nullptr;}

	return std::make_shared<MemoryOperand>(result - _records.begin(), this);
}

std::shared_ptr<MemoryOperand> SymbolTable::checkMain()
{
	auto is_ok = [](TSrec& _s) {return _s._name == "main" && _s._scope == GlobalScope; };
	auto result = std::find_if(_records.begin(), _records.end(), is_ok);

	if (result == _records.end() || result->_kind != TSrec::RecordKind::func) { return nullptr; }

	return std::make_shared<MemoryOperand>(result - _records.begin(), this);
}

std::ostream& operator<<(std::ostream& stream, const SymbolTable& symbolTable) {
	stream << "Symbol Table\n";
	int line = 0;
	stream << "code      name      kind      type      len       init      scope     offset" << std::endl;
	for (auto& item : symbolTable._records) {
		stream << std::setiosflags(std::ios::left) << std::setw(10) << line++ << std::setw(10);
		if (item._name.length() == 0) 
		{
			stream << "[tmp" + std::to_string((line - 1)) + "]";
		}
		else 
		{
			stream << item._name;
		}
		int kind = static_cast<int>(item._kind);
		stream << std::setw(10);
		switch (kind)
		{
		case 0: stream << "unknown"; break;
		case 1: stream << "var"; break;
		case 2: stream << "func"; break;
		}
		int type = static_cast<int>(item._type);
		stream << std::setw(10);
		switch (type)
		{
		case 0: stream << "unknown"; break;
		case 1: stream << "integer"; break;
		case 2: stream << "chr"; break;
		}
		stream << std::setw(10);
		if (kind == 2) {stream << item._len;}
		else {stream << "None";}

		stream << std::setw(10) << item._init << std::setw(10) << item._scope << std::setw(10) << item._offset << std::endl;
	}
	return stream;
}