/*
 * ============================================================
 *  Arithmetic Expression Evaluator – Bottom-Up Parsing (SDD)
 *  Compiler Design Practical
 * ============================================================
 *
 *  Grammar (with SDD semantic rules):
 *
 *   L  → E n        { Print(E.val) }
 *   E  → E + T      { E.val = E.val + T.val }
 *   E  → E - T      { E.val = E.val - T.val }
 *   E  → T          { E.val = T.val }
 *   T  → T * F      { T.val = T.val * F.val }
 *   T  → T / F      { T.val = T.val / F.val }
 *   T  → F          { T.val = F.val }
 *   F  → G ^ F      { F.val = G.val ^ F.val }  (right-assoc)
 *   F  → G          { F.val = G.val }
 *   G  → ( E )      { G.val = E.val }
 *   G  → digit      { G.val = digit.lexval }
 *
 *  Operator Precedence (low → high):
 *   + -  <  * /  <  ^  <  unary  <  ( )
 *
 *  Compile:  g++ -std=c++17 -o evaluator arithmetic_evaluator.cpp
 *  Run:      ./evaluator
 * ============================================================
 */

#include <iostream>
#include <string>
#include <stack>
#include <sstream>
#include <cmath>
#include <cctype>
#include <stdexcept>
#include <vector>
#include <iomanip>

using namespace std;

enum class TokenType {
    NUMBER, PLUS, MINUS, STAR, SLASH, CARET,
    LPAREN, RPAREN, END, INVALID
};

struct Token {
    TokenType type;
    double    value;   // used when type == NUMBER
    char      ch;      // used for operator tokens (display)

    Token(TokenType t, double v = 0.0, char c = '\0')
        : type(t), value(v), ch(c) {}
};

// for tokenization of the input string into a sequence of tokens
class Lexer {
public:
    explicit Lexer(const string& input)
        : src_(input), pos_(0) {}

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
                    case '+': tokens.emplace_back(TokenType::PLUS,   0.0, '+'); break;
                    case '-': tokens.emplace_back(TokenType::MINUS,  0.0, '-'); break;
                    case '*': tokens.emplace_back(TokenType::STAR,   0.0, '*'); break;
                    case '/': tokens.emplace_back(TokenType::SLASH,  0.0, '/'); break;
                    case '^': tokens.emplace_back(TokenType::CARET,  0.0, '^'); break;
                    case '(': tokens.emplace_back(TokenType::LPAREN, 0.0, '('); break;
                    case ')': tokens.emplace_back(TokenType::RPAREN, 0.0, ')'); break;
                    default:
                        throw runtime_error(
                            string("Invalid character: '") + c + "'");
                }
                ++pos_;
            }
        }
        tokens.emplace_back(TokenType::END);
        return tokens;
    }

private:
    string src_;
    size_t      pos_;

    void skipWhitespace() {
        while (pos_ < src_.size() && isspace(src_[pos_]))
            ++pos_;
    }

    Token readNumber() {
        size_t start = pos_;
        bool hasDot = false;
        while (pos_ < src_.size() &&
               (isdigit(src_[pos_]) || (src_[pos_] == '.' && !hasDot))) {
            if (src_[pos_] == '.') hasDot = true;
            ++pos_;
        }
        double val = stod(src_.substr(start, pos_ - start));
        return Token(TokenType::NUMBER, val, '#');
    }
};

// ─────────────────────────────────────────────────────────────
//  PARSER / EVALUATOR  (Recursive Descent, Bottom-Up semantics)
//
//  Implements the SDD by attaching semantic values (.val) to
//  each grammar symbol as the parse tree is built upward.
//
//  Precedence levels mirror the grammar exactly:
//   parseExpression  → handles E  (+/-)
//   parseTerm        → handles T  (*/)
//   parseFactor      → handles F  (^, right-assoc)
//   parsePrimary     → handles G  (digit | parenthesised E)
// ─────────────────────────────────────────────────────────────
class Parser {
public:
    explicit Parser(const vector<Token>& tokens)
        : tokens_(tokens), pos_(0) {}

    // Entry point: L → E  (returns E.val)
    double parse() {
        double result = parseExpression();
        if (current().type != TokenType::END)
            throw runtime_error("Unexpected token after expression.");
        return result;
    }

private:
    const vector<Token>& tokens_;
    size_t pos_;

    const Token& current() const { return tokens_[pos_]; }

    Token consume() {
        Token t = tokens_[pos_];
        ++pos_;
        return t;
    }

    Token expect(TokenType type, const string& msg) {
        if (current().type != type)
            throw runtime_error(msg);
        return consume();
    }

    // ── E → E + T | E - T | T ────────────────────────────────
    //  SDD: E.val = E1.val ± T.val
    double parseExpression() {
        double val = parseTerm();    // E.val ← T.val  (E → T)

        while (current().type == TokenType::PLUS ||
               current().type == TokenType::MINUS) {
            TokenType op = current().type;
            consume();
            double tval = parseTerm();
            if (op == TokenType::PLUS)
                val = val + tval;   // E.val = E.val + T.val
            else
                val = val - tval;   // E.val = E.val - T.val
        }
        return val;
    }

    // ── T → T * F | T / F | F ────────────────────────────────
    //  SDD: T.val = T1.val */ F.val
    double parseTerm() {
        double val = parseFactor();  // T.val ← F.val  (T → F)

        while (current().type == TokenType::STAR ||
               current().type == TokenType::SLASH) {
            TokenType op = current().type;
            consume();
            double fval = parseFactor();
            if (op == TokenType::STAR) {
                val = val * fval;   // T.val = T.val * F.val
            } else {
                if (fval == 0.0)
                    throw runtime_error("Division by zero.");
                val = val / fval;   // T.val = T.val / F.val
            }
        }
        return val;
    }

    // ── F → G ^ F | G ────────────────────────────────────────
    //  SDD: F.val = G.val ^ F.val   (right-associative)
    double parseFactor() {
        double base = parsePrimary();  // G.val

        if (current().type == TokenType::CARET) {
            consume();
            double exp = parseFactor();   // right-recursive: F → G ^ F
            return pow(base, exp);   // F.val = G.val ^ F.val
        }
        return base;  // F.val = G.val  (F → G)
    }

    // ── G → ( E ) | digit ────────────────────────────────────
    //  SDD: G.val = E.val  OR  G.val = digit.lexval
    double parsePrimary() {
        const Token& tok = current();

        // G → digit
        if (tok.type == TokenType::NUMBER) {
            consume();
            return tok.value;   // G.val = digit.lexval
        }

        // G → ( E )
        if (tok.type == TokenType::LPAREN) {
            consume();   // eat '('
            double val = parseExpression();   // G.val = E.val
            expect(TokenType::RPAREN,
                   "Missing closing parenthesis ')'.");
            return val;
        }

        // Unary minus support:  -(expr)
        if (tok.type == TokenType::MINUS) {
            consume();
            return -parseFactor();
        }

        throw runtime_error("Unexpected token in expression.");
    }
};

string evaluate(const string& expr) {
    // Trim
    string trimmed = expr;
    size_t s = trimmed.find_first_not_of(" \t\r\n");
    if (s == string::npos) return "Invalid expression";
    trimmed = trimmed.substr(s);
    size_t e = trimmed.find_last_not_of(" \t\r\n");
    trimmed = trimmed.substr(0, e + 1);

    try {
        Lexer lexer(trimmed);
        vector<Token> tokens = lexer.tokenize();

        Parser parser(tokens);
        double result = parser.parse();

        // Format: if result is an integer value, show without decimal
        ostringstream oss;
        if (result == floor(result) && abs(result) < 1e15)
            oss << static_cast<long long>(result);
        else
            oss << setprecision(10) << result;
        return oss.str();

    } catch (const exception& ex) {
        return "Invalid expression";
    }
}

// ─────────────────────────────────────────────────────────────
//  DISPLAY HELPERS
// ─────────────────────────────────────────────────────────────
void printSeparator(int w = 60) {
    cout << string(w, '-') << "\n";
}

void printHeader() {
    printSeparator();
    cout
        << "  Arithmetic Expression Evaluator\n"
        << "  Bottom-Up Parsing with SDD Semantic Rules\n"
        << "  Compiler Design Practical\n";
    printSeparator();
    cout
        << "  Supported operators : +  -  *  /  ^  ( )\n"
        << "  Exponentiation (^)  : right-associative\n"
        << "  Operands            : integers and decimals\n"
        << "  Type 'exit' or 'quit' to stop\n";
    printSeparator();
}

// ─────────────────────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────────────────────
int main() {
    printHeader();

    string line;
    while (true) {
        cout << "  Enter expression: ";
        if (!getline(cin, line)) break;

        string trimmed = line;
        size_t s = trimmed.find_first_not_of(" \t\r\n");
        if (s == string::npos) continue;
        trimmed = trimmed.substr(s);

        if (trimmed == "exit" || trimmed == "quit") {
            cout << "  Goodbye!\n";
            break;
        }

        cout << "  Result = " << evaluate(trimmed) << "\n\n";
    }

    return 0;
}
