#include "StringTable.h"
#include <algorithm>
#include <stdexcept>

const std::string& StringTable::operator [](const int index) const
{
	if (index < 0 || index >= _strings.size()) {throw "string table error";}
	return _strings[index];
}

int StringTable::add(const std::string name)
{
	auto ptr = find(_strings.begin(), _strings.end(), name);
	if (ptr != _strings.end()) { return ptr - _strings.begin(); }
	_strings.push_back(name);
	return _strings.size() - 1;
}