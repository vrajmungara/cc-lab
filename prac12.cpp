/*
 * ============================================================
 *  Code Optimization – Constant Folding
 *  Compiler Design Practical
 * ============================================================
 *
 *  Constant Folding:
 *   Identifies sub-expressions involving only constants at
 *   compile-time and replaces them with their computed value.
 *   Variables are left untouched in the expression.
 *
 *  Example:
 *   Input  : 5 + x - 3 * 2
 *   Output : 5 + x - 6        (3*2 folded to 6)
 *
 *  Compile:  g++ -std=c++17 -o folding constant_folding.cpp
 *  Run:      ./folding
 * ============================================================
 */

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cctype>
#include <cmath>
#include <sstream>
#include <iomanip>

using namespace std;

enum class TT {
    NUMBER, IDENT,
    PLUS, MINUS, STAR, SLASH,
    LPAREN, RPAREN, END
};

struct Token {
    TT     type;
    string raw;    
    double numval;   
    Token(TT t, const string& r, double n = 0)
        : type(t), raw(r), numval(n) {}
};

class Lexer {
    string src_;
    size_t pos_;
public:
    explicit Lexer(const string& s) : src_(s), pos_(0) {}

    vector<Token> tokenize() {
        vector<Token> toks;
        while (pos_ < src_.size()) {
            while (pos_ < src_.size() && isspace(src_[pos_])) ++pos_;
            if (pos_ >= src_.size()) break;
            char c = src_[pos_];
            if (isdigit(c) || c == '.') { toks.push_back(readNum()); continue; }
            if (isalpha(c) || c == '_') { toks.push_back(readIdent()); continue; }
            switch (c) {
                case '+': toks.emplace_back(TT::PLUS,   "+"); break;
                case '-': toks.emplace_back(TT::MINUS,  "-"); break;
                case '*': toks.emplace_back(TT::STAR,   "*"); break;
                case '/': toks.emplace_back(TT::SLASH,  "/"); break;
                case '(': toks.emplace_back(TT::LPAREN, "("); break;
                case ')': toks.emplace_back(TT::RPAREN, ")"); break;
                default:  throw runtime_error(string("Unknown char: ") + c);
            }
            ++pos_;
        }
        toks.emplace_back(TT::END, "$");
        return toks;
    }
private:
    Token readNum() {
        size_t s = pos_; bool dot = false;
        while (pos_ < src_.size() &&
               (isdigit(src_[pos_]) || (src_[pos_] == '.' && !dot))) {
            if (src_[pos_] == '.') dot = true; ++pos_;
        }
        string raw = src_.substr(s, pos_ - s);
        return Token(TT::NUMBER, raw, stod(raw));
    }
    Token readIdent() {
        size_t s = pos_;
        while (pos_ < src_.size() && (isalnum(src_[pos_]) || src_[pos_] == '_'))
            ++pos_;
        return Token(TT::IDENT, src_.substr(s, pos_ - s));
    }
};

struct Node {
    enum Kind { NUM, VAR, BINOP, UNARY } kind;
    char   op;         
    double numval;   
    string name;       
    Node*  left = nullptr;
    Node*  right = nullptr;

    static Node* num(double v) {
        auto* n = new Node; n->kind = NUM; n->numval = v; return n; }
    static Node* var(const string& s) {
        auto* n = new Node; n->kind = VAR; n->name = s; return n; }
    static Node* binop(char op, Node* l, Node* r) {
        auto* n = new Node; n->kind = BINOP; n->op = op;
        n->left = l; n->right = r; return n; }
    static Node* unary(char op, Node* child) {
        auto* n = new Node; n->kind = UNARY; n->op = op;
        n->left = child; return n; }
};

// ─────────────────────────────────────────────────────────────
//  PARSER  (builds AST)
class Parser {
    const vector<Token>& toks_;
    size_t pos_;
    const Token& cur() { return toks_[pos_]; }
    Token consume() { return toks_[pos_++]; }
    void expect(TT t, const string& m) {
        if (cur().type != t) throw runtime_error(m); consume(); }
public:
    explicit Parser(const vector<Token>& t) : toks_(t), pos_(0) {}

    Node* parse() {
        Node* r = expr();
        if (cur().type != TT::END) throw runtime_error("Unexpected token.");
        return r;
    }

    // E → T ((+|-) T)*
    Node* expr() {
        Node* n = term();
        while (cur().type == TT::PLUS || cur().type == TT::MINUS) {
            char op = consume().raw[0];
            n = Node::binop(op, n, term());
        }
        return n;
    }

    // T → F ((*|/) F)*
    Node* term() {
        Node* n = factor();
        while (cur().type == TT::STAR || cur().type == TT::SLASH) {
            char op = consume().raw[0];
            n = Node::binop(op, n, factor());
        }
        return n;
    }

    // F → ( E ) | -F | number | ident
    Node* factor() {
        if (cur().type == TT::LPAREN) {
            consume();
            Node* n = expr();
            expect(TT::RPAREN, "Expected ')'");
            return n;
        }
        if (cur().type == TT::MINUS) {
            consume();
            return Node::unary('-', factor());
        }
        if (cur().type == TT::NUMBER) {
            double v = cur().numval; consume();
            return Node::num(v);
        }
        if (cur().type == TT::IDENT) {
            string nm = cur().raw; consume();
            return Node::var(nm);
        }
        throw runtime_error("Unexpected token in factor.");
    }
};

// ─────────────────────────────────────────────────────────────
//  CONSTANT FOLDING  (recursive tree walk)
//  Returns true if the sub-tree is a pure constant (no vars)
bool fold(Node*& n) {
    if (!n) return false;
    if (n->kind == Node::NUM) return true;
    if (n->kind == Node::VAR) return false;

    if (n->kind == Node::UNARY) {
        bool c = fold(n->left);
        if (c) {
            double v = (n->op == '-') ? -n->left->numval : n->left->numval;
            delete n->left;
            n->kind = Node::NUM; n->numval = v; n->left = nullptr;
            return true;
        }
        return false;
    }

    bool lc = fold(n->left);
    bool rc = fold(n->right);

    if (lc && rc) {
        double lv = n->left->numval, rv = n->right->numval, res = 0;
        switch (n->op) {
            case '+': res = lv + rv; break;
            case '-': res = lv - rv; break;
            case '*': res = lv * rv; break;
            case '/':
                if (rv == 0) throw runtime_error("Division by zero.");
                res = lv / rv; break;
        }
        delete n->left; delete n->right;
        n->kind = Node::NUM; n->numval = res;
        n->left = n->right = nullptr;
        return true;
    }
    return false;
}


// Format a double: if whole number print as int, else fixed 4dp
string fmtNum(double v) {
    if (v == floor(v) && fabs(v) < 1e12) {
        ostringstream os; os << (long long)v; return os.str();
    }
    ostringstream os; os << fixed << setprecision(4) << v;
    // trim trailing zeros
    string s = os.str();
    s.erase(s.find_last_not_of('0') + 1);
    if (s.back() == '.') s.pop_back();
    return s;
}

// precedence of an operator
int prec(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

string toStr(const Node* n, int parentPrec = 0) {
    if (!n) return "";
    if (n->kind == Node::NUM) return fmtNum(n->numval);
    if (n->kind == Node::VAR) return n->name;

    if (n->kind == Node::UNARY) {
        string s = "-" + toStr(n->left, 3);
        return (parentPrec > 2) ? "(" + s + ")" : s;
    }

    int p = prec(n->op);
    string ls = toStr(n->left,  p);
    string rs = toStr(n->right, p + 1);  // +1 forces parens for same-prec right child
    string s = ls + " " + n->op + " " + rs;
    return (parentPrec > p) ? "(" + s + ")" : s;
}

void freeTree(Node* n) {
    if (!n) return;
    freeTree(n->left);
    freeTree(n->right);
    delete n;
}


int main() {
    cout << "============================================================\n";
    cout << "  Code Optimization - Constant Folding\n";
    cout << "============================================================\n";
    cout << "  Operands  : numbers (3, 2.5) or variables (x, abc)\n";
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
        size_t e = trimmed.find_last_not_of(" \t\r\n");
        trimmed = trimmed.substr(0, e + 1);

        if (trimmed == "exit" || trimmed == "quit") {
            cout << "  Goodbye!\n"; break;
        }

        try {
            Lexer lexer(trimmed);
            vector<Token> tokens = lexer.tokenize();

            Parser parser(tokens);
            Node* root = parser.parse();

            fold(root);

            string optimized = toStr(root);
            freeTree(root);

            cout << "  Optimized : " << optimized << "\n";

        } catch (const exception& ex) {
            cout << "  Error: " << ex.what() << "\n";
        }
    }

    return 0;
}
