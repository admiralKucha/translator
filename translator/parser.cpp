#include "parser.h"

Parser::Parser(std::istream& stream) : _currentLabel{ 1 }, _scanner{ stream }, _currentToken{ _currentToken = _scanner.getNextToken() }, _zero{ std::make_shared<NumberOperand>(0) }, _one{ std::make_shared<NumberOperand>(1) },
_jumps{
	{LexemType::opeq, "EQ"},
	{LexemType::opne, "NE"},
	{LexemType::oplt, "LT"},
	{LexemType::opgt, "GT"},
	{LexemType::ople, "LE" },
} 
{};

void Parser::printAtoms(std::ostream& stream) 
{
	for (auto& atoms : _atoms) 
	{
		for (auto& atom : atoms.second)
		{
			stream << atom->toString() << std::endl;
		}
	}
};

void Parser::printSymbolTable(std::ostream& stream){ stream << _symbolTable;}

void Parser::generateAtom(std::unique_ptr<Atom> atom, Scope scope){ _atoms[scope].push_back(std::move(atom));}

std::shared_ptr<LabelOperand> Parser::newLabel(){ return std::make_shared<LabelOperand>(_currentLabel++);}

void Parser::syntaxError(const std::string& message) { throw std::invalid_argument("Syntax error : " + message + "\n" + "Ошибка в строчке " + std::to_string(_scanner._n_str) + ":\n" + _scanner._buffer2 + "\n" + _scanner._buffer); }

void Parser::lexicalError(const std::string& message) { throw std::invalid_argument("Lexical error : " + message + "\n" + "Ошибка в строчке " + std::to_string(_scanner._n_str) + ":\n" + _scanner._buffer2 + "\n" + _scanner._buffer); }

void Parser::lexCheck() {
	if (_currentToken.getType() == LexemType::error) 
	{
		lexicalError(_currentToken.getStr());
	}
}

std::shared_ptr<Rvalue> Parser::E(Scope scope)
{
	return E7(scope);
}

std::shared_ptr<Rvalue> Parser::E1_(Scope scope, std::string name)
{
	lexCheck();
	if (_currentToken.getType() == LexemType::opinc)
	{
		_currentToken = _scanner.getNextToken();
		auto r = _symbolTable.alloc(scope);
		auto s = _symbolTable.checkVar(scope, name);

		if (s == nullptr){syntaxError("Переменнная " + name + " не определена");}
		generateAtom(std::make_unique<UnaryOpAtom>("MOV", s, r, _tab ), scope);
		generateAtom(std::make_unique<BinaryOpAtom>("ADD", s, _one, std::dynamic_pointer_cast<MemoryOperand>(s), _tab), scope);
		
		return r;
	}
	else if (_currentToken.getType() == LexemType::lpar)
	{
		_currentToken = _scanner.getNextToken();
		auto n = ArgList(scope);
		auto s = _symbolTable.checkFunc(name, n);
		if (s != nullptr) {
			if (_currentToken.getType() != LexemType::rpar) {syntaxError("Не закрыта )");}
			_currentToken = _scanner.getNextToken();
			auto r = _symbolTable.alloc(scope);
			generateAtom(std::make_unique<CallAtom>(s, r, _tab), scope);
			return r;
		}
		syntaxError("Попытка вызыва необъяленной функции");
	}
	if (_symbolTable.checkVar(scope, name) == nullptr) {syntaxError("Попытка вызыва необъяленной переменной");}
	return _symbolTable.checkVar(scope, name);
}

std::shared_ptr<Rvalue> Parser::E1(Scope scope)
{
	lexCheck();
	if (_currentToken.getType() == LexemType::num || _currentToken.getType() == LexemType::chr)
	{
		auto q = std::make_shared<NumberOperand>(_currentToken.getValue());
		_currentToken = _scanner.getNextToken();
		return q; 
	}

	if (_currentToken.getType() == LexemType::lpar)
	{
		_currentToken = _scanner.getNextToken();
		auto q = E(scope);
		lexCheck();

		if (_currentToken.getType() != LexemType::rpar){syntaxError("Не закрыта скобка");}
		_currentToken = _scanner.getNextToken();
		return q;
	}

	if (_currentToken.getType() == LexemType::opinc)
	{
		_currentToken = _scanner.getNextToken();

		if (_currentToken.getType() != LexemType::id){syntaxError("Неправильное значение инкремента");}
		auto q = _symbolTable.checkVar(scope, _currentToken.getStr());

		if (q == nullptr){syntaxError("Переменнная " + _currentToken.getStr() + " не определена");}
		generateAtom(std::make_unique<BinaryOpAtom>("ADD", q, _one, q, _tab), scope);
		_currentToken = _scanner.getNextToken();
		return q;
	}
	if (_currentToken.getType() == LexemType::id)
	{
		auto r = _currentToken.getStr();
		_currentToken = _scanner.getNextToken();
		auto p = E1_(scope, r);
		lexCheck();
		return p;
	}
	 syntaxError("Ошибка");
}

std::shared_ptr<Rvalue> Parser::E2(Scope scope) {
	lexCheck();
	if (_currentToken.getType() == LexemType::opnot)
	{
		_currentToken = _scanner.getNextToken();
		auto q = E1(scope);
		lexCheck();
		auto r = _symbolTable.alloc(scope);
		generateAtom(std::make_unique<UnaryOpAtom>("NOT", q, r, _tab), scope);
		return r;
	}
	auto q = E1(scope);
	lexCheck();
	return  q;
}

std::shared_ptr<Rvalue> Parser::E3(Scope scope)
{
	lexCheck();
	auto q = E2(scope);
	return E3_(q, scope);
}

std::shared_ptr<Rvalue> Parser::E3_(std::shared_ptr<Rvalue> p, Scope scope)
{
	lexCheck();
	if (_currentToken.getType() == LexemType::opmult) 
	{
		_currentToken = _scanner.getNextToken();
		auto r = E2(scope);
		auto s = _symbolTable.alloc(scope);
		generateAtom(std::make_unique<BinaryOpAtom>("MULT", p, r, s, _tab), scope);
		return E3_(s, scope);
	}
	return p;
}

std::shared_ptr<Rvalue> Parser::E4(Scope scope)
{
	lexCheck();
	auto q = E3(scope);
	return E4_(q, scope);
}

std::shared_ptr<Rvalue> Parser::E4_(std::shared_ptr<Rvalue> p, Scope scope) 
{
	lexCheck();
	if (_currentToken.getType() == LexemType::opplus)
	{
		_currentToken = _scanner.getNextToken();
		auto r = E3(scope);
		auto s = _symbolTable.alloc(scope);
		generateAtom(std::make_unique<BinaryOpAtom>("ADD", p, r, s, _tab), scope);
		return E4_(s, scope);
	}
	else if (_currentToken.getType() == LexemType::opminus)
	{
		_currentToken = _scanner.getNextToken();
		auto r = E3(scope);
		auto s = _symbolTable.alloc(scope);
		generateAtom(std::make_unique<BinaryOpAtom>("SUB", p, r, s, _tab), scope);
		return E4_(s, scope);
	}
	return p;
}

std::shared_ptr<Rvalue> Parser::E5(Scope scope) 
{
	lexCheck();
	auto q = E4(scope);
	return E5_(q, scope);
}

std::shared_ptr<Rvalue> Parser::E5_(std::shared_ptr<Rvalue> p, Scope scope)
{
	lexCheck();
	if (_currentToken.getType() == LexemType::opeq ||
		_currentToken.getType() == LexemType::opne ||
		_currentToken.getType() == LexemType::oplt ||
		_currentToken.getType() == LexemType::opgt ||
		_currentToken.getType() == LexemType::ople) 
	{
		auto op = _currentToken.getType();
		_currentToken = _scanner.getNextToken();
		auto r = E4(scope);
		auto s = _symbolTable.alloc(scope);
		auto l = newLabel();

		generateAtom(std::make_unique<UnaryOpAtom>("MOV", _one, s, _tab), scope);
		generateAtom(std::make_unique<ConditonalJumpAtom>(_jumps[op], p, r, l, _tab), scope);
		generateAtom(std::make_unique<UnaryOpAtom>("MOV", _zero, s, _tab), scope);
		generateAtom(std::make_unique<LabelAtom>(l, _tab), scope);
		return s;
	}
	return p;
}

std::shared_ptr<Rvalue> Parser::E6(Scope scope) 
{
	lexCheck();
	auto q = E5(scope);
	return E6_(q, scope);
}

std::shared_ptr<Rvalue> Parser::E6_(std::shared_ptr<Rvalue> p, Scope scope) 
{
	lexCheck();
	if (_currentToken.getType() == LexemType::opand) {
		_currentToken = _scanner.getNextToken();
		auto r = E5(scope);
		auto s = _symbolTable.alloc(scope);
		generateAtom(std::make_unique<BinaryOpAtom>("AND", p, r, s, _tab), scope);
		return E6_(s, scope);
	}
	return p;
}

std::shared_ptr<Rvalue> Parser::E7(Scope scope) {
	lexCheck();
	auto q = E6(scope);
	return E7_(q, scope);
}

std::shared_ptr<Rvalue> Parser::E7_(std::shared_ptr<Rvalue> p, Scope scope) {
	lexCheck();
	if (_currentToken.getType() == LexemType::opor) {
		_currentToken = _scanner.getNextToken();
		auto r = E6(scope);
		lexCheck();
		auto s = _symbolTable.alloc(scope);
		generateAtom(std::make_unique<BinaryOpAtom>("OR", p, r, s, _tab), scope);
		return E7_(s, scope);
	}
	return p;
}

int Parser::ArgList(Scope scope) 
{
	lexCheck();
	if (_currentToken.getType() != LexemType::rpar) 
	{
		auto p = E(scope);
		lexCheck();
		auto m = ArgList_(scope);
		generateAtom(std::make_unique<ParamAtom>(p, _tab), scope);
		return m + 1;
	}
	return 0;
}

int Parser::ArgList_(Scope scope) 
{
	lexCheck();
	if (_currentToken.getType() == LexemType::comma)
	{
		_currentToken = _scanner.getNextToken();
		auto p = E(scope);
		lexCheck();
		auto m = ArgList_(scope);
		lexCheck();
		generateAtom(std::make_unique<ParamAtom>(p, _tab), scope);
		return m + 1;
	}
	return 0;
}

///1
void Parser::DeclareStmt(Scope scope)
{
	lexCheck();
	SymbolTable::TableRecord::RecordType p;
	switch (_currentToken.getType())
	{
	case LexemType::kwchar:
		 p = SymbolTable::TableRecord::RecordType::chr;
		 break;

	case LexemType::kwint:
		 p = SymbolTable::TableRecord::RecordType::integer;
		 break;

	default:
		syntaxError("Ошибка объявления");
	}

	_currentToken = _scanner.getNextToken();
	if (_currentToken.getType() != LexemType::id){syntaxError("Ошибка объявления");}

	std::string name = _currentToken.getStr();
	_currentToken = _scanner.getNextToken();
	DeclareStmt_(p, name, scope);
}

///2-4
void Parser::DeclareStmt_(SymbolTable::TableRecord::RecordType type, std::string name, Scope scope)
{
	lexCheck();
	if (_currentToken.getType() == LexemType::lpar)
	{
		if (scope > -1){	syntaxError("Функция определена внутри функции");}
		_currentToken = _scanner.getNextToken();
		auto newFunc = _symbolTable.addFunc(name, type, 0);
		int index = (*newFunc).index();
		generateAtom(std::make_unique<Comment>("//Начало функции", _tab), index);
		std::string buf = _tab;
		_tab = _tab + "  ";
		int n = ParamList(index);
		_symbolTable.chFuncLen(index, n);

		if (_currentToken.getType() != LexemType::rpar) { syntaxError("Не закрыта скобка с передаваемыми переменными"); }
		_currentToken = _scanner.getNextToken();
		if (_currentToken.getType() != LexemType::lbrace) {syntaxError("Нет тела функции");}
		_currentToken = _scanner.getNextToken();

		StmtList(index);
		if (_currentToken.getType() != LexemType::rbrace) {	syntaxError("Не закрыта скобка после тела функции");}

		generateAtom(std::make_unique<RetAtom>(_zero, _tab), index);
		_tab = buf;
		generateAtom(std::make_unique<Comment>("Конец функции//\n", _tab), index);
		_currentToken = _scanner.getNextToken();
	}
	else if (_currentToken.getType() == LexemType::opassign)
	{
		_currentToken = _scanner.getNextToken();

		if (_currentToken.getType() != LexemType::num && _currentToken.getType() != LexemType::chr){syntaxError("Неподходящее значение для присваивания");}
		auto checkExist = _symbolTable.addVar(name, scope, type, _currentToken.getValue());

		if (checkExist == nullptr) {syntaxError("Данная переменная уже обьявлена");}
		_currentToken = _scanner.getNextToken();
		DeclVarList_(type, scope);

		if (_currentToken.getType() != LexemType::semicolon) {syntaxError("Нету точки с запятой");}
		_currentToken = _scanner.getNextToken();
	}
	else
	{
		_symbolTable.addVar(name, scope, type);
		DeclVarList_(type, scope);

		if (_currentToken.getType() != LexemType::semicolon){	syntaxError("Нету точки с запятой");}
		_currentToken = _scanner.getNextToken();
	}
}

//7-8
void Parser::DeclVarList_(SymbolTable::TableRecord::RecordType type, Scope scope) 
{
	lexCheck();
	if (_currentToken.getType() == LexemType::comma) 
	{
		_currentToken = _scanner.getNextToken();

		if (_currentToken.getType() != LexemType::id){syntaxError("Ошибка объявления");}
		std::string name = _currentToken.getStr();
		_currentToken = _scanner.getNextToken();
		InitVar(type, name, scope);
		DeclVarList_(type, scope);
	}
}

//9-10
void Parser::InitVar(SymbolTable::TableRecord::RecordType type, std::string name, Scope scope) 
{
	lexCheck();
	if (_currentToken.getType() == LexemType::opassign)
	{
		_currentToken = _scanner.getNextToken();
		if (_currentToken.getType() == LexemType::num || _currentToken.getType() == LexemType::chr) 
		{
			_symbolTable.addVar(name, scope, type, _currentToken.getValue());
			_currentToken = _scanner.getNextToken();
		}
		else {	syntaxError("Неподходящее значение для присваивания");}
	}
	else 
	{
		_symbolTable.addVar(name, scope, type);
	}
}

// 11-12
int Parser::ParamList(Scope scope) 
{
	lexCheck();

	if (_currentToken.getType() == LexemType::kwint || _currentToken.getType() == LexemType::kwchar) 
	{

		SymbolTable::TableRecord::RecordType q;
		switch (_currentToken.getType())
		{
		case LexemType::kwchar:
			q = SymbolTable::TableRecord::RecordType::integer;
			break;

		case LexemType::kwint:
			q = SymbolTable::TableRecord::RecordType::chr;
			break;
		}
		_currentToken = _scanner.getNextToken();

		if (_currentToken.getType() != LexemType::id){syntaxError("Ошибка объявления внутри секции передаваемых параметров");}
		_symbolTable.addVar(_currentToken.getStr(), scope, q);
		_currentToken = _scanner.getNextToken();
		return ParamList_(scope) + 1;
	}
	return 0;
}

//13-14
int Parser::ParamList_(Scope scope) {
	lexCheck();

	if (_currentToken.getType() == LexemType::comma) {
		_currentToken = _scanner.getNextToken();
		if (_currentToken.getType() == LexemType::kwint || _currentToken.getType() == LexemType::kwchar) 
		{
			SymbolTable::TableRecord::RecordType q;
			switch (_currentToken.getType())
			{
			case LexemType::kwchar:
				q = SymbolTable::TableRecord::RecordType::integer;
				break;

			case LexemType::kwint:
				q = SymbolTable::TableRecord::RecordType::chr;
				break;
			}

			_currentToken = _scanner.getNextToken();
			if (_currentToken.getType() != LexemType::id)
			{
				syntaxError("Ошибка объявления внутри секции передаваемых параметров");
			}

			_symbolTable.addVar(_currentToken.getStr(), scope, q);
			_currentToken = _scanner.getNextToken();
			return ParamList_(scope) + 1;
		}
		else 
		{
			syntaxError("Ошибка внутри секции передаваемых параметров");
		}
	}
	return 0;
}

//15-16
void Parser::StmtList(Scope scope) 
{
	lexCheck();
	if ((_currentToken.getType() != LexemType::eof) &&
		(_currentToken.getType() != LexemType::rbrace) &&
		(_currentToken.getType() != LexemType::kwcase) &&
		(_currentToken.getType() != LexemType::kwdefault))
	{
		Stmt(scope);
		StmtList(scope);
	}
}

//17-27
void Parser::Stmt(Scope scope) 
{
	lexCheck();
	if ((_currentToken.getType() == LexemType::kwint) || (_currentToken.getType() == LexemType::kwchar)) 
	{
		DeclareStmt(scope);
	}

	else if (_currentToken.getType() == LexemType::id) {
		if (scope == -1) {syntaxError("Оператор должен быть внутри функции");}
		AssignOrCallOp(scope);
	}

	else if (_currentToken.getType() == LexemType::kwwhile)
	{
		if (scope == -1) {syntaxError("Оператор должен быть внутри функции");}
		WhileOp(scope);
	}

	else if (_currentToken.getType() == LexemType::kwfor) 
	{
		if (scope == -1) {syntaxError("Оператор должен быть внутри функции");}
		ForOp(scope);
	}

	else if (_currentToken.getType() == LexemType::kwif) 
	{
		if (scope == -1) {syntaxError("Оператор должен быть внутри функции");}
		IfOp(scope);
	}

	else if (_currentToken.getType() == LexemType::kwswitch) 
	{
		if (scope == -1) {syntaxError("Оператор должен быть внутри функции");}
		SwitchOp(scope);
	}

	else if (_currentToken.getType() == LexemType::kwin) 
	{
		if (scope == -1) {syntaxError("Оператор должен быть внутри функции");}
		IOp(scope);
	}

	else if (_currentToken.getType() == LexemType::kwout) 
	{
		if (scope == -1) {syntaxError("Оператор должен быть внутри функции");}
		OOp(scope);
	}

	else if (_currentToken.getType() == LexemType::lbrace) 
	{
		if (scope == -1) {syntaxError("Оператор должен быть внутри функции");}
		_currentToken = _scanner.getNextToken();
		StmtList(scope);
		lexCheck();
		if (_currentToken.getType() != LexemType::rbrace) {syntaxError("Не закрыта }");}
		_currentToken = _scanner.getNextToken();
	}

	else if (_currentToken.getType() == LexemType::kwreturn) 
	{
		if (scope == -1) {syntaxError("Оператор должен быть внутри функции");}
		_currentToken = _scanner.getNextToken();
		auto p = E(scope);
		lexCheck();
		generateAtom(std::make_unique<RetAtom>(p, _tab), scope);
		if (_currentToken.getType() != LexemType::semicolon) {syntaxError("Нету точки с запятой");}
		_currentToken = _scanner.getNextToken();
	}

	else if (_currentToken.getType() == LexemType::semicolon) 
	{
		_currentToken = _scanner.getNextToken();
	}
	else {syntaxError("Ошибка");}
}

//28
void Parser::AssignOrCallOp(Scope scope) 
{
	lexCheck();
	AssignOrCall(scope);
	lexCheck();

	if (_currentToken.getType() != LexemType::semicolon) {syntaxError("Нету точки с запятой");}
	_currentToken = _scanner.getNextToken();
}

//29
void Parser::AssignOrCall(Scope scope)
{
	lexCheck();
	if (_currentToken.getType() != LexemType::id) {syntaxError("Ожидалась переменная или функция");}
	std::string name = _currentToken.getStr();
	_currentToken = _scanner.getNextToken();
	AssignOrCall_(name, scope);

}

//30-31
void Parser::AssignOrCall_(std::string p, Scope scope) 
{
	lexCheck();
	if (_currentToken.getType() == LexemType::opassign) 
	{
		_currentToken = _scanner.getNextToken();
		auto q = E(scope);
		auto r = _symbolTable.checkVar(scope, p);

		if (r == nullptr) {syntaxError("Переменная не объявлена");}
		generateAtom(std::make_unique<UnaryOpAtom>("MOV", q, r, _tab), scope);
	}
	else if (_currentToken.getType() == LexemType::lpar) 
	{
		_currentToken = _scanner.getNextToken();
		auto n = ArgList(scope);

		lexCheck();
		if (_currentToken.getType() != LexemType::rpar) {syntaxError("Не закрыта )");}
		_currentToken = _scanner.getNextToken();

		auto q = _symbolTable.checkFunc(p, n);
		if (q == nullptr) {syntaxError("Функция не найдена");}
		auto r = _symbolTable.alloc(scope);

		generateAtom(std::make_unique<CallAtom>(q, r, _tab), scope);
	}
	//ргр
	else if (_currentToken.getType() == LexemType::abbsub)
	{
		_currentToken = _scanner.getNextToken();
		auto q = E(scope);
		auto r = _symbolTable.checkVar(scope, p);

		if (r == nullptr) { syntaxError("Переменная не объявлена"); }
		auto s = _symbolTable.alloc(scope);
		generateAtom(std::make_unique<BinaryOpAtom>("SUB", r, q, s, _tab), scope);
		generateAtom(std::make_unique<UnaryOpAtom>("MOV", s, r, _tab), scope);
	//
	}
	else { syntaxError("Ошибка");}
}

//32
void Parser::WhileOp(Scope scope) 
{
	lexCheck();
	_currentToken = _scanner.getNextToken();
	auto l1 = newLabel();
	generateAtom(std::make_unique<Comment>("//Начало секции while", _tab), scope);
	generateAtom(std::make_unique<LabelAtom>(l1, _tab), scope);

	if (_currentToken.getType() != LexemType::lpar){syntaxError("Неправильное условие while");}
	_currentToken = _scanner.getNextToken();
	generateAtom(std::make_unique<Comment>("//Начало секции условия while", _tab), scope);
	std::shared_ptr<Rvalue> p = E(scope);

	if (_currentToken.getType() != LexemType::rpar) { syntaxError("Не закрыта )");}
	_currentToken = _scanner.getNextToken();
	auto l2 = newLabel();
	generateAtom(std::make_unique<ConditonalJumpAtom>("EQ", p, _zero, l2, _tab), scope);
	generateAtom(std::make_unique<Comment>( "Конец секции условия while//", _tab), scope);
	std::string buf = _tab;
	_tab = _tab + "  ";
	generateAtom(std::make_unique<Comment>("//Начало тела while", _tab), scope);
	Stmt(scope);
	generateAtom(std::make_unique<JumpAtom>(l1, _tab), scope);
	generateAtom(std::make_unique<LabelAtom>(l2, _tab), scope);
	generateAtom(std::make_unique<Comment>("Конец тела while//", _tab), scope);
	_tab = buf;
}

//33
void Parser::ForOp(Scope scope) 
{
	lexCheck();
	_currentToken = _scanner.getNextToken();
	generateAtom(std::make_unique<Comment>("//Начало секции for", _tab), scope);
	if (_currentToken.getType() != LexemType::lpar) { syntaxError("Неправильное условие for"); }
	_currentToken = _scanner.getNextToken();
	generateAtom(std::make_unique<Comment>("//Объявление счетчика for", _tab), scope);
	ForInit(scope);
	generateAtom(std::make_unique<Comment>("Конец объявление счетчика for//", _tab), scope);
	lexCheck();

	if (_currentToken.getType() != LexemType::semicolon) { syntaxError("Неправильное условие for"); }
	_currentToken = _scanner.getNextToken();
	auto l1 = newLabel();
	generateAtom(std::make_unique<Comment>("//Объявление условия for", _tab), scope);
	generateAtom(std::make_unique<LabelAtom>(l1, _tab), scope);
	auto p = ForExp(scope);
	lexCheck();

	if (_currentToken.getType() != LexemType::semicolon) { syntaxError("Неправильное условие for"); }
	_currentToken = _scanner.getNextToken();
	auto l4 = newLabel();
	generateAtom(std::make_unique<ConditonalJumpAtom>("EQ", p, _zero, l4, _tab), scope);

	auto l2 = newLabel();
	auto l3 = newLabel();
	generateAtom(std::make_unique<JumpAtom>(l3, _tab), scope);
	generateAtom(std::make_unique<Comment>("Конец объявление условия for//", _tab), scope);
	generateAtom(std::make_unique<Comment>("//Начало инкремента for", _tab), scope);
	generateAtom(std::make_unique<LabelAtom>(l2, _tab), scope);

	ForLoop(scope);
	lexCheck();

	generateAtom(std::make_unique<JumpAtom>(l1, _tab), scope);
	generateAtom(std::make_unique<Comment>("Конец инкремента for и перехода в начало//", _tab), scope);

	if (_currentToken.getType() != LexemType::rpar) { syntaxError("Не закрыта скобка )");}
	_currentToken = _scanner.getNextToken();
	std::string buf = _tab;
	_tab = _tab + "  ";
	generateAtom(std::make_unique<Comment>("//Переход в тело цикла", _tab), scope);
	generateAtom(std::make_unique<LabelAtom>(l3, _tab), scope);
	Stmt(scope);
	lexCheck();

	generateAtom(std::make_unique<JumpAtom>(l2, _tab), scope);
	generateAtom(std::make_unique<Comment>("Выход из тела цикла//", _tab), scope);
	_tab = buf;
	generateAtom(std::make_unique<LabelAtom>(l4, _tab), scope);
}

//34-35
void Parser::ForInit(Scope scope)
{
	lexCheck();
	if (_currentToken.getType() == LexemType::id)
	{
		AssignOrCall(scope);
	}
}

//36-37
std::shared_ptr<Rvalue> Parser::ForExp(Scope scope) 
{
	lexCheck();
	if (_currentToken.getType() == LexemType::opnot ||
		_currentToken.getType() == LexemType::lpar ||
		_currentToken.getType() == LexemType::opinc ||
		_currentToken.getType() == LexemType::id ||
		_currentToken.getType() == LexemType::num) 
	{
		auto p = E(scope);
		lexCheck();
		return p;
	}
	return _one;
}

//38-40
void Parser::ForLoop(Scope scope) 
{
	lexCheck();

	if (_currentToken.getType() == LexemType::id) 
	{
		AssignOrCallOp(scope);
	}

	else if (_currentToken.getType() == LexemType::opinc) {
		_currentToken = _scanner.getNextToken();
		if (_currentToken.getType() != LexemType::id) {syntaxError("Неправильное условие for");}
		_currentToken = _scanner.getNextToken();
		auto p = _symbolTable.checkVar(scope, _currentToken.getStr());

		if (p == nullptr) {syntaxError("Неправильное условие for");}
		generateAtom(std::make_unique <BinaryOpAtom>("ADD", p, _one, p, _tab), scope);
	}
}

//41
void Parser::IfOp(Scope scope) 
{
	lexCheck();
	_currentToken = _scanner.getNextToken();

	if (_currentToken.getType() != LexemType::lpar) {syntaxError("Неправильно построенный if");}
	_currentToken = _scanner.getNextToken();
	generateAtom(std::make_unique<Comment>("//Начало секции if-else", _tab), scope);
	generateAtom(std::make_unique<Comment>("//Начало секции условия if", _tab), scope);
	auto p = E(scope);
	generateAtom(std::make_unique<Comment>("Конец секции условия if//", _tab), scope);
	lexCheck();

	if (_currentToken.getType() != LexemType::rpar) { syntaxError("Не закрыта скобка )");}
	_currentToken = _scanner.getNextToken();
	auto l1 = newLabel();
	generateAtom(std::make_unique<Comment>("//Начало секции if", _tab), scope);
	std::string buf = _tab;
	_tab = _tab + "  ";
	generateAtom(std::make_unique<ConditonalJumpAtom>("EQ", p, _zero, l1, _tab), scope);

	Stmt(scope);
	auto l2 = newLabel();
	generateAtom(std::make_unique<JumpAtom>(l2, _tab), scope);
	_tab = buf;
	generateAtom(std::make_unique<Comment>("Конец секции if//", _tab), scope);
	generateAtom(std::make_unique<Comment>("//Начало секции else", _tab), scope);
	_tab = _tab + "  ";
	generateAtom(std::make_unique<LabelAtom>(l1, _tab), scope);
	ElsePart(scope);
	_tab = buf;
	generateAtom(std::make_unique<Comment>("Конец секции if-else//", _tab), scope);
	lexCheck();
	generateAtom(std::make_unique<LabelAtom>(l2, _tab), scope);
}

//42-43
void Parser::ElsePart(Scope scope) 
{
	lexCheck();
	if (_currentToken.getType() == LexemType::kwelse)
	{
		_currentToken = _scanner.getNextToken();
		generateAtom(std::make_unique<Comment>("//Начало секции else", _tab), scope);
		Stmt(scope);
	}
}

//44
void Parser::SwitchOp(Scope scope) 
{
	lexCheck();
	_currentToken = _scanner.getNextToken();

	if (_currentToken.getType() != LexemType::lpar) { syntaxError("Неправильный switch");}
	_currentToken = _scanner.getNextToken();
	generateAtom(std::make_unique<Comment>("//Начало секции условия switch", _tab), scope);
	auto p = E(scope);
	lexCheck();
	generateAtom(std::make_unique<Comment>("Конец секции условия switch//", _tab), scope);
	if (_currentToken.getType() != LexemType::rpar) { syntaxError("Не закрыта скобка )");}
	_currentToken = _scanner.getNextToken();
	auto end = newLabel();

	if (_currentToken.getType() != LexemType::lbrace) { syntaxError("Нет тела switch");}
	_currentToken = _scanner.getNextToken();
	generateAtom(std::make_unique<Comment>("//Начало тела switch", _tab), scope);
	Cases(p, end, scope);
	generateAtom(std::make_unique<Comment>("Конец тела switch//", _tab), scope);
	if (_currentToken.getType() != LexemType::rbrace) { syntaxError("Не закрыта скобка }");}
	_currentToken = _scanner.getNextToken();
	generateAtom(std::make_unique<LabelAtom>(end, _tab), scope);
}

//45
void Parser::Cases(std::shared_ptr<Rvalue> p, std::shared_ptr<LabelOperand> end, Scope scope) 
{
	lexCheck();
	auto def1 = Acase(p, end, scope);
	lexCheck();
	Cases_(p, end, def1, scope);
}

//46-47
void Parser::Cases_(std::shared_ptr<Rvalue> p, std::shared_ptr<LabelOperand> end, std::shared_ptr<LabelOperand> def, Scope scope) 
{
	lexCheck();

	if ((_currentToken.getType() == LexemType::kwcase) ||(_currentToken.getType() == LexemType::kwdefault))
	{
		auto def1 = Acase(p, end, scope);
		lexCheck();
		if ((def != nullptr) && (def1 != nullptr)) { syntaxError("Две default секции");}
		Cases_(p, end, def == nullptr ? def1 : def, scope);
	}
	else 
	{
		generateAtom(std::make_unique<JumpAtom>(def != nullptr ? def : end, _tab), scope);
	}
}

//48-49
std::shared_ptr<LabelOperand> Parser::Acase(std::shared_ptr<Rvalue> p, std::shared_ptr<LabelOperand> end, Scope scope) {
	lexCheck();

	if (_currentToken.getType() == LexemType::kwcase) 
	{
		_currentToken = _scanner.getNextToken();

		if (_currentToken.getType() != LexemType::num) { syntaxError("В case должно быть число");}
			auto value = std::make_shared<NumberOperand>(_currentToken.getValue());
			auto next = newLabel();
			generateAtom(std::make_unique<Comment>("//case", _tab), scope);
			generateAtom(std::make_unique<ConditonalJumpAtom>("NE", p, value, next, _tab), scope);
			_currentToken = _scanner.getNextToken();

			if (_currentToken.getType() != LexemType::colon) { syntaxError("Нету двоеточия"); }
			std::string buf = _tab;
			_tab = _tab + "  ";
			_currentToken = _scanner.getNextToken();
			StmtList(scope);
			lexCheck();
			_tab = buf;
			generateAtom(std::make_unique<JumpAtom>(end, _tab), scope);
			generateAtom(std::make_unique<LabelAtom>(next, _tab), scope);

			return nullptr;
	}

	else if (_currentToken.getType() == LexemType::kwdefault) 
	{
		_currentToken = _scanner.getNextToken();

		lexCheck();
		if (_currentToken.getType() != LexemType::colon) { syntaxError("Нету двоеточия");}
		_currentToken = _scanner.getNextToken();

		auto next = newLabel();
		auto def = newLabel();
		generateAtom(std::make_unique<Comment>("//default", _tab), scope);
		generateAtom(std::make_unique<JumpAtom>(next, _tab), scope);
		generateAtom(std::make_unique<LabelAtom>(def, _tab), scope);
		std::string buf = _tab;
		_tab = _tab + "  ";
		StmtList(scope);
		_tab = buf;
		lexCheck();
		generateAtom(std::make_unique<JumpAtom>(end, _tab), scope);
		generateAtom(std::make_unique<LabelAtom>(next, _tab), scope);

		return def;
	}
	else { syntaxError("Неправильно построенный switch");}
}

//50
void Parser::IOp(Scope scope) 
{
	lexCheck();
	_currentToken = _scanner.getNextToken();

	if (_currentToken.getType() != LexemType::id) { syntaxError("Ожидалась переменная или функция");}
	auto p = _symbolTable.checkVar(scope, _currentToken.getStr());
	_currentToken = _scanner.getNextToken();

	if (p == nullptr) { syntaxError("Переменная не объявлена");}

	if (_currentToken.getType() != LexemType::semicolon) { syntaxError("Нету точки с запятой");}
	_currentToken = _scanner.getNextToken();
	generateAtom(std::make_unique<InAtom>(p, _tab), scope);
}

//51
void Parser::OOp(Scope scope) 
{
	lexCheck();
	_currentToken = _scanner.getNextToken();
	OOp_(scope);

	if (_currentToken.getType() != LexemType::semicolon) { syntaxError("Нету точки с запятой");}
	_currentToken = _scanner.getNextToken();
}

//52-53
void Parser::OOp_(Scope scope) 
{
	lexCheck();

	if (_currentToken.getType() == LexemType::str) 
	{
		auto str = _stringTable.add(_currentToken.getStr());
		auto r = std::make_shared<StringOperand>(str, &_stringTable);
		generateAtom(std::make_unique<OutAtom>(r, _tab), scope);
		_currentToken = _scanner.getNextToken();
	}

	else 
	{
		auto p = E(scope);
		lexCheck();
		generateAtom(std::make_unique<OutAtom>(p, _tab), scope);
	}
}

void Parser::checkMain()
{
	if (_symbolTable.checkMain() == nullptr){syntaxError("Отсутсвует main");}
}