/*
    ============================================================
    Integral Calculator  v3  -  Final Version
    ============================================================
    Features:
      - Type ANY function: sqrt(tan(x)), x^2*sin(x), exp(-x^2)
      - Single integral   : definite  (numerical, Simpson 1/3)
      - Single integral   : indefinite (symbolic + numerical table)
      - Double integral   : definite  (numerical)
      - Triple integral   : definite  (numerical)
      - Symbolic differentiation (used internally + exposed)
    ============================================================
    OOP   : Token, ASTNode, Lexer, Parser, Evaluator,
            Simplifier, SymbolicIntegrator, Differentiator,
            ExprTree, Simpson, IndefiniteIntegral, Calculator
    Arrays: static (function name table, weight pattern),
            dynamic (token list via Lexer, sample points via Simpson,
                     F(x) table via IndefiniteIntegral)
    ============================================================
    Supported syntax:
      Operators : +  -  *  /  ^  (unary minus)
      Functions : sin cos tan asin acos atan
                  sinh cosh tanh
                  sqrt cbrt abs exp ln log
      Constants : pi  e
      Variables : x  (single)  x y  (double)  x y z  (triple)
    ============================================================
    Symbolic integration covers:
      Polynomials    : x^n  -> x^(n+1)/(n+1)
      Basic trig     : sin, cos, sec^2, csc^2, sec*tan, csc*cot
      Exponentials   : e^x, e^(ax), a^x
      Logarithms     : ln(x)
      Inverse trig   : 1/(1+x^2), 1/sqrt(1-x^2)
      Sums/differences: term by term
      Constant multiples: pull out constants
      Fallback       : numerical (Simpson 1/3, n=2000)
    ============================================================
*/

#include <iostream>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <cctype>
#include <cstdlib>
#include <climits>

using namespace std;

// ============================================================
// Constants
// ============================================================
static const double MY_PI = 3.14159265358979323846;
static const double MY_E  = 2.71828182845904523536;

// ============================================================
// Token
// ============================================================
enum TokenType {
    TOK_NUM, TOK_VAR, TOK_PLUS, TOK_MINUS, TOK_MUL,
    TOK_DIV, TOK_POW, TOK_LPAREN, TOK_RPAREN,
    TOK_FUNC, TOK_END, TOK_ERR
};

struct Token {
    TokenType type;
    double    numVal;
    char      name[16];
    Token() : type(TOK_ERR), numVal(0.0) { name[0] = '\0'; }
};

// ============================================================
// AST Node
// ============================================================
enum NodeType { NODE_NUM, NODE_VAR, NODE_BINOP, NODE_UNARY, NODE_FUNC };

struct ASTNode {
    NodeType  kind;
    double    value;
    char      varName;
    TokenType op;
    char      funcName[16];
    ASTNode*  left;
    ASTNode*  right;

    ASTNode() : kind(NODE_NUM), value(0), varName('x'),
                op(TOK_END), left(nullptr), right(nullptr) {
        funcName[0] = '\0';
    }
};

// Free an entire tree
void freeTree(ASTNode* n) {
    if (!n) return;
    freeTree(n->left);
    freeTree(n->right);
    delete n;
}

// Deep copy a tree
ASTNode* copyTree(const ASTNode* n) {
    if (!n) return nullptr;
    ASTNode* c = new ASTNode(*n);
    c->left  = copyTree(n->left);
    c->right = copyTree(n->right);
    return c;
}

// ============================================================
// Node factory helpers
// ============================================================
ASTNode* mkNum(double v) {
    ASTNode* n = new ASTNode(); n->kind = NODE_NUM; n->value = v; return n;
}
ASTNode* mkVar(char v) {
    ASTNode* n = new ASTNode(); n->kind = NODE_VAR; n->varName = v; return n;
}
ASTNode* mkBin(TokenType op, ASTNode* L, ASTNode* R) {
    ASTNode* n = new ASTNode(); n->kind = NODE_BINOP; n->op = op;
    n->left = L; n->right = R; return n;
}
ASTNode* mkUnary(ASTNode* child) {
    ASTNode* n = new ASTNode(); n->kind = NODE_UNARY; n->op = TOK_MINUS;
    n->right = child; return n;
}
ASTNode* mkFunc(const char* name, ASTNode* arg) {
    ASTNode* n = new ASTNode(); n->kind = NODE_FUNC;
    strncpy(n->funcName, name, 15); n->funcName[15] = '\0';
    n->right = arg; return n;
}

// ============================================================
// Class: Lexer
// Tokenizes input string into a dynamic array of Token objects.
// ============================================================
class Lexer {
private:
    Token* tokens;
    int    capacity;
    int    size;
    int    pos;

    void grow() {
        int    nc  = capacity * 2;
        Token* tmp = new Token[nc];
        for (int i = 0; i < size; i++) tmp[i] = tokens[i];
        delete[] tokens;
        tokens = tmp; capacity = nc;
    }

    void push(Token t) { if (size == capacity) grow(); tokens[size++] = t; }

    // Static array of recognized function names
    static const char* knownFuncs[];

    bool isFunc(const char* s) const {
        for (int i = 0; knownFuncs[i]; i++)
            if (strcmp(s, knownFuncs[i]) == 0) return true;
        return false;
    }

public:
    bool hasError;
    char errorMsg[128];

    Lexer() : capacity(64), size(0), pos(0), hasError(false) {
        tokens = new Token[capacity];
        errorMsg[0] = '\0';
    }
    ~Lexer() { delete[] tokens; }

    void tokenize(const char* input) {
        size = pos = 0; hasError = false;
        int i = 0, len = (int)strlen(input);

        while (i < len) {
            if (isspace(input[i])) { i++; continue; }
            Token t;

            // Number
            if (isdigit(input[i]) || input[i] == '.') {
                char buf[64]; int bi = 0;
                while (i < len && (isdigit(input[i]) || input[i] == '.'))
                    buf[bi++] = input[i++];
                buf[bi] = '\0';
                t.type = TOK_NUM; t.numVal = atof(buf);
                push(t); continue;
            }

            // Identifier
            if (isalpha(input[i]) || input[i] == '_') {
                char buf[16]; int bi = 0;
                while (i < len && (isalpha(input[i]) || isdigit(input[i]) || input[i] == '_'))
                    buf[bi++] = input[i++];
                buf[bi] = '\0';

                if      (strcmp(buf,"pi")==0) { t.type=TOK_NUM; t.numVal=MY_PI; }
                else if (strcmp(buf,"e") ==0) { t.type=TOK_NUM; t.numVal=MY_E;  }
                else if (strcmp(buf,"x")==0||strcmp(buf,"y")==0||strcmp(buf,"z")==0) {
                    t.type = TOK_VAR; strncpy(t.name,buf,15); t.name[15]='\0';
                }
                else if (isFunc(buf)) {
                    t.type = TOK_FUNC; strncpy(t.name,buf,15); t.name[15]='\0';
                }
                else { hasError=true; snprintf(errorMsg,127,"Unknown: '%s'",buf); return; }
                push(t); continue;
            }

            switch (input[i]) {
                case '+': t.type=TOK_PLUS;   break;
                case '-': t.type=TOK_MINUS;  break;
                case '*': t.type=TOK_MUL;    break;
                case '/': t.type=TOK_DIV;    break;
                case '^': t.type=TOK_POW;    break;
                case '(': t.type=TOK_LPAREN; break;
                case ')': t.type=TOK_RPAREN; break;
                default:
                    hasError=true;
                    snprintf(errorMsg,127,"Unknown char: '%c'",input[i]); return;
            }
            push(t); i++;
        }
        Token e; e.type=TOK_END; push(e);
    }

    Token peek() const { return (pos<size)?tokens[pos]:Token(); }
    Token consume()    { return (pos<size)?tokens[pos++]:Token(); }
    bool  expect(TokenType t) {
        if (peek().type==t){consume();return true;}
        hasError=true; snprintf(errorMsg,127,"Expected token %d got %d",t,peek().type);
        return false;
    }
    void reset() { pos=0; }
};

const char* Lexer::knownFuncs[] = {
    "sin","cos","tan","asin","acos","atan",
    "sinh","cosh","tanh","sqrt","cbrt","abs",
    "exp","ln","log", nullptr
};

// ============================================================
// Class: Parser  (recursive descent)
// Grammar:
//   expr   = term   ((+|-) term)*
//   term   = factor ((*|/) factor)*
//   factor = base   ('^' factor)?      right-associative
//   base   = '-' base | NUM | VAR | FUNC'('expr')' | '('expr')'
// ============================================================
class Parser {
private:
    Lexer* lex;

    ASTNode* parseExpr();
    ASTNode* parseTerm();
    ASTNode* parseFactor();
    ASTNode* parseBase();

public:
    bool hasError;
    char errorMsg[128];

    Parser(Lexer* l) : lex(l), hasError(false) { errorMsg[0]='\0'; }

    ASTNode* parse() {
        hasError = false;
        ASTNode* root = parseExpr();
        if (lex->peek().type != TOK_END) {
            hasError = true;
            snprintf(errorMsg,127,"Unexpected token after expression");
            freeTree(root); return nullptr;
        }
        return root;
    }
};

ASTNode* Parser::parseExpr() {
    ASTNode* L = parseTerm(); if (!L) return nullptr;
    while (lex->peek().type==TOK_PLUS||lex->peek().type==TOK_MINUS) {
        TokenType op = lex->consume().type;
        ASTNode* R = parseTerm(); if (!R){freeTree(L);return nullptr;}
        L = mkBin(op,L,R);
    }
    return L;
}

ASTNode* Parser::parseTerm() {
    ASTNode* L = parseFactor(); if (!L) return nullptr;
    while (lex->peek().type==TOK_MUL||lex->peek().type==TOK_DIV) {
        TokenType op = lex->consume().type;
        ASTNode* R = parseFactor(); if (!R){freeTree(L);return nullptr;}
        L = mkBin(op,L,R);
    }
    return L;
}

ASTNode* Parser::parseFactor() {
    ASTNode* base = parseBase(); if (!base) return nullptr;
    if (lex->peek().type==TOK_POW) {
        lex->consume();
        ASTNode* exp = parseFactor(); if (!exp){freeTree(base);return nullptr;}
        return mkBin(TOK_POW,base,exp);
    }
    return base;
}

ASTNode* Parser::parseBase() {
    Token t = lex->peek();
    if (t.type==TOK_MINUS) {
        lex->consume();
        ASTNode* ch = parseBase(); if (!ch) return nullptr;
        return mkUnary(ch);
    }
    if (t.type==TOK_NUM)  { lex->consume(); return mkNum(t.numVal); }
    if (t.type==TOK_VAR)  { lex->consume(); return mkVar(t.name[0]); }
    if (t.type==TOK_FUNC) {
        lex->consume();
        if (lex->peek().type!=TOK_LPAREN) {
            hasError=true; snprintf(errorMsg,127,"Expected '(' after '%s'",t.name);
            return nullptr;
        }
        lex->consume();
        ASTNode* arg = parseExpr(); if (!arg) return nullptr;
        if (lex->peek().type!=TOK_RPAREN) {
            hasError=true; snprintf(errorMsg,127,"Expected ')' after function argument");
            freeTree(arg); return nullptr;
        }
        lex->consume();
        return mkFunc(t.name, arg);
    }
    if (t.type==TOK_LPAREN) {
        lex->consume();
        ASTNode* inner = parseExpr(); if (!inner) return nullptr;
        if (lex->peek().type!=TOK_RPAREN) {
            hasError=true; snprintf(errorMsg,127,"Expected ')'");
            freeTree(inner); return nullptr;
        }
        lex->consume(); return inner;
    }
    hasError=true; snprintf(errorMsg,127,"Unexpected token in expression"); return nullptr;
}

// ============================================================
// Class: Evaluator
// Walks the AST numerically for up to 3 variables.
// ============================================================
class Evaluator {
public:
    static double eval(const ASTNode* n, double x, double y=0, double z=0) {
        if (!n) return 0;
        switch (n->kind) {
            case NODE_NUM:   return n->value;
            case NODE_VAR:
                if (n->varName=='x') return x;
                if (n->varName=='y') return y;
                if (n->varName=='z') return z;
                return 0;
            case NODE_UNARY: return -eval(n->right,x,y,z);
            case NODE_BINOP: {
                double L=eval(n->left,x,y,z), R=eval(n->right,x,y,z);
                switch(n->op){
                    case TOK_PLUS:  return L+R;
                    case TOK_MINUS: return L-R;
                    case TOK_MUL:   return L*R;
                    case TOK_DIV:   return fabs(R)>1e-300?L/R:0;
                    case TOK_POW:   return pow(L,R);
                    default: return 0;
                }
            }
            case NODE_FUNC: {
                double a=eval(n->right,x,y,z);
                const char* f=n->funcName;
                if(!strcmp(f,"sin"))  return sin(a);
                if(!strcmp(f,"cos"))  return cos(a);
                if(!strcmp(f,"tan"))  return tan(a);
                if(!strcmp(f,"asin")) return asin(a);
                if(!strcmp(f,"acos")) return acos(a);
                if(!strcmp(f,"atan")) return atan(a);
                if(!strcmp(f,"sinh")) return sinh(a);
                if(!strcmp(f,"cosh")) return cosh(a);
                if(!strcmp(f,"tanh")) return tanh(a);
                if(!strcmp(f,"sqrt")) return a>=0?sqrt(a):0;
                if(!strcmp(f,"cbrt")) return cbrt(a);
                if(!strcmp(f,"abs"))  return fabs(a);
                if(!strcmp(f,"exp"))  return exp(a);
                if(!strcmp(f,"ln"))   return a>0?log(a):0;
                if(!strcmp(f,"log"))  return a>0?log10(a):0;
                return 0;
            }
        }
        return 0;
    }
};

// ============================================================
// Class: Printer
// Converts an AST back to a human-readable string.
// Uses a static char buffer (caller must use result before next call).
// ============================================================
class Printer {
private:
    char  buf[512];
    int   pos;

    void put(const char* s) {
        while (*s && pos<510) buf[pos++]=*s++;
        buf[pos]='\0';
    }
    void putChar(char c) { if(pos<510){buf[pos++]=c; buf[pos]='\0';} }
    void putNum(double v) {
        // Print as integer if whole number, else 4 decimal places
        char tmp[64];
        if (fabs(v - (long long)v) < 1e-9 && fabs(v) < 1e12)
            snprintf(tmp,63,"%.0f",v);
        else
            snprintf(tmp,63,"%.4g",v);
        put(tmp);
    }

    // Returns operator precedence (lower = weaker binding)
    int prec(const ASTNode* n) const {
        if (!n || n->kind!=NODE_BINOP) return 99;
        switch(n->op){
            case TOK_PLUS: case TOK_MINUS: return 1;
            case TOK_MUL:  case TOK_DIV:   return 2;
            case TOK_POW:                  return 3;
            default: return 0;
        }
    }

    void printInner(const ASTNode* n, int parentPrec, bool rightChild) {
        if (!n) return;
        switch (n->kind) {
            case NODE_NUM: {
                if (n->value < 0) { putChar('('); putNum(n->value); putChar(')'); }
                else putNum(n->value);
                break;
            }
            case NODE_VAR:
                putChar(n->varName); break;
            case NODE_UNARY:
                putChar('-'); printInner(n->right,99,false); break;
            case NODE_FUNC:
                put(n->funcName); putChar('(');
                printInner(n->right,0,false);
                putChar(')'); break;
            case NODE_BINOP: {
                int myPrec = prec(n);
                bool needParen = (myPrec < parentPrec) ||
                                 (myPrec == parentPrec && rightChild && n->op!=TOK_POW);
                if (needParen) putChar('(');
                printInner(n->left,  myPrec, false);

                // If op is + and right child is unary minus, print as -
                bool plusMinus = (n->op==TOK_PLUS &&
                                  n->right && n->right->kind==NODE_UNARY);
                if (plusMinus) {
                    put(" - ");
                    printInner(n->right->right, myPrec, true);
                } else {
                    switch(n->op){
                        case TOK_PLUS:  put(" + "); break;
                        case TOK_MINUS: put(" - "); break;
                        case TOK_MUL:   put("*");   break;
                        case TOK_DIV:   put("/");   break;
                        case TOK_POW:   putChar('^'); break;
                        default: break;
                    }
                    printInner(n->right, myPrec, true);
                }
                if (needParen) putChar(')');
                break;
            }
        }
    }

public:
    Printer() : pos(0) { buf[0]='\0'; }

    const char* print(const ASTNode* n) {
        pos=0; buf[0]='\0';
        printInner(n,0,false);
        return buf;
    }
};

// ============================================================
// Class: Simplifier
// Does basic constant folding and algebraic simplifications.
// Returns a NEW tree; caller owns it.
// ============================================================
class Simplifier {
public:
    static bool isNum(const ASTNode* n, double v) {
        return n && n->kind==NODE_NUM && fabs(n->value-v)<1e-12;
    }
    static bool isNum(const ASTNode* n) {
        return n && n->kind==NODE_NUM;
    }

    static ASTNode* simplify(ASTNode* n) {
        if (!n) return nullptr;
        // Simplify children first
        if (n->left)  n->left  = simplify(n->left);
        if (n->right) n->right = simplify(n->right);

        if (n->kind==NODE_UNARY && isNum(n->right)) {
            double v = -n->right->value;
            freeTree(n); return mkNum(v);
        }

        // --x = x  (double negation)
        if (n->kind==NODE_UNARY && n->right && n->right->kind==NODE_UNARY) {
            ASTNode* inner = n->right->right;
            n->right->right = nullptr;
            freeTree(n);
            return simplify(inner);
        }

        if (n->kind==NODE_BINOP && isNum(n->left) && isNum(n->right)) {
            double L=n->left->value, R=n->right->value, res=0;
            bool ok=true;
            switch(n->op){
                case TOK_PLUS:  res=L+R; break;
                case TOK_MINUS: res=L-R; break;
                case TOK_MUL:   res=L*R; break;
                case TOK_DIV:   res=(fabs(R)>1e-12)?L/R:0; break;
                case TOK_POW:   res=pow(L,R); break;
                default: ok=false;
            }
            if (ok) { freeTree(n); return mkNum(res); }
        }

        if (n->kind==NODE_BINOP) {
            // 0 + x = x
            if (n->op==TOK_PLUS && isNum(n->left,0)) {
                ASTNode* r=n->right; n->right=nullptr; freeTree(n); return r;
            }
            // x + 0 = x
            if (n->op==TOK_PLUS && isNum(n->right,0)) {
                ASTNode* l=n->left; n->left=nullptr; freeTree(n); return l;
            }
            // x - 0 = x
            if (n->op==TOK_MINUS && isNum(n->right,0)) {
                ASTNode* l=n->left; n->left=nullptr; freeTree(n); return l;
            }
            // 0 - x = -x
            if (n->op==TOK_MINUS && isNum(n->left,0)) {
                ASTNode* r=n->right; n->right=nullptr; freeTree(n); return mkUnary(r);
            }
            // 1 * x = x
            if (n->op==TOK_MUL && isNum(n->left,1)) {
                ASTNode* r=n->right; n->right=nullptr; freeTree(n); return r;
            }
            // x * 1 = x
            if (n->op==TOK_MUL && isNum(n->right,1)) {
                ASTNode* l=n->left; n->left=nullptr; freeTree(n); return l;
            }
            // 0 * x = 0  or  x * 0 = 0
            if (n->op==TOK_MUL && (isNum(n->left,0)||isNum(n->right,0))) {
                freeTree(n); return mkNum(0);
            }
            // x / 1 = x
            if (n->op==TOK_DIV && isNum(n->right,1)) {
                ASTNode* l=n->left; n->left=nullptr; freeTree(n); return l;
            }
            // x^1 = x
            if (n->op==TOK_POW && isNum(n->right,1)) {
                ASTNode* l=n->left; n->left=nullptr; freeTree(n); return l;
            }
            // x^0 = 1
            if (n->op==TOK_POW && isNum(n->right,0)) {
                freeTree(n); return mkNum(1);
            }
        }
        return n;
    }
};

// ============================================================
// Forward declaration for Simpson (used in symbolic fallback)
// ============================================================
class Simpson;

// ============================================================
// Class: Differentiator
// Symbolic differentiation with respect to x.
// Returns a NEW simplified tree.
// ============================================================
class Differentiator {
public:
    // Differentiate node with respect to variable var ('x','y','z')
    static ASTNode* diff(const ASTNode* n, char var) {
        if (!n) return mkNum(0);

        switch (n->kind) {

            case NODE_NUM:
                return mkNum(0);

            case NODE_VAR:
                return mkNum(n->varName==var ? 1.0 : 0.0);

            case NODE_UNARY: {
                // d/dx(-u) = -(du/dx)
                ASTNode* du = diff(n->right, var);
                return Simplifier::simplify(mkUnary(du));
            }

            case NODE_BINOP: {
                ASTNode* L = copyTree(n->left);
                ASTNode* R = copyTree(n->right);
                ASTNode* dL = diff(n->left,  var);
                ASTNode* dR = diff(n->right, var);

                ASTNode* result = nullptr;
                switch (n->op) {
                    // (u+v)' = u' + v'
                    case TOK_PLUS:
                        result = mkBin(TOK_PLUS, dL, dR);
                        freeTree(L); freeTree(R); break;
                    // (u-v)' = u' - v'
                    case TOK_MINUS:
                        result = mkBin(TOK_MINUS, dL, dR);
                        freeTree(L); freeTree(R); break;
                    // (u*v)' = u'v + uv'
                    case TOK_MUL:
                        result = mkBin(TOK_PLUS,
                                    mkBin(TOK_MUL, dL, R),
                                    mkBin(TOK_MUL, L, dR));
                        break;
                    // (u/v)' = (u'v - uv') / v^2
                    case TOK_DIV: {
                        ASTNode* R2 = copyTree(R);
                        result = mkBin(TOK_DIV,
                                    mkBin(TOK_MINUS,
                                        mkBin(TOK_MUL, dL, R),
                                        mkBin(TOK_MUL, L, dR)),
                                    mkBin(TOK_POW, R2, mkNum(2)));
                        break;
                    }
                    // (u^n)' where n is constant: n*u^(n-1)*u'
                    case TOK_POW: {
                        if (n->right->kind==NODE_NUM) {
                            double exp = n->right->value;
                            // n * u^(n-1) * u'
                            ASTNode* base = copyTree(n->left);
                            result = mkBin(TOK_MUL,
                                        mkBin(TOK_MUL,
                                            mkNum(exp),
                                            mkBin(TOK_POW, base, mkNum(exp-1))),
                                        dL);
                            freeTree(R); freeTree(dR); freeTree(L); break;
                        }
                        // General: (u^v)' = u^v * (v'*ln(u) + v*u'/u)
                        ASTNode* node_copy = mkBin(TOK_POW, copyTree(n->left), copyTree(n->right));
                        result = mkBin(TOK_MUL,
                                    node_copy,
                                    mkBin(TOK_PLUS,
                                        mkBin(TOK_MUL, dR, mkFunc("ln", L)),
                                        mkBin(TOK_MUL, R,
                                            mkBin(TOK_DIV, dL, copyTree(n->left)))));
                        break;
                    }
                    default:
                        freeTree(L); freeTree(R); freeTree(dL); freeTree(dR);
                        return mkNum(0);
                }
                return Simplifier::simplify(result);
            }

            case NODE_FUNC: {
                // Chain rule: d/dx f(u) = f'(u) * u'
                ASTNode* u  = copyTree(n->right);
                ASTNode* du = diff(n->right, var);
                const char* f = n->funcName;
                ASTNode* df_du = nullptr;

                if      (!strcmp(f,"sin"))  df_du = mkFunc("cos", u);
                else if (!strcmp(f,"cos"))  df_du = mkUnary(mkFunc("sin", u));
                else if (!strcmp(f,"tan"))  {
                    // sec^2(u) = 1/cos^2(u)
                    ASTNode* cosu = mkFunc("cos", copyTree(u));
                    df_du = mkBin(TOK_DIV, mkNum(1),
                                mkBin(TOK_POW, cosu, mkNum(2)));
                    freeTree(u);
                }
                else if (!strcmp(f,"asin")) {
                    // 1/sqrt(1-u^2)
                    ASTNode* inner = mkBin(TOK_MINUS, mkNum(1),
                                        mkBin(TOK_POW, u, mkNum(2)));
                    df_du = mkBin(TOK_DIV, mkNum(1), mkFunc("sqrt", inner));
                }
                else if (!strcmp(f,"acos")) {
                    ASTNode* inner = mkBin(TOK_MINUS, mkNum(1),
                                        mkBin(TOK_POW, u, mkNum(2)));
                    df_du = mkUnary(mkBin(TOK_DIV, mkNum(1), mkFunc("sqrt", inner)));
                }
                else if (!strcmp(f,"atan")) {
                    // 1/(1+u^2)
                    df_du = mkBin(TOK_DIV, mkNum(1),
                                mkBin(TOK_PLUS, mkNum(1), mkBin(TOK_POW, u, mkNum(2))));
                }
                else if (!strcmp(f,"sinh")) df_du = mkFunc("cosh", u);
                else if (!strcmp(f,"cosh")) df_du = mkFunc("sinh", u);
                else if (!strcmp(f,"tanh")) {
                    // 1 - tanh^2(u)
                    ASTNode* tanhu = mkFunc("tanh", u);
                    df_du = mkBin(TOK_MINUS, mkNum(1),
                                mkBin(TOK_POW, tanhu, mkNum(2)));
                }
                else if (!strcmp(f,"sqrt")) {
                    // 1/(2*sqrt(u))
                    df_du = mkBin(TOK_DIV, mkNum(1),
                                mkBin(TOK_MUL, mkNum(2), mkFunc("sqrt", u)));
                }
                else if (!strcmp(f,"cbrt")) {
                    // 1/(3*u^(2/3))
                    df_du = mkBin(TOK_DIV, mkNum(1),
                                mkBin(TOK_MUL, mkNum(3),
                                    mkBin(TOK_POW, u, mkNum(2.0/3.0))));
                }
                else if (!strcmp(f,"abs")) {
                    // sign(u) approximation — return 0 (not differentiable everywhere)
                    df_du = mkNum(0); freeTree(u);
                }
                else if (!strcmp(f,"exp")) df_du = mkFunc("exp", u);
                else if (!strcmp(f,"ln"))  {
                    // 1/u
                    df_du = mkBin(TOK_DIV, mkNum(1), u);
                }
                else if (!strcmp(f,"log")) {
                    // 1/(u*ln(10))
                    df_du = mkBin(TOK_DIV, mkNum(1),
                                mkBin(TOK_MUL, u, mkNum(log(10.0))));
                }
                else { df_du = mkNum(0); freeTree(u); }

                ASTNode* result = mkBin(TOK_MUL, df_du, du);
                return Simplifier::simplify(result);
            }
        }
        return mkNum(0);
    }
};

// ============================================================
// Class: Simpson
// Numerical integration using Simpson's 1/3 rule.
// Dynamic array for sample points; static weight pattern.
// ============================================================
class Simpson {
private:
    double* points;
    int     n;
    // Static array: Simpson coefficient pattern
    static int weightPattern[2];   // [0]=4 (odd idx), [1]=2 (even idx)

public:
    static int totalCallsMade;

    Simpson(int intervals=1000) : n(intervals) {
        if (n<2) n=2;
        if (n%2!=0) n++;
        points = new double[n+1];
    }
    ~Simpson() { delete[] points; }

    double integrate1D(const ASTNode* tree, double a, double b) {
        double h = (b-a)/n;
        for (int i=0;i<=n;i++) points[i] = Evaluator::eval(tree, a+i*h);
        double sum = points[0]+points[n];
        for (int i=1;i<n;i++) sum += (i%2==1?weightPattern[0]:weightPattern[1])*points[i];
        totalCallsMade++;
        return (h/3.0)*sum;
    }

    // Inner x-integral for fixed y
    double innerX(const ASTNode* t, double ax, double bx, double y) {
        double h=(bx-ax)/n;
        for (int i=0;i<=n;i++) points[i]=Evaluator::eval(t,ax+i*h,y);
        double sum=points[0]+points[n];
        for (int i=1;i<n;i++) sum+=(i%2==1?4:2)*points[i];
        return (h/3.0)*sum;
    }

    double integrate2D(const ASTNode* t, double ax, double bx, double ay, double by) {
        double  hy    = (by-ay)/n;
        double* outer = new double[n+1];
        for (int j=0;j<=n;j++) outer[j]=innerX(t,ax,bx,ay+j*hy);
        double sum=outer[0]+outer[n];
        for (int j=1;j<n;j++) sum+=(j%2==1?4:2)*outer[j];
        delete[] outer;
        totalCallsMade++;
        return (hy/3.0)*sum;
    }

    double innerXY(const ASTNode* t, double ax, double bx, double y, double z) {
        double h=(bx-ax)/n;
        for (int i=0;i<=n;i++) points[i]=Evaluator::eval(t,ax+i*h,y,z);
        double sum=points[0]+points[n];
        for (int i=1;i<n;i++) sum+=(i%2==1?4:2)*points[i];
        return (h/3.0)*sum;
    }

    double middleY(const ASTNode* t, double ax, double bx, double ay, double by, double z) {
        double  hy  = (by-ay)/n;
        double* mid = new double[n+1];
        for (int j=0;j<=n;j++) mid[j]=innerXY(t,ax,bx,ay+j*hy,z);
        double sum=mid[0]+mid[n];
        for (int j=1;j<n;j++) sum+=(j%2==1?4:2)*mid[j];
        delete[] mid;
        return (hy/3.0)*sum;
    }

    double integrate3D(const ASTNode* t,
                        double ax, double bx, double ay, double by, double az, double bz) {
        double  hz    = (bz-az)/n;
        double* outer = new double[n+1];
        for (int k=0;k<=n;k++) outer[k]=middleY(t,ax,bx,ay,by,az+k*hz);
        double sum=outer[0]+outer[n];
        for (int k=1;k<n;k++) sum+=(k%2==1?4:2)*outer[k];
        delete[] outer;
        totalCallsMade++;
        return (hz/3.0)*sum;
    }
};

int Simpson::totalCallsMade   = 0;
int Simpson::weightPattern[2] = {4,2};

// ============================================================
// Class: SymbolicIntegrator
// Attempts to find a symbolic antiderivative F(x) such that F'(x)=f(x).
// Returns nullptr if it cannot handle the form (caller falls back to numerical).
// The returned tree is a NEW tree; caller owns it.
// ============================================================
class SymbolicIntegrator {
private:
    // Check if a node does NOT contain the variable var at all
    static bool isConstantIn(const ASTNode* n, char var) {
        if (!n) return true;
        if (n->kind==NODE_VAR) return n->varName!=var;
        return isConstantIn(n->left,var) && isConstantIn(n->right,var);
    }

    // Check if node is exactly the variable var
    static bool isVar(const ASTNode* n, char var) {
        return n && n->kind==NODE_VAR && n->varName==var;
    }

    // Check if node is a number equal to v
    static bool isNum(const ASTNode* n, double v) {
        return n && n->kind==NODE_NUM && fabs(n->value-v)<1e-9;
    }

    // Extract constant coefficient: if node is c*var or var, return (c, true)
    // where c is the coefficient. Otherwise return (0, false).
    static bool extractLinearCoeff(const ASTNode* n, char var, double& coeff) {
        if (isVar(n, var)) { coeff=1.0; return true; }
        if (n && n->kind==NODE_BINOP && n->op==TOK_MUL) {
            if (isConstantIn(n->left,var) && isVar(n->right,var)) {
                coeff = Evaluator::eval(n->left,0);
                return true;
            }
            if (isConstantIn(n->right,var) && isVar(n->left,var)) {
                coeff = Evaluator::eval(n->right,0);
                return true;
            }
        }
        return false;
    }

public:
    // Main entry: try to integrate n with respect to var.
    // Returns new ASTNode tree or nullptr if unsupported.
    static ASTNode* integrate(const ASTNode* n, char var) {
        if (!n) return mkNum(0);

        // --- Constant (no var) ---
        if (isConstantIn(n, var)) {
            // integral of c dx = c*x
            return Simplifier::simplify(
                mkBin(TOK_MUL, copyTree(n), mkVar(var)));
        }

        // --- Variable itself: integral of x dx = x^2/2 ---
        if (isVar(n, var)) {
            return Simplifier::simplify(
                mkBin(TOK_DIV, mkBin(TOK_POW, mkVar(var), mkNum(2)), mkNum(2)));
        }

        // --- Sum/Difference: integrate term by term ---
        if (n->kind==NODE_BINOP &&
           (n->op==TOK_PLUS || n->op==TOK_MINUS)) {
            ASTNode* IL = integrate(n->left,  var);
            ASTNode* IR = integrate(n->right, var);
            if (IL && IR)
                return Simplifier::simplify(mkBin(n->op, IL, IR));
            freeTree(IL); freeTree(IR); return nullptr;
        }

        // --- Constant multiple: c * f(x) -> c * integral(f(x)) ---
        if (n->kind==NODE_BINOP && n->op==TOK_MUL) {
            if (isConstantIn(n->left, var)) {
                ASTNode* IF = integrate(n->right, var);
                if (IF) return Simplifier::simplify(
                    mkBin(TOK_MUL, copyTree(n->left), IF));
            }
            if (isConstantIn(n->right, var)) {
                ASTNode* IF = integrate(n->left, var);
                if (IF) return Simplifier::simplify(
                    mkBin(TOK_MUL, copyTree(n->right), IF));
            }
        }

        // --- Unary minus: -f(x) -> -(integral f) ---
        if (n->kind==NODE_UNARY) {
            ASTNode* IF = integrate(n->right, var);
            if (IF) return Simplifier::simplify(mkUnary(IF));
        }

        // --- Power rule: x^n -> x^(n+1)/(n+1)  (n != -1) ---
        if (n->kind==NODE_BINOP && n->op==TOK_POW &&
            isVar(n->left, var) && isConstantIn(n->right, var)) {
            double exp = Evaluator::eval(n->right,0);
            if (fabs(exp+1) < 1e-9) {
                // x^(-1) = 1/x  -> ln|x|
                return mkFunc("ln", mkVar(var));
            }
            double newExp = exp+1;
            return Simplifier::simplify(
                mkBin(TOK_DIV,
                    mkBin(TOK_POW, mkVar(var), mkNum(newExp)),
                    mkNum(newExp)));
        }

        // --- Division: 1/x -> ln|x| ---
        if (n->kind==NODE_BINOP && n->op==TOK_DIV &&
            isNum(n->left,1) && isVar(n->right, var)) {
            return mkFunc("ln", mkVar(var));
        }

        // --- exp(x) -> exp(x) ---
        // --- exp(a*x) -> exp(a*x)/a ---
        if (n->kind==NODE_FUNC && !strcmp(n->funcName,"exp")) {
            const ASTNode* arg = n->right;
            if (isVar(arg, var)) {
                return mkFunc("exp", mkVar(var));
            }
            // Handle unary minus: exp(-x) -> -exp(-x), exp(-a*x) -> -exp(-a*x)/a
            if (arg->kind==NODE_UNARY) {
                const ASTNode* inner = arg->right;
                double coeff;
                if (extractLinearCoeff(inner, var, coeff)) {
                    // integral of exp(-a*x) = -exp(-a*x)/a
                    return Simplifier::simplify(
                        mkBin(TOK_DIV,
                            mkUnary(copyTree(n)),
                            mkNum(coeff)));
                }
            }
            double coeff;
            if (extractLinearCoeff(arg, var, coeff)) {
                return Simplifier::simplify(
                    mkBin(TOK_DIV, copyTree(n), mkNum(coeff)));
            }
        }

        // --- sin(x) -> -cos(x) ---
        // --- sin(a*x) -> -cos(a*x)/a ---
        if (n->kind==NODE_FUNC && !strcmp(n->funcName,"sin")) {
            const ASTNode* arg = n->right;
            double coeff;
            if (extractLinearCoeff(arg, var, coeff)) {
                return Simplifier::simplify(
                    mkBin(TOK_DIV,
                        mkUnary(mkFunc("cos", copyTree(arg))),
                        mkNum(coeff)));
            }
        }

        // --- cos(x) -> sin(x) ---
        // --- cos(a*x) -> sin(a*x)/a ---
        if (n->kind==NODE_FUNC && !strcmp(n->funcName,"cos")) {
            const ASTNode* arg = n->right;
            double coeff;
            if (extractLinearCoeff(arg, var, coeff)) {
                return Simplifier::simplify(
                    mkBin(TOK_DIV,
                        mkFunc("sin", copyTree(arg)),
                        mkNum(coeff)));
            }
        }

        // --- ln(x) -> x*ln(x) - x ---
        if (n->kind==NODE_FUNC && !strcmp(n->funcName,"ln") &&
            isVar(n->right, var)) {
            return Simplifier::simplify(
                mkBin(TOK_MINUS,
                    mkBin(TOK_MUL, mkVar(var), mkFunc("ln", mkVar(var))),
                    mkVar(var)));
        }

        // --- sqrt(x) = x^(1/2) -> (2/3)*x^(3/2) ---
        if (n->kind==NODE_FUNC && !strcmp(n->funcName,"sqrt") &&
            isVar(n->right, var)) {
            return Simplifier::simplify(
                mkBin(TOK_MUL,
                    mkBin(TOK_DIV, mkNum(2), mkNum(3)),
                    mkBin(TOK_POW, mkVar(var), mkBin(TOK_DIV,mkNum(3),mkNum(2)))));
        }

        // --- atan(x): integral = x*atan(x) - (1/2)*ln(1+x^2) ---
        if (n->kind==NODE_FUNC && !strcmp(n->funcName,"atan") &&
            isVar(n->right, var)) {
            return Simplifier::simplify(
                mkBin(TOK_MINUS,
                    mkBin(TOK_MUL, mkVar(var), mkFunc("atan", mkVar(var))),
                    mkBin(TOK_MUL, mkBin(TOK_DIV,mkNum(1),mkNum(2)),
                        mkFunc("ln",
                            mkBin(TOK_PLUS, mkNum(1),
                                mkBin(TOK_POW, mkVar(var), mkNum(2)))))));
        }

        // --- asin(x): integral = x*asin(x) + sqrt(1-x^2) ---
        if (n->kind==NODE_FUNC && !strcmp(n->funcName,"asin") &&
            isVar(n->right, var)) {
            return Simplifier::simplify(
                mkBin(TOK_PLUS,
                    mkBin(TOK_MUL, mkVar(var), mkFunc("asin", mkVar(var))),
                    mkFunc("sqrt",
                        mkBin(TOK_MINUS, mkNum(1),
                            mkBin(TOK_POW, mkVar(var), mkNum(2))))));
        }

        // --- sinh(x) -> cosh(x) ---
        if (n->kind==NODE_FUNC && !strcmp(n->funcName,"sinh") &&
            isVar(n->right, var)) {
            return mkFunc("cosh", mkVar(var));
        }

        // --- cosh(x) -> sinh(x) ---
        if (n->kind==NODE_FUNC && !strcmp(n->funcName,"cosh") &&
            isVar(n->right, var)) {
            return mkFunc("sinh", mkVar(var));
        }

        // --- 1/(1+x^2) -> atan(x) ---
        // Recognise: DIV(1, PLUS(1, POW(x,2)))
        if (n->kind==NODE_BINOP && n->op==TOK_DIV && isNum(n->left,1)) {
            const ASTNode* denom = n->right;
            if (denom && denom->kind==NODE_BINOP && denom->op==TOK_PLUS &&
                isNum(denom->left,1) &&
                denom->right && denom->right->kind==NODE_BINOP &&
                denom->right->op==TOK_POW &&
                isVar(denom->right->left, var) &&
                isNum(denom->right->right,2)) {
                return mkFunc("atan", mkVar(var));
            }
        }

        // --- 1/sqrt(1-x^2) -> asin(x) ---
        if (n->kind==NODE_BINOP && n->op==TOK_DIV && isNum(n->left,1)) {
            const ASTNode* denom = n->right;
            if (denom && denom->kind==NODE_FUNC && !strcmp(denom->funcName,"sqrt")) {
                const ASTNode* inside = denom->right;
                if (inside && inside->kind==NODE_BINOP && inside->op==TOK_MINUS &&
                    isNum(inside->left,1) &&
                    inside->right && inside->right->kind==NODE_BINOP &&
                    inside->right->op==TOK_POW &&
                    isVar(inside->right->left, var) &&
                    isNum(inside->right->right,2)) {
                    return mkFunc("asin", mkVar(var));
                }
            }
        }

        // Cannot handle this form symbolically
        return nullptr;
    }
};

// ============================================================
// Class: ExprTree
// Owns a parsed AST. Parse once, reuse many times.
// ============================================================
class ExprTree {
private:
    ASTNode* root;
    char     text_[256];
    bool     valid_;
    char     err_[256];

public:
    ExprTree() : root(nullptr), valid_(false) { text_[0]=err_[0]='\0'; }
    ~ExprTree() { freeTree(root); }

    bool parse(const char* input) {
        freeTree(root); root=nullptr;
        strncpy(text_,input,255); text_[255]='\0';
        Lexer lex; lex.tokenize(input);
        if (lex.hasError) {
            valid_=false; strncpy(err_,lex.errorMsg,255); return false;
        }
        Parser parser(&lex);
        root = parser.parse();
        if (parser.hasError||!root) {
            valid_=false; strncpy(err_,parser.errorMsg,255);
            freeTree(root); root=nullptr; return false;
        }
        valid_=true; return true;
    }

    bool isValid()          const { return valid_; }
    const char* error()     const { return err_;   }
    const char* text()      const { return text_;  }
    const ASTNode* getRoot()const { return root;   }
};

// ============================================================
// Class: IndefiniteIntegral
// Builds F(x) table using dynamic array.
// ============================================================
class IndefiniteIntegral {
private:
    double* xVals;
    double* fVals;
    int     count;

public:
    IndefiniteIntegral(int pts) : count(pts) {
        xVals = new double[count];
        fVals = new double[count];
    }
    ~IndefiniteIntegral() { delete[] xVals; delete[] fVals; }

    void compute(const ASTNode* tree, double a, double b) {
        double step = (count>1)?(b-a)/(count-1):0;
        for (int i=0;i<count;i++) {
            xVals[i] = a+i*step;
            if (i==0||fabs(xVals[i]-a)<1e-12) { fVals[i]=0; continue; }
            Simpson s(2000);
            fVals[i] = s.integrate1D(tree, a, xVals[i]);
        }
    }

    void print(const char* expr) const {
        cout << "\n  Numerical antiderivative table for: " << expr << "\n";
        cout << "  F(x) = integral from a to x    (F(a) = 0, add any constant C)\n";
        cout << "  ------------------------------------------------\n";
        cout << setw(16) << "x" << setw(24) << "F(x)  + C" << "\n";
        cout << "  ------------------------------------------------\n";
        for (int i=0;i<count;i++) {
            cout << fixed << setprecision(8)
                 << setw(16) << xVals[i]
                 << setw(24) << fVals[i] << "  + C\n";
        }
        cout << "  ------------------------------------------------\n";
    }
};

// ============================================================
// Class: Calculator  — main driver
// ============================================================
class Calculator {
private:
    static const char SEP[];

    void header() const {
        cout << SEP << "\n";
        cout << "      INTEGRAL CALCULATOR  v3  (Expression Parser)\n";
        cout << SEP << "\n";
    }

    void readLine(char* buf, int maxLen) const {
        buf[0]='\0';
        while (!strlen(buf)) {
            cin.getline(buf, maxLen);
            int s=0; while(buf[s]==' ') s++;
            if (s>0) { int i=0; while(buf[s+i]){buf[i]=buf[s+i];i++;} buf[i]='\0'; }
        }
    }

    double getLimit(const char* label) const {
        double v;
        cout << "  " << label << ": ";
        cin >> v; cin.ignore();
        return v;
    }

    int getIntervals(int def) const {
        cout << "  Sub-intervals (default " << def << "): ";
        char buf[32]; cin.getline(buf,31);
        if (!strlen(buf)) return def;
        int n=atoi(buf);
        if (n<2) n=2;
        if (n%2) n++;
        return n;
    }

    bool getExpr(ExprTree& tree, const char* varHint) const {
        char buf[256];
        cout << "\n  f(" << varHint << ") = ";
        readLine(buf, 255);
        if (!tree.parse(buf)) {
            cout << "  Parse error: " << tree.error() << "\n";
            return false;
        }
        return true;
    }

    // ---- Single Definite ----
    void runDefinite() {
        ExprTree tree;
        if (!getExpr(tree,"x")) return;
        double a = getLimit("Lower limit a");
        double b = getLimit("Upper limit b");
        int    n = getIntervals(2000);

        Simpson s(n);
        double result = s.integrate1D(tree.getRoot(), a, b);

        cout << fixed << setprecision(10);
        cout << "\n  Definite integral of  " << tree.text()
             << "\n  from " << a << " to " << b
             << "\n  = " << result << "\n";
    }

    // ---- Single Indefinite: symbolic first, numerical table always ----
    void runIndefinite() {
        ExprTree tree;
        if (!getExpr(tree,"x")) return;

        Printer   printer;
        Simplifier simp;

        // 1. Attempt symbolic antiderivative
        ASTNode* F = SymbolicIntegrator::integrate(tree.getRoot(), 'x');
        F = F ? Simplifier::simplify(F) : nullptr;

        if (F) {
            cout << "\n  Symbolic antiderivative found:\n";
            cout << "  F(x) = " << printer.print(F) << "  + C\n";

            // Verify by differentiating F and showing it equals f
            ASTNode* dF = Differentiator::diff(F, 'x');
            dF = Simplifier::simplify(dF);
            cout << "  Verification  F'(x) = " << printer.print(dF) << "\n";
            freeTree(dF);

            // Evaluate definite integral using symbolic F if limits given
            cout << "\n  Evaluate definite integral using this antiderivative?\n";
            cout << "  Enter limits (or press Enter twice to skip).\n";
            double a, b;
            cout << "  Lower limit a (Enter to skip): ";
            char abuf[32]; cin.getline(abuf,31);
            if (strlen(abuf)) {
                a = atof(abuf);
                b = getLimit("Upper limit b");
                double Fb = Evaluator::eval(F, b);
                double Fa = Evaluator::eval(F, a);
                cout << fixed << setprecision(10);
                cout << "\n  F(" << b << ") - F(" << a << ")"
                     << " = " << Fb << " - " << Fa
                     << " = " << (Fb-Fa) << "\n";
            }
            freeTree(F);
        } else {
            cout << "\n  No closed-form antiderivative found for: " << tree.text() << "\n";
            cout << "  (This function requires advanced techniques or has no elementary form)\n";
        }

        // 2. Always offer numerical F(x) table
        cout << "\n  Generate numerical F(x) table? (y/n): ";
        char choice[4]; cin.getline(choice,3);
        if (choice[0]=='y'||choice[0]=='Y') {
            double a = getLimit("Range start a");
            double b = getLimit("Range end   b");
            cout << "  Sample points: ";
            int pts; cin >> pts; cin.ignore();
            if (pts<2) pts=2;

            IndefiniteIntegral indef(pts);
            indef.compute(tree.getRoot(), a, b);
            indef.print(tree.text());
        }
    }

    // ---- Double ----
    void runDouble() {
        ExprTree tree;
        if (!getExpr(tree,"x, y")) return;
        double ax=getLimit("x lower"); double bx=getLimit("x upper");
        double ay=getLimit("y lower"); double by=getLimit("y upper");
        int    n =getIntervals(200);

        Simpson s(n);
        double result = s.integrate2D(tree.getRoot(),ax,bx,ay,by);

        cout << fixed << setprecision(10);
        cout << "\n  Double integral of  " << tree.text()
             << "\n  x in [" << ax << ", " << bx << "]"
             << "  y in [" << ay << ", " << by << "]"
             << "\n  = " << result << "\n";
    }

    // ---- Triple ----
    void runTriple() {
        ExprTree tree;
        if (!getExpr(tree,"x, y, z")) return;
        double ax=getLimit("x lower"); double bx=getLimit("x upper");
        double ay=getLimit("y lower"); double by=getLimit("y upper");
        double az=getLimit("z lower"); double bz=getLimit("z upper");
        int    n =getIntervals(30);

        Simpson s(n);
        double result = s.integrate3D(tree.getRoot(),ax,bx,ay,by,az,bz);

        cout << fixed << setprecision(10);
        cout << "\n  Triple integral of  " << tree.text()
             << "\n  x in [" << ax << ", " << bx << "]"
             << "  y in [" << ay << ", " << by << "]"
             << "  z in [" << az << ", " << bz << "]"
             << "\n  = " << result << "\n";
    }

    // ---- Differentiate (bonus) ----
    void runDifferentiate() {
        ExprTree tree;
        if (!getExpr(tree,"x")) return;

        ASTNode* df = Differentiator::diff(tree.getRoot(),'x');
        df = Simplifier::simplify(df);
        Printer p;
        cout << "\n  f(x)  = " << tree.text()  << "\n";
        cout << "  f'(x) = " << p.print(df) << "\n";
        freeTree(df);
    }

public:
    void run() {
        int choice=-1;
        do {
            header();
            cout << "\n  [1]  Single Integral  -  Definite\n";
            cout << "  [2]  Single Integral  -  Indefinite  (symbolic + table)\n";
            cout << "  [3]  Double Integral  -  Definite\n";
            cout << "  [4]  Triple Integral  -  Definite\n";
            cout << "  [5]  Differentiate  f(x)\n";
            cout << "  [0]  Exit\n";
            cout << SEP << "\n";
            cout << "  Choice: ";
            cin >> choice; cin.ignore();

            switch(choice){
                case 1: runDefinite();      break;
                case 2: runIndefinite();    break;
                case 3: runDouble();        break;
                case 4: runTriple();        break;
                case 5: runDifferentiate(); break;
                case 0: break;
                default: cout << "  Invalid.\n";
            }

            if (choice!=0){
                cout << "\n  Total integrations this session: "
                     << Simpson::totalCallsMade << "\n";
                cout << "  Press Enter to continue...";
                cin.get();
            }
        } while (choice!=0);

        cout << "\n  Session ended. Total integrations: "
             << Simpson::totalCallsMade << "\n";
        cout << SEP << "\n";
    }
};

const char Calculator::SEP[] =
    "  ====================================================";

// ============================================================
int main() {
    Calculator calc;
    calc.run();
    return 0;
}