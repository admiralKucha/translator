#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <map>
#include <string>

#include "Scanner.h"
#include "Atoms.h"
#include "StringTable.h"
#include "SymbolTable.h"


class Parser
{
private:
	std::map<Scope, std::vector<std::unique_ptr<Atom>>> _atoms;
	StringTable _stringTable;
	SymbolTable _symbolTable;
	Scanner _scanner;
	Token _currentToken;
	int _currentLabel;
	std::string _tab = "";
	std::shared_ptr<NumberOperand> _one;
	std::shared_ptr<NumberOperand> _zero;
	std::map<LexemType, std::string> _jumps;

	void lexCheck();
	std::shared_ptr<LabelOperand> newLabel();

	std::shared_ptr<Rvalue> E(Scope scope);
	std::shared_ptr<Rvalue> E1(Scope scope);
	std::shared_ptr<Rvalue> E1_(Scope scope, std::string name);
	std::shared_ptr<Rvalue> E2(Scope scope);
	std::shared_ptr<Rvalue> E3(Scope scope);
	std::shared_ptr<Rvalue> E3_(std::shared_ptr<Rvalue> p, Scope scope);
	std::shared_ptr<Rvalue> E4(Scope scope);
	std::shared_ptr<Rvalue> E4_(std::shared_ptr<Rvalue> p, Scope scope);
	std::shared_ptr<Rvalue> E5(Scope scope);
	std::shared_ptr<Rvalue> E5_(std::shared_ptr<Rvalue> p, Scope scope);
	std::shared_ptr<Rvalue> E6(Scope scope);
	std::shared_ptr<Rvalue> E6_(std::shared_ptr<Rvalue> p, Scope scope);
	std::shared_ptr<Rvalue> E7(Scope scope);
	std::shared_ptr<Rvalue> E7_(std::shared_ptr<Rvalue> p, Scope scope);
	int ArgList(Scope scope);
	int ArgList_(Scope scope);

	void DeclareStmt(Scope scope);
	void DeclareStmt_(SymbolTable::TableRecord::RecordType type, std::string name, Scope scope);
	void DeclVarList_(SymbolTable::TableRecord::RecordType type, Scope scope);
	void InitVar(SymbolTable::TableRecord::RecordType type, std::string name, Scope scope);
	int ParamList(Scope scope);
	int ParamList_(Scope scope);
	void Stmt(Scope scope);

	void AssignOrCallOp(Scope scope);
	void AssignOrCall(Scope scope);
	void AssignOrCall_(std::string name, Scope scope);
	void WhileOp(Scope scope);
	void ForOp(Scope scope);
	void ForInit(Scope scope);
	std::shared_ptr<Rvalue> ForExp(Scope scope);
	void ForLoop(Scope scope);
	void IfOp(Scope scope);
	void ElsePart(Scope scope);
	void SwitchOp(Scope scope);
	void Cases(std::shared_ptr<Rvalue> p, std::shared_ptr<LabelOperand> end, Scope scope);
	void Cases_(std::shared_ptr<Rvalue> p, std::shared_ptr<LabelOperand> end, std::shared_ptr<LabelOperand> def, Scope scope);
	std::shared_ptr<LabelOperand> Acase(std::shared_ptr<Rvalue> p, std::shared_ptr<LabelOperand> end, Scope scope);
	void IOp(Scope scope);
	void OOp(Scope scope);
	void OOp_(Scope scope);

public:
	Parser(std::istream& stream);
	void printAtoms(std::ostream& stream);
	void generateAtom(std::unique_ptr<Atom> atom, Scope scope);
	void syntaxError(const std::string& message);
	void lexicalError(const std::string& message);
	void printSymbolTable(std::ostream& stream);
	void checkMain();

	void StmtList(Scope scope);
};
