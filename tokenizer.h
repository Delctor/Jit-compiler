#pragma once
#include <iostream>
#include <string>
#include <optional>
#include <vector>
#include <bitset>
#include <ranges>
#include <immintrin.h>

#define getNSpaces(statementLen) \
		size_t nSpaces = 0; \
		size_t i = this->index - statementLen; \
		while (_addcarry_u64(0, i, -1, &i) && this->src[i] == ' ') \
		{ \
			nSpaces++; \
		}

namespace jit
{
	enum class TokenType
	{
		// Data type
		BOOL,
		INT,
		FLOAT,
		STRING,
		// Parentesis
		OPEN_PAREN,
		CLOSE_PAREN,
		// Semicolon
		SEMI,
		// Square Brackets
		OPEN_SQUARE_BRACKETS,
		CLOSE_SQUARE_BRACKETS,
		// Identifiers
		IDENT,
		// Operators
		ADD,
		SUB,
		MUL,
		DIV,
		REMAINDER,
		EEQ,
		GT,
		LT,
		GE,
		LE,
		EQ,
		IN,
		DOT_PRODUCT,
		// Block statements
		IF,
		ELIF,
		ELSE,
		FOR,
		WHILE,
		// Dot
		DOT,
		// Comma
		COMMA,
		// Scope
		SCOPE_START,
		SCOPE_END,
		// Def
		DEF, 
	};

	struct statementType
	{
		size_t nSpaces;
		bool closed = false;
	};

	struct Token
	{
		TokenType type;
		std::optional<std::string> value;
		std::optional<statementType> statement;
	};

	class tokenizer // I don't need to check a lot of syntax, because Python won't let me define functions with syntax errors.
	{
	public:
		inline tokenizer(const std::string& src) :
			src(src),
			index(0),
			nSpacesInitial(0) {}

		inline std::vector<Token> tokenize()
		{
			std::vector<Token> tokens;
			std::string buf;
			bool firstBlockStatement = true;
			while (this->peek().has_value())
			{
				// Statements and identifiers
				if (std::isalpha(this->peek().value()) || this->peek().value() == '_')
				{
					buf.push_back(this->consume());

					while (this->peek().has_value() && (std::isalnum(this->peek().value()) || this->peek().value() == '_'))
					{
						buf.push_back(this->consume());
					}

					if (buf == "if")
					{
						getNSpaces(2);
						if (firstBlockStatement) this->nSpacesInitial = nSpaces; firstBlockStatement = false;
						tokens.push_back({ .type = TokenType::IF, .statement = statementType {.nSpaces = nSpaces } });
					}
					else if (buf == "elif")
					{
						getNSpaces(4);
						if (firstBlockStatement) this->nSpacesInitial = nSpaces; firstBlockStatement = false;
						tokens.push_back({ .type = TokenType::ELIF, .statement = statementType {.nSpaces = nSpaces } });
					}
					else if (buf == "else")
					{
						getNSpaces(4);
						if (firstBlockStatement) this->nSpacesInitial = nSpaces; firstBlockStatement = false;
						tokens.push_back({ .type = TokenType::ELSE, .statement = statementType {.nSpaces = nSpaces } });
					}
					else if (buf == "for")
					{
						getNSpaces(3);
						if (firstBlockStatement) this->nSpacesInitial = nSpaces; firstBlockStatement = false;
						tokens.push_back({ .type = TokenType::FOR, .statement = statementType {.nSpaces = nSpaces } });
					}
					else if (buf == "while")
					{
						getNSpaces(5);
						if (firstBlockStatement) this->nSpacesInitial = nSpaces; firstBlockStatement = false;
						tokens.push_back({ .type = TokenType::WHILE, .statement = statementType {.nSpaces = nSpaces } });
					}
					else if (buf == "in")
					{
						tokens.push_back({ .type = TokenType::IN });
					}
					else if (buf == "True")
					{
						tokens.push_back({ .type = TokenType::BOOL, .value = "1" });
					}
					else if (buf == "False")
					{
						tokens.push_back({ .type = TokenType::BOOL, .value = "0" });
					}
					else if (buf == "def")
					{
						tokens.push_back({ .type = TokenType::DEF });
					}
					else
					{
						tokens.push_back({ .type = TokenType::IDENT });
					}
				}
				// Numbers
				else if (std::isdigit(this->peek().value()) ||
					(this->peek().value() == '.' && (this->peek(1).has_value() && std::isdigit(this->peek(1).value()))) ||
					(this->peek().value() == '-' && ((this->peek(1).has_value() && std::isdigit(this->peek(1).value())) || (this->peek(1).has_value() && this->peek(1).value() == '.'))))
				{
					buf.push_back(this->consume());

					while (std::isdigit(this->peek().value()) || this->peek().value() == '.')
					{
						buf.push_back(this->consume());
					}
					size_t nDots = std::count(buf.begin(), buf.end(), '.');
					if (nDots > 1)
					{
						std::cerr << "Invalid syntax" << std::endl;
						exit(1);
					}
					// Float
					else if (nDots)
					{
						double num = std::stod(buf);
						tokens.push_back({ .type = TokenType::FLOAT, .value = std::bitset<64>(reinterpret_cast<unsigned long long&>(num)).to_string() + 'b' });
					}
					// Int
					else
					{
						tokens.push_back({ .type = TokenType::INT, .value = buf });
					}
				}
				// Strings
				else if (this->peek().value() == '\"' || this->peek().value() == '\'')
				{
					this->consume();

					while (this->peek().has_value() && this->peek().value() != '\"' && this->peek().value() != '\'')
					{
						buf.push_back(this->consume());
					}
					this->consume();
					tokens.push_back({ .type = TokenType::STRING, .value = buf });
				}
				else if (this->peek().value() == '.')
				{
					this->consume();
					tokens.push_back({ .type = TokenType::DOT });
				}
				else if (this->peek().value() == ',')
				{
					this->consume();
					tokens.push_back({ .type = TokenType::COMMA });
				}
				else if (this->peek().value() == '(')
				{
					this->consume();
					tokens.push_back({ .type = TokenType::OPEN_PAREN });
				}
				else if (this->peek().value() == ')')
				{
					this->consume();
					tokens.push_back({ .type = TokenType::CLOSE_PAREN });
				}
				else if (this->peek().value() == '[')
				{
					this->consume();
					tokens.push_back({ .type = TokenType::OPEN_SQUARE_BRACKETS });
				}
				else if (this->peek().value() == ']')
				{
					this->consume();
					tokens.push_back({ .type = TokenType::CLOSE_SQUARE_BRACKETS });
				}
				else if (this->peek().value() == ';')
				{
					this->consume();
					tokens.push_back({ .type = TokenType::SEMI });
				}
				else if (this->peek().value() == '=')
				{
					this->consume();
					if (this->peek().has_value() && this->peek().value() == '=')
					{
						this->consume();
						tokens.push_back({ .type = TokenType::EEQ });
					}
					else
					{
						tokens.push_back({ .type = TokenType::EQ });
					}
				}
				else if (this->peek().value() == '>')
				{
					this->consume();
					if (this->peek().has_value() && this->peek().value() == '=')
					{
						this->consume();
						tokens.push_back({ .type = TokenType::GE });
					}
					else
					{
						tokens.push_back({ .type = TokenType::GT });
					}
				}
				else if (this->peek().value() == '<')
				{
					this->consume();
					if (this->peek().has_value() && this->peek().value() == '=')
					{
						this->consume();
						tokens.push_back({ .type = TokenType::LE });
					}
					else
					{
						tokens.push_back({ .type = TokenType::LT });
					}
				}
				else if (this->peek().value() == '+')
				{
					this->consume();
					tokens.push_back({ .type = TokenType::ADD });
				}
				else if (this->peek().value() == '-')
				{
					this->consume();
					tokens.push_back({ .type = TokenType::SUB });
				}
				else if (this->peek().value() == '*')
				{
					this->consume();
					tokens.push_back({ .type = TokenType::MUL });
				}
				else if (this->peek().value() == '/')
				{
					this->consume();
					tokens.push_back({ .type = TokenType::DIV });
				}
				else if (this->peek().value() == '%')
				{
					this->consume();
					tokens.push_back({ .type = TokenType::REMAINDER });
				}
				else if (this->peek().value() == '@')
				{
					this->consume();
					tokens.push_back({ .type = TokenType::DOT_PRODUCT });
					}
				else if (this->peek().value() == ':')
				{
					this->consume();
					tokens.push_back({ .type = TokenType::SCOPE_START });
				}
				else if (this->peek().value() == ' ')
				{
					this->consume();
				}
				else if (this->peek().value() == '\n')
				{
					this->consume();

					size_t nSpaces = 0;
					while (this->peek().has_value() && this->peek().value() == ' ')
					{
						this->consume();
						nSpaces++;
					}

					for (const Token& token : std::views::reverse(tokens))
					{
						if (token.statement.has_value())
						{
							if (token.statement.value().closed && token.statement.value().nSpaces == this->nSpacesInitial)
							{
								break;
							}
							if (!token.statement.value().closed && token.statement.value().nSpaces <= nSpaces)
							{
								tokens.push_back({ .type = TokenType::SCOPE_END });
							}
						}
					}
				}
				else
				{
					std::cerr << "Error" << std::endl;
					exit(1);
				}
				buf.clear();
			}

			// Function scope
			tokens.push_back({ .type = TokenType::SCOPE_END });

			this->index = 0;
			return tokens;
		}

	private:

		inline std::optional<char> peek(size_t offset = 0)
		{
			if (this->index + offset >= this->src.size())
			{
				return {};
			}
			else
			{
				return this->src[this->index + offset];
			}
		}

		inline char consume()
		{
			return this->src[this->index++];
		}

		const std::string src;
		size_t nSpacesInitial;
		size_t index;
	};

}
