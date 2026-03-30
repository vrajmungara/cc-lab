/*
 * ============================================================
 *  Intermediate Code Generator – Quadruple Table
 *  Compiler Design Practical
 * ============================================================
 *
 *  Grammar:
 *   E → E + T | E - T | T
 *   T → T * F | T / F | F
 *   F → ( E ) | digit
 *
 *  Each operation produces a quadruple:
 *   ( operator, operand1, operand2, result )
 *
 *  Temporaries are named t1, t2, t3, ...
 *
 *  Compile:  g++ -std=c++17 -o quadruple quadruple_generator.cpp
 *  Run:      ./quadruple
 * ============================================================
 */

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <iomanip>

using namespace std;

enum class TokenType {
    NUMBER, PLUS, MINUS, STAR, SLASH,
    LPAREN, RPAREN, END, INVALID
};

struct Token {
    TokenType type;
    string    lexval;   // raw text (number string or operator char)

    Token(TokenType t, const string& v = "")
        : type(t), lexval(v) {}
};

class Lexer {
public:
    explicit Lexer(const string& input) : src_(input), pos_(0) {}

    vector<Token> tokenize() {
        vector<Token> tokens;
        while (pos_ < src_.size()) {
            skipWhitespace();
            if (pos_ >= src_.size()) break;

            char c = src_[pos_];

            if (isdigit(c) || c == '.') {
                tokens.push_back(readNumber());
            } else {
                switch (c) {
                    case '+': tokens.emplace_back(TokenType::PLUS,   "+"); break;
                    case '-': tokens.emplace_back(TokenType::MINUS,  "-"); break;
                    case '*': tokens.emplace_back(TokenType::STAR,   "*"); break;
                    case '/': tokens.emplace_back(TokenType::SLASH,  "/"); break;
                    case '(': tokens.emplace_back(TokenType::LPAREN, "("); break;
                    case ')': tokens.emplace_back(TokenType::RPAREN, ")"); break;
                    default:
                        throw runtime_error(
                            string("Invalid character: '") + c + "'");
                }
                ++pos_;
            }
        }
        tokens.emplace_back(TokenType::END, "$");
        return tokens;
    }

private:
    string src_;
    size_t pos_;

    void skipWhitespace() {
        while (pos_ < src_.size() && isspace(src_[pos_])) ++pos_;
    }

    Token readNumber() {
        size_t start = pos_;
        bool hasDot = false;
        while (pos_ < src_.size() &&
               (isdigit(src_[pos_]) || (src_[pos_] == '.' && !hasDot))) {
            if (src_[pos_] == '.') hasDot = true;
            ++pos_;
        }
        return Token(TokenType::NUMBER, src_.substr(start, pos_ - start));
    }
};

// ─────────────────────────────────────────────────────────────
//  QUADRUPLE
// ─────────────────────────────────────────────────────────────
struct Quadruple {
    string op;
    string operand1;
    string operand2;
    string result;
};

class Parser {
public:
    explicit Parser(const vector<Token>& tokens)
        : tokens_(tokens), pos_(0), tempCount_(0) {}

    // Parse and fill quadruple table; returns final result name
    string parse(vector<Quadruple>& quads) {
        quads_ = &quads;
        string result = parseExpression();
        if (current().type != TokenType::END)
            throw runtime_error("Unexpected token after expression.");
        return result;
    }

private:
    const vector<Token>& tokens_;
    size_t               pos_;
    int                  tempCount_;
    vector<Quadruple>*   quads_;

    // ── Helpers ──────────────────────────────────────────────
    const Token& current() const { return tokens_[pos_]; }

    Token consume() {
        Token t = tokens_[pos_++];
        return t;
    }

    void expect(TokenType type, const string& msg) {
        if (current().type != type)
            throw runtime_error(msg);
        consume();
    }

    string newTemp() {
        return "t" + to_string(++tempCount_);
    }

    void emit(const string& op,
              const string& op1,
              const string& op2,
              const string& res) {
        quads_->push_back({op, op1, op2, res});
    }

    // ── E → E + T | E - T | T ────────────────────────────────
    string parseExpression() {
        string left = parseTerm();

        while (current().type == TokenType::PLUS ||
               current().type == TokenType::MINUS) {
            string op = consume().lexval;   // '+' or '-'
            string right = parseTerm();
            string tmp = newTemp();
            emit(op, left, right, tmp);
            left = tmp;
        }
        return left;
    }

    // ── T → T * F | T / F | F ────────────────────────────────
    string parseTerm() {
        string left = parseFactor();

        while (current().type == TokenType::STAR ||
               current().type == TokenType::SLASH) {
            string op = consume().lexval;   // '*' or '/'
            string right = parseFactor();
            string tmp = newTemp();
            emit(op, left, right, tmp);
            left = tmp;
        }
        return left;
    }

    // ── F → ( E ) | digit ────────────────────────────────────
    string parseFactor() {
        const Token& tok = current();

        if (tok.type == TokenType::NUMBER) {
            consume();
            return tok.lexval;   // raw number string as operand
        }

        if (tok.type == TokenType::LPAREN) {
            consume();                // eat '('
            string val = parseExpression();
            expect(TokenType::RPAREN, "Missing closing parenthesis ')'.");
            return val;
        }

        // Unary minus
        if (tok.type == TokenType::MINUS) {
            consume();
            string operand = parseFactor();
            string tmp = newTemp();
            emit("uminus", operand, "-", tmp);
            return tmp;
        }

        throw runtime_error("Unexpected token in expression.");
    }
};

// ─────────────────────────────────────────────────────────────
//  PRINT QUADRUPLE TABLE
// ─────────────────────────────────────────────────────────────
void printTable(const vector<Quadruple>& quads) {
    const int W1 = 10, W2 = 12, W3 = 12, W4 = 10;
    string sep(W1 + W2 + W3 + W4 + 5, '-');

    cout << "\n  " << sep << "\n";
    cout << "  "
         << left << setw(W1) << "Operator"
         << left << setw(W2) << "Operand 1"
         << left << setw(W3) << "Operand 2"
         << left << setw(W4) << "Result"
         << "\n";
    cout << "  " << sep << "\n";

    for (const auto& q : quads) {
        cout << "  "
             << left << setw(W1) << q.op
             << left << setw(W2) << q.operand1
             << left << setw(W3) << q.operand2
             << left << setw(W4) << q.result
             << "\n";
    }
    cout << "  " << sep << "\n";
}

bool generate(const string& expr) {
    // Trim whitespace
    size_t s = expr.find_first_not_of(" \t\r\n");
    if (s == string::npos) { cout << "  Empty input.\n"; return false; }
    string trimmed = expr.substr(s);
    size_t e = trimmed.find_last_not_of(" \t\r\n");
    trimmed = trimmed.substr(0, e + 1);

    try {
        Lexer lexer(trimmed);
        vector<Token> tokens = lexer.tokenize();

        vector<Quadruple> quads;
        Parser parser(tokens);
        string finalResult = parser.parse(quads);

        printTable(quads);
        return true;

    } catch (const exception& ex) {
        cout << "  Invalid expression: " << ex.what() << "\n";
        return false;
    }
}

int main() {
    cout << "============================================================\n";
    cout << "  Intermediate Code Generator  –  Quadruple Table\n";
    cout << "  Compiler Design Practical\n";
    cout << "============================================================\n";
    cout << "  Operators : +  -  *  /  ( )\n";
    cout << "  Type 'exit' or 'quit' to stop.\n";
    cout << "============================================================\n";

    string line;
    while (true) {
        cout << "\n  Enter expression: ";
        if (!getline(cin, line)) break;

        size_t s = line.find_first_not_of(" \t\r\n");
        if (s == string::npos) continue;
        string trimmed = line.substr(s);

        if (trimmed == "exit" || trimmed == "quit") {
            cout << "  Thank You!\n";
            break;
        }

        generate(trimmed);
    }

    return 0;
}
