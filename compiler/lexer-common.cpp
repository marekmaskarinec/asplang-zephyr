//
// Asp lexical analyzer implementation - common.
//

#include <lexer.h>
#include <token-types.h>
#include <cstdint>
#include <cctype>
#include <map>
#include <algorithm>
#include <limits>
#include <cstdlib>
#include <cerrno>

using namespace std;

// Keywords, used and reserved.
map<string, int> Lexer::keywords =
{
    {"and", TOKEN_AND},
    {"as", TOKEN_AS},
    {"assert", TOKEN_ASSERT},
    {"break", TOKEN_BREAK},
    {"class", TOKEN_CLASS},
    {"continue", TOKEN_CONTINUE},
    {"def", TOKEN_DEF},
    {"del", TOKEN_DEL},
    {"elif", TOKEN_ELIF},
    {"else", TOKEN_ELSE},
    {"except", TOKEN_EXCEPT},
    {"exec", TOKEN_EXEC},
    {"finally", TOKEN_FINALLY},
    {"for", TOKEN_FOR},
    {"from", TOKEN_FROM},
    {"global", TOKEN_GLOBAL},
    {"if", TOKEN_IF},
    {"import", TOKEN_IMPORT},
    {"in", TOKEN_IN},
    {"is", TOKEN_IS},
    {"lambda", TOKEN_LAMBDA},
    {"local", TOKEN_LOCAL},
    {"nonlocal", TOKEN_NONLOCAL},
    {"not", TOKEN_NOT},
    {"or", TOKEN_OR},
    {"pass", TOKEN_PASS},
    {"raise", TOKEN_RAISE},
    {"return", TOKEN_RETURN},
    {"try", TOKEN_TRY},
    {"while", TOKEN_WHILE},
    {"with", TOKEN_WITH},
    {"yield", TOKEN_YIELD},
    {"False", TOKEN_FALSE},
    {"None", TOKEN_NONE},
    {"True", TOKEN_TRUE},
};

Token *Lexer::ProcessLineContinuation()
{
    string lex;
    lex += static_cast<char>(Get()); // backslash

    // Allow trailing whitespace only if followed by a comment.
    bool trailingSpace = false;
    int c;
    while (c = Peek(), isspace(c) && c != '\n')
    {
        trailingSpace = true;
        lex += static_cast<char>(Get());
    }
    if (c == '#')
    {
        trailingSpace = false;
        ProcessComment();
    }
    Get(); // newline

    return !trailingSpace ? nullptr : new Token(sourceLocation, -1, lex);
}

Token *Lexer::ProcessComment()
{
    int c;
    while (c = Peek(), c != '\n' && c != EOF)
        Get();
    return nullptr; // no token
}

Token *Lexer::ProcessStatementEnd()
{
    Get(); // ; or newline
    return new Token(sourceLocation, TOKEN_STATEMENT_END);
}

Token *Lexer::ProcessNumber()
{
    enum class State
    {
        Null,
        Zero,
        Decimal,
        DecimalSeparator,
        IncompleteBinary,
        Binary,
        BinarySeparator,
        IncompleteHexadecimal,
        Hexadecimal,
        HexadecimalSeparator,
        IncompleteSimpleFloat,
        WholeFloat,
        SimpleFloat,
        SimpleFloatSeparator,
        IncompleteExponential,
        IncompleteSignedExponential,
        Exponential,
        ExponentialSeparator,
        Invalid,
    } state = State::Null;
    string lex;
    while (!is.eof())
    {
        auto c = Peek();

        bool end = false;
        switch (state)
        {
            case State::Null:
                if (c == '0')
                    state = State::Zero;
                else if (c == '.')
                    state = State::IncompleteSimpleFloat;
                else if (isdigit(c))
                    state = State::Decimal;
                else
                    state = State::Invalid;
                break;

            case State::Zero:
                if (tolower(c) == 'x')
                    state = State::IncompleteHexadecimal;
                else if (tolower(c) == 'b')
                    state = State::IncompleteBinary;
                else if (c == '.')
                {
                    if (Peek(1) == '.')
                        end = true;
                    else
                        state = State::SimpleFloat;
                }
                else if (tolower(c) == 'e')
                    state = State::IncompleteExponential;
                else if (isdigit(c))
                    state = State::Decimal;
                else if (isalpha(c))
                    state = State::Invalid;
                else
                    end = true;
                break;

            case State::Decimal:
                if (c == '.')
                {
                    if (Peek(1) == '.')
                        end = true;
                    else
                        state = State::WholeFloat;
                }
                else if (tolower(c) == 'e')
                    state = State::IncompleteExponential;
                else if (isalpha(c))
                    state = State::Invalid;
                else if (c == '_')
                    state = State::DecimalSeparator;
                else if (!isdigit(c))
                    end = true;
                break;

            case State::DecimalSeparator:
                if (isdigit(c))
                    state = State::Decimal;
                else
                    state = State::Invalid;
                break;

            case State::IncompleteBinary:
                if (c == '0' || c == '1')
                    state = State::Binary;
                else
                    state = State::Invalid;
                break;

            case State::Binary:
                if (c != '0' && c != '1')
                {
                    if (isalnum(c))
                        state = State::Invalid;
                    else if (c == '_')
                        state = State::BinarySeparator;
                    else if (c == '.')
                    {
                        if (Peek(1) == '.')
                            end = true;
                        else
                            state = State::Invalid;
                    }
                    else
                        end = true;
                }
                break;

            case State::BinarySeparator:
                if (c == '0' || c == '1')
                    state = State::Binary;
                else
                    state = State::Invalid;
                break;

            case State::IncompleteHexadecimal:
                if (isxdigit(c))
                    state = State::Hexadecimal;
                else
                    state = State::Invalid;
                break;

            case State::Hexadecimal:
                if (!isxdigit(c))
                {
                    if (isalpha(c))
                        state = State::Invalid;
                    else if (c == '_')
                        state = State::HexadecimalSeparator;
                    else if (c == '.')
                    {
                        if (Peek(1) == '.')
                            end = true;
                        else
                            state = State::Invalid;
                    }
                    else
                        end = true;
                }
                break;

            case State::HexadecimalSeparator:
                if (isxdigit(c))
                    state = State::Hexadecimal;
                else
                    state = State::Invalid;
                break;

            case State::IncompleteSimpleFloat:
                if (isdigit(c))
                    state = State::SimpleFloat;
                else
                    state = State::Invalid;
                break;

            case State::WholeFloat:
            case State::SimpleFloat:
                if (tolower(c) == 'e')
                    state = State::IncompleteExponential;
                else if (isalpha(c))
                    state = State::Invalid;
                else if (c == '_')
                {
                    if (state == State::SimpleFloat)
                        state = State::SimpleFloatSeparator;
                    else
                        state = State::Invalid;
                }
                else if (c == '.')
                {
                    if (Peek(1) == '.')
                        end = true;
                    else
                        state = State::Invalid;
                }
                else if (isdigit(c))
                    state = State::SimpleFloat;
                else
                    end = true;
                break;

            case State::SimpleFloatSeparator:
                if (isdigit(c))
                    state = State::SimpleFloat;
                else
                    state = State::Invalid;
                break;

            case State::IncompleteExponential:
                if (c == '+' || c == '-')
                    state = State::IncompleteSignedExponential;
                else if (isdigit(c))
                    state = State::Exponential;
                else
                    state = State::Invalid;
                break;

            case State::IncompleteSignedExponential:
                if (isdigit(c))
                    state = State::Exponential;
                else
                    state = State::Invalid;
                break;

            case State::Exponential:
                if (isalpha(c))
                    state = State::Invalid;
                else if (c == '_')
                    state = State::ExponentialSeparator;
                else if (c == '.')
                {
                    if (Peek(1) == '.')
                        end = true;
                    else
                        state = State::Invalid;
                }
                else if (!isdigit(c))
                    end = true;
                break;

            case State::ExponentialSeparator:
                if (isdigit(c))
                    state = State::Exponential;
                else
                    state = State::Invalid;
                break;
        }

        if (end)
            break;

        if (c != '\n')
            lex += static_cast<char>(c);

        if (state == State::Invalid)
            break;

        Get();
    }

    // Remove any separators.
    auto cleanLex = lex;
    if (state != State::Invalid)
        cleanLex.erase
            (remove(cleanLex.begin(), cleanLex.end(), '_'),
             cleanLex.end());

    const auto s = cleanLex.c_str();
    switch (state)
    {
        case State::Zero:
        case State::Decimal:
        {
            static const uint32_t max =
                static_cast<uint32_t>(INT32_MAX) + 1U;
            errno = 0;
            auto ulValue = strtoul(s, nullptr, 10);
            auto uValue = static_cast<uint32_t>(ulValue);
            return
                errno != 0 || ulValue > max ?
                new Token
                    (sourceLocation, -1, lex,
                     "Decimal constant out of range") :
                new Token
                    (sourceLocation,
                     *reinterpret_cast<int32_t *>(&uValue),
                     uValue == max, lex);
        }

        case State::Binary:
        case State::Hexadecimal:
        {
            errno = 0;
            auto ulValue = strtoul
                (s + 2, nullptr, state == State::Binary ? 2 : 0x10);
            auto uValue = static_cast<uint32_t>(ulValue);
            return
                errno != 0 || ulValue > UINT32_MAX ?
                new Token
                    (sourceLocation, -1, lex,
                     string
                        (state == State::Binary ? "Binary" : "Hexadecimal") +
                     " constant out of range") :
                new Token
                    (sourceLocation,
                     *reinterpret_cast<int32_t *>(&uValue),
                     false, lex);
        }

        case State::SimpleFloat:
        case State::WholeFloat:
        case State::Exponential:
            return new Token(sourceLocation, strtod(s, nullptr), lex);
    }

    return new Token(sourceLocation, -1, lex);
}

Token *Lexer::ProcessString()
{
    string lex, escapeLex;

    auto quote = static_cast<char>(Get());

    enum class State
    {
        End,
        Normal,
        CharacterEscape,
        DecimalEscape,
        HexadecimalEscape,
        HexadecimalEscapeDigits,
    } state = State::Normal;

    bool badString = false;
    while (!badString && state != State::End)
    {
        int ci = Get();
        if (ci == EOF)
            break;
        auto c = static_cast<char>(ci);

        switch (state)
        {
            default: // No escape sequence
                if (c == '\n')
                    badString = true;
                else if (c == '\\')
                {
                    // Look ahead to determine next state.
                    int c2 = Peek();
                    if (c2 == EOF)
                        badString = true;
                    else if (isdigit(c2))
                        state = State::DecimalEscape;
                    else if (c2 == 'x')
                        state = State::HexadecimalEscape;
                    else
                        state = State::CharacterEscape;
                }
                else if (c == quote)
                    state = State::End;
                else
                    lex += c;
                break;

            case State::CharacterEscape:
                switch (c)
                {
                    case 'a':
                        c = '\a';
                        break;
                    case 'b':
                        c = '\b';
                        break;
                    case 'f':
                        c = '\f';
                        break;
                    case 'n':
                    case '\n':
                        c = '\n';
                        break;
                    case 'r':
                        c = '\r';
                        break;
                    case 't':
                        c = '\t';
                        break;
                    case 'v':
                        c = '\v';
                        break;
                    case '\\':
                        c = '\\';
                        break;
                    case '"':
                        c = '"';
                        break;
                    case '\'':
                        c = '\'';
                        break;
                    default:
                        badString = true;
                        break;
                }
                lex += c;
                state = State::Normal;
                break;

            case State::DecimalEscape:
            {
                escapeLex += c;
                auto c2 = static_cast<char>(Peek());
                if (escapeLex.size() == 3 || !isdigit(c2))
                {
                    int value = atoi(escapeLex.c_str());
                    if (value > 0xFF)
                        badString = true;
                    else
                    {
                        lex += static_cast<char>(value);
                        escapeLex.clear();
                        state = State::Normal;
                    }
                }
                break;
            }

            case State::HexadecimalEscape:
                state = State::HexadecimalEscapeDigits;
                break;

            case State::HexadecimalEscapeDigits:
                escapeLex += c;
                if (!isxdigit(c))
                    badString = true;
                else if (escapeLex.size() == 2)
                {
                    lex += static_cast<char>
                        (strtol(escapeLex.c_str(), nullptr, 16));
                    escapeLex.clear();
                    state = State::Normal;
                }
                break;
        }
    }

    if (state != State::End)
        return new Token
            (sourceLocation, -1, !escapeLex.empty() ? escapeLex : lex);

    return new Token(sourceLocation, lex);
}

Token *Lexer::ProcessName()
{
    string lex;
    while (!is.eof())
    {
        auto c = Peek();
        if (!isalpha(c) && !isdigit(c) && c != '_')
            break;
        lex += static_cast<char>(c);
        Get();
    }

    // Extract keywords.
    auto iter = keywords.find(lex);
    return
        iter != keywords.end() ?
        new Token(sourceLocation, iter->second, lex) :
        new Token(sourceLocation, TOKEN_NAME, lex);
}

int Lexer::Peek(unsigned n)
{
    if (n < prefetch.size())
        return prefetch[n];

    int c = EOF;
    for (n -= static_cast<unsigned>(prefetch.size()), n++;
         !is.eof() && n > 0; n--)
    {
        c = Read();
        prefetch.push_back(c);
    }

    return c;
}

int Lexer::Read()
{
    int c;
    if (!readahead.empty())
    {
        c = readahead.front();
        readahead.pop_front();
        if (c == EOF)
            return c;
    }
    else
        c = is.get();

    // Ensure the last character ends a line.
    if (c == EOF)
    {
        readahead.push_back(c);
        c = '\n';
    }

    return c;
}
