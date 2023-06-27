#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "Scanner.h"

Token::Token(LexemType type) : _type{ type }, _value{ 0 } {}
Token::Token(int value) : _value{ value }, _type{ LexemType::num } {}
Token::Token(LexemType type, const std::string& str) : _type{ type }, _value{ 0 }, _str{ str } {}
Token::Token(char c) : _type{ LexemType::chr }, _value{ c } {}

LexemType Token::getType() {return _type;}

int Token::getValue() {return _value;}

std::string Token::getStr() {return _str;}

std::vector<std::string> LexemTypeNames{ "num", "chr", "str", "id", "lpar", "rpar", "lbrace", "rbrace", "lbracket", "rbracket", "semicolon", "comma", "colon", "opassign", "opplus", "opminus", "opmult", "opinc", "opeq", "opne", "oplt", "opgt", "ople", "opnot", "opor", "opand", "kwint", "kwchar", "kwif", "kwelse", "kwswitch", "kwcase", "kwdefault", "kwwhile", "kwfor", "kwreturn", "kwin", "kwout", "eof", "error" };

std::ostream& operator<<(std::ostream& stream, Token& token) 
{
    stream << "[" << LexemTypeNames[static_cast<int>(token._type)];
    switch (token._type) 
    {
    case LexemType::num: stream << ", " << token._value; break;
    case LexemType::chr: stream << ", " << static_cast<char>(token._value); break;
    case LexemType::error:
    case LexemType::id:
    case LexemType::str: stream << ", " << token._str; break;
    default: break;
    }
    stream << "]";
    return stream;
}

Scanner::Scanner(std::istream& stream) : _inputStream{ stream }, _bufferPosition{ -1 }, _bufferLength{ 0 }, _savedChar{ 0 }, _symb{ 0 }, _state{ 0 }, _buffer2{ " " }, _buffer{ " " }, _n_str{ 0 } {}

char Scanner::peekChar() {

    if (!_inputStream) return 0;

    if (_savedChar) return _savedChar;

    if (_bufferPosition >= _bufferLength || _bufferPosition < 0) 
    { 
        _bufferLength = 0;

        while (_inputStream && _bufferLength == 0) 
        {
            _buffer2 = _buffer;
            std::getline(_inputStream, _buffer);
            _bufferLength = _buffer.length();
            _n_str = _n_str + 1;
        }

        if (!_inputStream) 
        {
            return 0;
        }

        else 
        {
            _bufferPosition = 0;
        }
    }
    return _buffer[_bufferPosition];
}

char Scanner::readChar() 
{
    char c = peekChar();
    if (_savedChar) _savedChar = 0;
    else if (c) ++_bufferPosition;
    return c;
}

inline void Scanner::returnChar(char c) {_savedChar = c;}

inline bool Scanner::isDigit(char c) {return c >= '0' && c <= '9';}

std::map <char, LexemType> punctuation
{
    { '(', LexemType::lpar},
    { ')', LexemType::rpar},
    { '{', LexemType::lbrace},
    { '}', LexemType::rbrace},
    { '[', LexemType::lbracket},
    { ']', LexemType::rbracket},
    { ';', LexemType::semicolon},
    { ',', LexemType::comma},
    { ':', LexemType::colon}
};

std::map <std::string, LexemType> keywords
{
    { "int", LexemType::kwint},
    { "char", LexemType::kwchar},
    { "if", LexemType::kwif},
    { "else", LexemType::kwelse},
    { "switch", LexemType::kwswitch},
    { "case", LexemType::kwcase},
    { "default", LexemType::kwdefault},
    { "while", LexemType::kwwhile},
    { "for", LexemType::kwfor},
    { "return", LexemType::kwreturn},
    { "in", LexemType::kwin},
    { "out", LexemType::kwout}
};

Token Scanner::getNextToken() 
{

    int integerValue{ 0 };
    std::string stringValue;

    while (true)
    {
        _symb = readChar();
        switch (_state) 
        {
        case 0:
            if (_symb == 0) return Token(LexemType::eof);
            else if (isspace(_symb)) continue;
            else if (isDigit(_symb)) 
            {
                integerValue = _symb - '0';
                _state = 1;
            }
            else if (_symb == '\'')
            {
                integerValue = 0;
                _state = 2;
            }
            else if (_symb == '\"') 
            {
                stringValue = "";
                _state = 4;
            }
            else if (isalpha(_symb))
            {
                stringValue = _symb;
                _state = 5;
            }
            else if (punctuation.find(_symb) != punctuation.end()) { return Token(punctuation[_symb]); }
            else if (_symb == '>') { return Token(LexemType::opgt); }
            else if (_symb == '*') { return Token(LexemType::opmult); }
            else if (_symb == '-') { _state = 6; }
            else if (_symb == '!') { _state = 7; }
            else if (_symb == '<') { _state = 8; }
            else if (_symb == '=') { _state = 9; }
            else if (_symb == '+') { _state = 10; }
            else if (_symb == '|') { _state = 11; }
            else if (_symb == '&') { _state = 12; }
            else { return Token(LexemType::error, "Unknown symbol"); }
            break;

        case 1:
            if (isDigit(_symb)) 
            {
                integerValue *= 10;
                if (integerValue > 0) { integerValue += (_symb - '0'); }
                else { integerValue -= (_symb - '0'); }
            }
            else 
            {
                returnChar(_symb);
                _state = 0;
                return Token(integerValue);
            }
            break;

        case 2:
            if (_symb == '\'')
            {
                _state = 0;
                return Token(LexemType::error, "Empty char constant");
            }
            else 
            {
                _state = 3;
                integerValue = _symb;
            }
            break;

        case 3:
            if (_symb == '\'')
            {
                _state = 0;
                return Token(static_cast<char>(integerValue));
            }
            else 
            {
                _state = 0;
                return Token(LexemType::error, "Unclosed char const");
            }
            break;

        case 4:
            if (_symb == 0) 
            {
                _state = 0;
                return Token(LexemType::error, "Unclosed string const");
            }
            else if (_symb == '\"')
            {
                _state = 0;
                return Token(LexemType::str, stringValue);
            }
            else {stringValue += _symb; }
            break;
        case 5:
            if (isalnum(_symb)) {stringValue += _symb;}
            else 
            {
                _state = 0;
                returnChar(_symb);
                if (keywords.find(stringValue) != keywords.end()) { return Token(keywords[stringValue]); }
                return Token(LexemType::id, stringValue);
            }
            break;

        case 6:
            if (isDigit(_symb))
            {
                integerValue -= _symb - '0';
                _state = 1;
            }
            //ргр
            else if (_symb == '=')
            {
                _state = 0;
                return Token(LexemType::abbsub);
            }
            //
            else 
            {
                _state = 0;
                returnChar(_symb);
                return Token(LexemType::opminus);
            }
            break;
        case 7:
            _state = 0;
            if (_symb == '=') { return Token(LexemType::opne); }
            returnChar(_symb);
            return Token(LexemType::opnot);
            break;

        case 8:
            _state = 0;
            if (_symb == '=') { return Token(LexemType::oplt); }
            returnChar(_symb);
            return Token(LexemType::ople);
            break;

        case 9:
            _state = 0;
            if (_symb == '=') { return Token(LexemType::opeq); }
            returnChar(_symb);
            return Token(LexemType::opassign);
            break;

        case 10:
            _state = 0;
            if (_symb == '+') { return Token(LexemType::opinc); }
            returnChar(_symb);
            return Token(LexemType::opplus);
            break;

        case 11:
            _state = 0;
            if (_symb == '|') { return Token(LexemType::opor); }
            return Token(LexemType::error, "Unknown operation |");
            break;

        case 12:
            _state = 0;
            if (_symb == '&') { return Token(LexemType::opand); }
            return Token(LexemType::error, "Unknown operation &");
            break;

        default:
            return Token(LexemType::error, "Undefine behavior");
            break;
        }
    }
}