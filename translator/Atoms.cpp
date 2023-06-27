#include <sstream>
#include "SymbolTable.h"
#include "Atoms.h"
std::string Comment::toString() const
{
	std::ostringstream iss;
	iss << _tab << _com;
	return iss.str();
}

std::string LabelOperand::toString() const 
{
	std::ostringstream iss;
	iss  << "l" << _LabelID;
	return iss.str();
}

std::string MemoryOperand::toString() const 
{
	std::ostringstream iss;
	if ((*_symbolTable)[_index]._name.size() == 0) {
		iss << "tmp" << _index;
	}
	else {
		iss << (*_symbolTable)[_index]._name;
	}
	return iss.str();
}

int MemoryOperand::index() const {return _index;}

std::string StringOperand::toString() const 
{
	return (*_stringTable)[_index];
}

std::string NumberOperand::toString() const 
{
	std::ostringstream iss;
	iss << "'" << _value << "'";
	return iss.str();
}

std::string UnaryOpAtom::toString() const
{
	std::ostringstream oss;
	oss << _tab << "(" << _name << ',' << _operand->toString() << ",," << _result->toString() << ')';
	return oss.str();
}

std::string BinaryOpAtom::toString() const 
{
	std::ostringstream oss;
	oss << _tab << "(" << _name << ',' << _left->toString() << "," << _right->toString() << "," << _result->toString() << ")";
	return oss.str();
}


std::string OutAtom::toString() const 
{
	std::ostringstream oss;
	oss<< _tab << "(OUT,,," << _value->toString() << ')';
	return oss.str();
}

std::string InAtom::toString() const 
{
	std::ostringstream oss;
	oss<< _tab << "(IN,,," << _result->toString() << ')';
	return oss.str();
}

std::string ConditonalJumpAtom::toString() const
{
	std::ostringstream oss;
	oss<< _tab << "(" << _condition << ',' << _left->toString() << "," << _right->toString() << "," << _label->toString() << ")";
	return oss.str();
}

std::string LabelAtom::toString() const
{
	std::ostringstream oss;
	oss<<_tab << "(LBL,,," << _label->toString() << ')';
	return oss.str();
}

std::string JumpAtom::toString() const
{
	std::ostringstream oss;
	oss<<_tab << "(JMP,,," << _label->toString() << ')';
	return oss.str();
}

std::string CallAtom::toString() const 
{
	std::ostringstream oss;
	oss<<_tab << "(CALL," << _fucntionToCall->toString() << ",," << _result->toString() << ')';
	return oss.str();
}

std::string RetAtom::toString() const 
{
	std::ostringstream oss;
	oss<<_tab << "(RET,,," << _result->toString() << ')';
	return oss.str();
}

std::string ParamAtom::toString() const 
{
	std::ostringstream oss;
	oss<<_tab << "(PARAM,,," << _parametr->toString() << ')';
	return oss.str();
}