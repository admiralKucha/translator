#pragma once
#include <string>
#include <memory>
#include "StringTable.h"

class SymbolTable;

class Operand 
{
public:
	Operand() = default;
	virtual std::string toString() const = 0;
};

class Rvalue : public Operand 
{
public:
	Rvalue() = default;
};

class StringOperand : public Operand
{
public:
	StringOperand(int index, const StringTable* stringTable) : _index{ index }, _stringTable{ stringTable } {};
	std::string toString() const;
protected:
	int _index;
	const StringTable* _stringTable;
};

class LabelOperand : public Operand 
{
public:
	LabelOperand(int LabelID) : _LabelID{ LabelID } {};
	std::string toString() const;
protected:
	int _LabelID;
};

class MemoryOperand : public Rvalue 
{
public:
	MemoryOperand(int index, const SymbolTable* symbolTable) : _index{ index }, _symbolTable{ symbolTable }  {};
	std::string toString() const;
	int index() const;
protected:
	int _index;
	const SymbolTable* _symbolTable;
};

class NumberOperand : public Rvalue 
{
public:
	NumberOperand(int value) : _value{ value } {};
	std::string toString() const;
protected:
	int _value;
};


class Atom 
{
public:
	virtual std::string toString() const = 0;
};

class Comment : public Atom
{
protected:
	std::string _com;
	std::string _tab;
public:
	Comment(std::string com, std::string tab) : _com{ com }, _tab{ tab }{};
	std::string toString() const;
};

class UnaryOpAtom : public Atom 
{
protected:
	std::string _name;
	std::string _tab;
	std::shared_ptr<Rvalue> _operand;
	std::shared_ptr<MemoryOperand> _result;

public:
	UnaryOpAtom(const std::string& name, std::shared_ptr<Rvalue> operand, std::shared_ptr<MemoryOperand> result, std::string tab) : _name{ name }, _operand{ operand }, _result{ result }, _tab{ tab } {};
	std::string toString() const;
};

class BinaryOpAtom : public Atom 
{
protected:
	std::string _name;
	std::shared_ptr<Rvalue> _left;
	std::shared_ptr<Rvalue> _right;
	std::shared_ptr<MemoryOperand> _result;
	std::string _tab;
public:
	BinaryOpAtom(const std::string& name, std::shared_ptr<Rvalue> left, std::shared_ptr<Rvalue> right, std::shared_ptr<MemoryOperand> result, std::string tab) : _name{ name }, _left{ left }, _right{ right }, _result{ result }, _tab{ tab } {};
	std::string toString() const;
};

class OutAtom : public Atom 
{
public:
	OutAtom(std::shared_ptr<Operand> value, std::string tab) : _value{ value }, _tab{ tab } {};
	std::string toString() const;
protected:
	std::shared_ptr<Operand> _value;
	std::string _tab;
};

class InAtom : public Atom 
{
public:
	InAtom(std::shared_ptr<MemoryOperand> result, std::string tab) : _result{ result }, _tab{ tab } {};
	std::string toString() const;
protected:
	std::string _tab;
	std::shared_ptr<MemoryOperand> _result;
};

class ConditonalJumpAtom : public Atom 
{
public:
	ConditonalJumpAtom(const std::string& condition, std::shared_ptr<Rvalue> left, std::shared_ptr<Rvalue> right, std::shared_ptr<LabelOperand> label, std::string tab) : _condition{ condition }, _left{ left }, _right{ right }, _label{ label }, _tab{ tab } {};
	std::string toString() const;
protected:
	std::string _condition;
	std::shared_ptr<Rvalue> _left;
	std::shared_ptr<Rvalue> _right;
	std::shared_ptr<LabelOperand> _label;
	std::string _tab;
};

class LabelAtom : public Atom
{
public:
	LabelAtom(std::shared_ptr<LabelOperand> label, std::string tab) : _label{ label }, _tab{ tab } {};
	std::string toString() const;
protected:
	std::shared_ptr<LabelOperand> _label;
	std::string _tab;
};

class JumpAtom : public Atom 
{
public:
	JumpAtom(std::shared_ptr<LabelOperand> label, std::string tab) : _label{ label }, _tab{ tab } {};
	std::string toString() const;
protected:
	std::shared_ptr<LabelOperand> _label;
	std::string _tab;
};

class CallAtom : public Atom 
{
public:
	CallAtom(std::shared_ptr<MemoryOperand> fucntionToCall, std::shared_ptr<MemoryOperand> result, std::string tab) : _fucntionToCall{ fucntionToCall }, _result{ result }, _tab{ tab } {};
	std::string toString() const;

protected:
	std::shared_ptr<MemoryOperand> _fucntionToCall;
	std::shared_ptr<MemoryOperand> _result;
	std::string _tab;
};

class RetAtom : public Atom 
{
public:
	RetAtom(std::shared_ptr<Rvalue> result, std::string tab) : _result{ result }, _tab{ tab } {};
	std::string toString() const;

protected:
	std::shared_ptr<Rvalue> _result;
	std::string _tab;
};

class ParamAtom : public Atom 
{
public:
	ParamAtom(std::shared_ptr<Rvalue> parametr, std::string tab) : _parametr{ parametr }, _tab{ tab } {};
	std::string toString() const;

protected:
	std::shared_ptr<Rvalue> _parametr;
	std::string _tab;
};
