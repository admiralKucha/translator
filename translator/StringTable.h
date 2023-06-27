#pragma once
#include <string>
#include <vector>

class StringTable 
{
public:
	const std::string& operator [](const int index) const;
	int add(const std::string name);
private:
	std::vector<std::string> _strings;
};