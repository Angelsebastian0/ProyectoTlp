#include <bits/stdc++.h>
using namespace std;

/* -------------------- Tokens & Lexer  -------------------- */

enum class TokenType {
    CLASS, EXTENDS, INT, STRING, BOOL, METHOD, METHODMAIN, PRINT,
    IF, ELSE, WHILE, FOR, RETURN,
    LBRACE, RBRACE, LBRACKET, RBRACKET, LPAREN, RPAREN,
    SEMICOLON, COMMA,
    EQUAL, PLUS, MINUS, STAR, SLASH,
    EQEQ, NOTEQ, LT, GT, LE, GE,
    ANDAND, OROR, NOT,
    STRING_LITERAL, NUMBER, IDENT, BOOLEAN_LITERAL, NULL_LITERAL,
    COMMENT,
    END_OF_FILE, UNKNOWN
};

struct Token {
    TokenType type;
    string lexeme;
    int line;
    int col;
};

string tokenTypeName(TokenType t) {
    switch(t){
        case TokenType::CLASS: return "CLASS";
        case TokenType::EXTENDS: return "EXTENDS";
        case TokenType::INT: return "INT";
        case TokenType::STRING: return "STRING";
        case TokenType::BOOL: return "BOOL";
        case TokenType::METHOD: return "METHOD";
        case TokenType::METHODMAIN: return "METHODMAIN";
        case TokenType::PRINT: return "PRINT";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::WHILE: return "WHILE";
        case TokenType::FOR: return "FOR";
        case TokenType::RETURN: return "RETURN";
        case TokenType::LBRACE: return "LBRACE";
        case TokenType::RBRACE: return "RBRACE";
        case TokenType::LBRACKET: return "LBRACKET";
        case TokenType::RBRACKET: return "RBRACKET";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::COMMA: return "COMMA";
        case TokenType::EQUAL: return "EQUAL";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::STAR: return "STAR";
        case TokenType::SLASH: return "SLASH";
        case TokenType::EQEQ: return "EQEQ";
        case TokenType::NOTEQ: return "NOTEQ";
        case TokenType::LT: return "LT";
        case TokenType::GT: return "GT";
        case TokenType::LE: return "LE";
        case TokenType::GE: return "GE";
        case TokenType::ANDAND: return "ANDAND";
        case TokenType::OROR: return "OROR";
        case TokenType::NOT: return "NOT";
        case TokenType::STRING_LITERAL: return "STRING_LITERAL";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::IDENT: return "IDENT";
        case TokenType::BOOLEAN_LITERAL: return "BOOLEAN";
        case TokenType::NULL_LITERAL: return "NULL";
        case TokenType::COMMENT: return "COMMENT";
        case TokenType::END_OF_FILE: return "EOF";
        default: return "UNKNOWN";
    }
}

struct Lexer {
    string src;
    size_t i = 0;
    int line = 1, col = 1;
    vector<Token> tokens;
    Lexer(const string &s): src(s) {}

    char peek() { return (i < src.size() ? src[i] : '\0'); }
    char peekNext() { return (i+1 < src.size() ? src[i+1] : '\0'); }
    char get() {
        if (i >= src.size()) return '\0';
        char c = src[i++];
        if (c == '\n') { line++; col = 1; } else col++;
        return c;
    }
    static bool isIdentStart(char c) {
        return (isalpha((unsigned char)c) || c == '_' || (unsigned char)c >= 128);
    }
    static bool isIdentChar(char c) {
        return isalnum((unsigned char)c) || c == '_' || (unsigned char)c >= 128;
    }

    void addToken(TokenType t, const string &lex) {
        int startCol = max(1, col - (int)lex.size());
        tokens.push_back({t, lex, line, startCol});
    }

    void skipWhitespace() {
        while (true) {
            char c = peek();
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') get();
            else break;
        }
    }

    void tokenize() {
        while (true) {
            skipWhitespace();
            char c = peek();
            if (c == '\0') { addToken(TokenType::END_OF_FILE, ""); break; }

            // comments
            if (c == '/') {
                if (peekNext() == '/') {
                    get(); get();
                    string com;
                    while (peek() != '\0' && peek() != '\n') com.push_back(get());
                    addToken(TokenType::COMMENT, com);
                    continue;
                } else if (peekNext() == '*') {
                    get(); get();
                    string com;
                    while (!(peek() == '*' && peekNext() == '/') && peek() != '\0') com.push_back(get());
                    if (peek() == '*' && peekNext() == '/') { get(); get(); }
                    addToken(TokenType::COMMENT, com);
                    continue;
                }
            }

            // string literal
            if (c == '"') {
                get();
                string val;
                while (peek() != '\0' && peek() != '"') {
                    char ch = get();
                    if (ch == '\\' && peek()!='\0') {
                        char esc = get();
                        if (esc == 'n') val.push_back('\n');
                        else if (esc == 't') val.push_back('\t');
                        else val.push_back(esc);
                    } else val.push_back(ch);
                }
                if (peek() == '"') get();
                addToken(TokenType::STRING_LITERAL, val);
                continue;
            }

            // number
            if (isdigit((unsigned char)c)) {
                string num;
                while (isdigit((unsigned char)peek())) num.push_back(get());
                addToken(TokenType::NUMBER, num);
                continue;
            }

            // identifiers and keywords
            if (isIdentStart(c)) {
                string id;
                while (isIdentChar(peek())) id.push_back(get());
                string lower = id; for (auto &ch : lower) ch = tolower((unsigned char)ch);
                if (lower == "class") addToken(TokenType::CLASS, id);
                else if (lower == "extends") addToken(TokenType::EXTENDS, id);
                else if (lower == "int") addToken(TokenType::INT, id);
                else if (lower == "string") addToken(TokenType::STRING, id);
                else if (lower == "bool") addToken(TokenType::BOOL, id);
                else if (lower == "method") addToken(TokenType::METHOD, id);
                else if (lower == "methodmain") addToken(TokenType::METHODMAIN, id);
                else if (lower == "print") addToken(TokenType::PRINT, id);
                else if (lower == "if") addToken(TokenType::IF, id);
                else if (lower == "else") addToken(TokenType::ELSE, id);
                else if (lower == "while") addToken(TokenType::WHILE, id);
                else if (lower == "for") addToken(TokenType::FOR, id);
                else if (lower == "return") addToken(TokenType::RETURN, id);
                else if (lower == "true" || lower == "false") addToken(TokenType::BOOLEAN_LITERAL, lower);
                else if (lower == "null") addToken(TokenType::NULL_LITERAL, lower);
                else addToken(TokenType::IDENT, id);
                continue;
            }

            // multi-char and single char ops
            if (c == '=') {
                if (peekNext() == '=') { get(); get(); addToken(TokenType::EQEQ, "=="); }
                else { get(); addToken(TokenType::EQUAL, "="); }
                continue;
            }
            if (c == '!') {
                if (peekNext() == '=') { get(); get(); addToken(TokenType::NOTEQ, "!="); }
                else { get(); addToken(TokenType::NOT, "!"); }
                continue;
            }
            if (c == '<') {
                if (peekNext() == '=') { get(); get(); addToken(TokenType::LE, "<="); }
                else { get(); addToken(TokenType::LT, "<"); }
                continue;
            }
            if (c == '>') {
                if (peekNext() == '=') { get(); get(); addToken(TokenType::GE, ">="); }
                else { get(); addToken(TokenType::GT, ">"); }
                continue;
            }
            if (c == '&' && peekNext() == '&') { get(); get(); addToken(TokenType::ANDAND, "&&"); continue; }
            if (c == '|' && peekNext() == '|') { get(); get(); addToken(TokenType::OROR, "||"); continue; }

            switch (c) {
                case '{': get(); addToken(TokenType::LBRACE, "{"); break;
                case '}': get(); addToken(TokenType::RBRACE, "}"); break;
                case '[': get(); addToken(TokenType::LBRACKET, "["); break;
                case ']': get(); addToken(TokenType::RBRACKET, "]"); break;
                case '(': get(); addToken(TokenType::LPAREN, "("); break;
                case ')': get(); addToken(TokenType::RPAREN, ")"); break;
                case ';': get(); addToken(TokenType::SEMICOLON, ";"); break;
                case ',': get(); addToken(TokenType::COMMA, ","); break;
                case '+': get(); addToken(TokenType::PLUS, "+"); break;
                case '-': get(); addToken(TokenType::MINUS, "-"); break;
                case '*': get(); addToken(TokenType::STAR, "*"); break;
                // ✅ FIX: quitar get() extra en el token SLASH
                case '/': get(); addToken(TokenType::SLASH, "/"); break;
                default: {
                    string s; s.push_back(get());
                    addToken(TokenType::UNKNOWN, s);
                    break;
                }
            }
        }
    }
};

/* -------------------- AST Node -------------------- */
struct AST {
    string nodeType;
    unordered_map<string,string> kv;
    vector<AST*> children;
    int line = 0, col = 0;
    AST(const string &t=""): nodeType(t) {}
};

/* -------------------- Parser (recursive descent) -------------------- */

struct Parser {
    vector<Token> &tokens;
    size_t idx = 0;
    Parser(vector<Token> &toks): tokens(toks) {}

    Token cur() { return (idx < tokens.size() ? tokens[idx] : Token{TokenType::END_OF_FILE,"",0,0}); }
    bool match(TokenType tt) {
        if (cur().type == tt) { idx++; return true; }
        return false;
    }
    bool expect(TokenType tt, const string &errMsg) {
        if (match(tt)) return true;
        Token c = cur();
        cerr << "Error sintaxis (esperado " << tokenTypeName(tt) << ") en linea " << c.line << " col " << c.col << ". " << errMsg << "\n";
        throw runtime_error("Parse error");
    }

    // program = { class } , methodMain ;
    AST* parseProgram(){
        AST* root = new AST("Program");
        while (cur().type == TokenType::CLASS) {
            root->children.push_back(parseClass());
        }
        if (cur().type == TokenType::METHODMAIN) {
            root->children.push_back(parseMethodMain());
        } else {
            Token c = cur();
            cerr << "Error: falta methodMain al final. Linea " << c.line << " col " << c.col << "\n";
            throw runtime_error("Parse error");
        }
        return root;
    }
     // class = "Class", identificador, [ "extends", identificador ], "{", { miembro }, "}" ;
    AST* parseClass(){
        Token tclass = cur(); expect(TokenType::CLASS, "Se esperaba 'Class'");
        Token name = cur(); expect(TokenType::IDENT, "Nombre de clase esperado");
        AST* node = new AST("Class"); node->kv["name"] = name.lexeme; node->line = name.line; node->col = name.col;
        if (cur().type == TokenType::EXTENDS) {
            match(TokenType::EXTENDS);
            Token p = cur(); expect(TokenType::IDENT, "Identificador despues de extends esperado");
            node->kv["extends"] = p.lexeme;
        }
        expect(TokenType::LBRACE, "Se esperaba '{' para iniciar clase");
        while (cur().type != TokenType::RBRACE && cur().type != TokenType::END_OF_FILE) {
            if (cur().type == TokenType::INT || cur().type == TokenType::STRING || cur().type == TokenType::BOOL) {
                node->children.push_back(parseAttribute());
                continue;
            } else if (cur().type == TokenType::METHOD) {
                node->children.push_back(parseMethod());
                continue;
            } else if (cur().type == TokenType::COMMENT) {
                idx++;
                continue;
            } else {
                
                Token c = cur();
                cerr << "Error: miembro inesperado en clase '"<< name.lexeme <<"' linea " << c.line << " col " << c.col << "\n";
                throw runtime_error("Parse error");
            }
        }
        expect(TokenType::RBRACE, "Se esperaba '}' para cerrar clase");
        return node;
    }
    /*AST* parseAttribute(){
        Token tipoTok = cur(); idx++;
        string tipoLex = tokenTypeName(tipoTok.type); 
        Token name = cur(); expect(TokenType::IDENT, "Nombre de atributo esperado");
        AST* a = new AST("Attribute");
        a->kv["name"] = name.lexeme;
        a->kv["type"] = (tipoLex=="STRING" ? "string" : (tipoLex=="INT" ? "int" : (tipoLex=="BOOL"? "bool": tipoLex)));
        a->line = name.line; a->col = name.col;
        if (match(TokenType::EQUAL)) {
            AST* expr = parseExpressionNode();
            
            AST* init = new AST("Init");
            init->kv["expr_type"] = expr->nodeType;
            init->children.push_back(expr);
            a->children.push_back(init);
        }
        expect(TokenType::SEMICOLON, "Falta ';' despues del atributo");
        return a;
    }*/
    AST* parseMethod(){
        Token mt = cur(); expect(TokenType::METHOD, "Se esperaba 'method'");
        Token name = cur(); expect(TokenType::IDENT, "Nombre de metodo esperado");
        AST* m = new AST("Method"); m->kv["name"] = name.lexeme; m->line = name.line; m->col = name.col;
        expect(TokenType::LBRACE, "Se esperaba '{' en metodo");
        while (cur().type != TokenType::RBRACE && cur().type != TokenType::END_OF_FILE) {
            AST* instr = parseInstruction();
            if (instr) m->children.push_back(instr);
            //else { idx++; }
        }
        expect(TokenType::RBRACE, "Se esperaba '}' para cerrar metodo");
        return m;
    }
    AST* parseMethodMain(){
        Token mt = cur(); expect(TokenType::METHODMAIN, "Se esperaba 'methodMain'");
        AST* m = new AST("MethodMain"); m->line = mt.line; m->col = mt.col;
        expect(TokenType::LBRACE, "Se esperaba '{' en methodMain");
        while (cur().type != TokenType::RBRACE && cur().type != TokenType::END_OF_FILE) {
            AST* instr = parseInstruction();
            if (instr) m->children.push_back(instr);
            //else idx++;
        }
        expect(TokenType::RBRACE, "Se esperaba '}' para cerrar methodMain");
        return m;
    }
    
    AST* parseInstruction() {
    TokenType tt = cur().type;

    // Instrucciones válidas
    if (tt == TokenType::PRINT) 
        return parsePrint();
    else if (tt == TokenType::IDENT) 
        return parseAssignment();

    // Comentarios dentro de métodos
    else if (tt == TokenType::COMMENT) {
        idx++; 
        return nullptr;
    }

    // Si llega un METHOD dentro de un método o clase, no debe lanzar error aquí
    else if (tt == TokenType::METHOD || tt == TokenType::CLASS) {
        // no es una instrucción, devolver nullptr para que el nivel superior lo procese
        return nullptr;
    }

    // Cierre de bloque o EOF: también se ignoran aquí
    else if (tt == TokenType::RBRACE || tt == TokenType::END_OF_FILE) {
        return nullptr;
    }

    // Cualquier otro token realmente inválido
    else {
        Token t = cur();
        cerr << "Error: instruccion inesperada token " 
             << tokenTypeName(t.type) 
             << " linea " << t.line << "\n";
        throw runtime_error("Parse error");
    }
}

    /*AST* parseInstruction(){
        Token c = cur();
        if (c.type == TokenType::COMMENT) { idx++; return nullptr; }
        if (c.type == TokenType::IDENT) {
            
            Token next = tokens.size() > idx+1 ? tokens[idx+1] : Token{TokenType::END_OF_FILE,"",0,0};
            if (next.type == TokenType::EQUAL) {
                return parseAssignment();
            } else if (next.type == TokenType::LPAREN) {
                return parseCall();
            } else {
                cerr << "Error: instruccion desconocida empezando en identificador linea " << c.line << "\n";
                throw runtime_error("Parse error");
            }
        } else if (c.type == TokenType::PRINT) {
            return parsePrint();
        } else if (c.type == TokenType::IF) {
            return parseIf();
        } else if (c.type == TokenType::WHILE) {
            return parseWhile();
        } else if (c.type == TokenType::FOR) {
            return parseFor();
        } else if (c.type == TokenType::RETURN) {
            return parseReturn();
        } else {
            cerr << "Error: instruccion inesperada token " << tokenTypeName(c.type) << " linea " << c.line << "\n";
            throw runtime_error("Parse error");
        }
    }*/
    AST* parseExpressionNode(){
        return parseOr();
    }
    AST* parseOr() {
        AST* left = parseAnd();
        while (cur().type == TokenType::OROR) {
            Token op = cur(); match(TokenType::OROR);
            AST* right = parseAnd();
            AST* n = new AST("BinaryOp"); n->kv["op"] = "||"; n->children.push_back(left); n->children.push_back(right);
            left = n;
        }
        return left;
    }
    AST* parseAnd() {
        AST* left = parseEquality();
        while (cur().type == TokenType::ANDAND) {
            match(TokenType::ANDAND);
            AST* right = parseEquality();
            AST* n = new AST("BinaryOp"); n->kv["op"] = "&&"; n->children.push_back(left); n->children.push_back(right);
            left = n;
        }
        return left;
    }
    AST* parseEquality() {
        AST* left = parseComparison();
        while (cur().type == TokenType::EQEQ || cur().type == TokenType::NOTEQ) {
            Token op = cur(); idx++;
            AST* right = parseComparison();
            AST* n = new AST("BinaryOp"); n->kv["op"] = (op.type==TokenType::EQEQ?"==":"!="); n->children.push_back(left); n->children.push_back(right);
            left = n;
        }
        return left;
    }
    AST* parseComparison() {
        AST* left = parseAdd();
        while (cur().type == TokenType::LT || cur().type == TokenType::GT || cur().type==TokenType::LE || cur().type==TokenType::GE) {
            Token op = cur(); idx++;
            AST* right = parseAdd();
            string ops = (op.type==TokenType::LT?"<": (op.type==TokenType::GT?">": (op.type==TokenType::LE?"<=":">=")));
            AST* n = new AST("BinaryOp"); n->kv["op"] = ops; n->children.push_back(left); n->children.push_back(right);
            left = n;
        }
        return left;
    }
    AST* parseAdd() {
        AST* left = parseMul();
        while (cur().type == TokenType::PLUS || cur().type == TokenType::MINUS) {
            Token op = cur(); idx++;
            AST* right = parseMul();
            AST* n = new AST("BinaryOp"); n->kv["op"] = (op.type==TokenType::PLUS?"+":"-"); n->children.push_back(left); n->children.push_back(right);
            left = n;
        }
        return left;
    }
    AST* parseMul() {
        AST* left = parseUnary();
        while (cur().type == TokenType::STAR) {
            match(TokenType::STAR);
            AST* right = parseUnary();
            AST* n = new AST("BinaryOp"); n->kv["op"] = "*"; n->children.push_back(left); n->children.push_back(right);
            left = n;
        }
        return left;
    }
    AST* parseUnary() {
        if (cur().type == TokenType::NOT) { match(TokenType::NOT); AST* child = parseUnary(); AST* n = new AST("UnaryOp"); n->kv["op"]="!"; n->children.push_back(child); return n; }
        if (cur().type == TokenType::MINUS) { match(TokenType::MINUS); AST* child = parseUnary(); AST* n = new AST("UnaryOp"); n->kv["op"]="neg"; n->children.push_back(child); return n; }
        return parsePrimary();
    }
    AST* parsePrimary(){
        Token c = cur();
        if (c.type == TokenType::NUMBER) { idx++; AST* n = new AST("Number"); n->kv["value"] = c.lexeme; return n; }
        if (c.type == TokenType::STRING_LITERAL) { idx++; AST* n = new AST("String"); n->kv["value"] = c.lexeme; return n; }
        if (c.type == TokenType::BOOLEAN_LITERAL) { idx++; AST* n = new AST("Boolean"); n->kv["value"] = c.lexeme; return n; }
        if (c.type == TokenType::NULL_LITERAL) { idx++; AST* n = new AST("Null"); return n; }
        if (c.type == TokenType::IDENT) { idx++; AST* n = new AST("Ident"); n->kv["name"] = c.lexeme; return n; }
        if (c.type == TokenType::LBRACKET) {
            
            match(TokenType::LBRACKET);
            AST* lst = new AST("List");
            if (cur().type == TokenType::STRING_LITERAL) {
                AST* s = new AST("String"); s->kv["value"] = cur().lexeme; lst->children.push_back(s); match(TokenType::STRING_LITERAL);
                while (match(TokenType::COMMA)) {
                    if (cur().type == TokenType::STRING_LITERAL) { AST* s2 = new AST("String"); s2->kv["value"]=cur().lexeme; lst->children.push_back(s2); match(TokenType::STRING_LITERAL); }
                }
            }
            expect(TokenType::RBRACKET, "falta ']' en lista");
            return lst;
        }
        if (c.type == TokenType::LPAREN) {
            match(TokenType::LPAREN);
            AST* inner = parseExpressionNode();
            expect(TokenType::RPAREN, "falta ')' en expresion");
            return inner;
        }
        cerr << "Error: termino inesperado en expresion token " << tokenTypeName(c.type) << " linea " << c.line << "\n";
        throw runtime_error("Parse error");
    }
    AST* parseBlock(){
        expect(TokenType::LBRACE, "esperado '{' en bloque");
        AST* blk = new AST("Block");
        while (cur().type != TokenType::RBRACE && cur().type != TokenType::END_OF_FILE) {
            AST* instr = parseInstruction();
            if (instr) blk->children.push_back(instr);
        }
        expect(TokenType::RBRACE, "esperado '}' al final del bloque");
        return blk;
    }
    AST* parseAssignment(){
        Token id = cur(); expect(TokenType::IDENT, "identificador esperado en asignacion");
        expect(TokenType::EQUAL, "esperado '=' en asignacion");
        AST* expr = parseExpressionNode();
        expect(TokenType::SEMICOLON, "falta ';' en asignacion");
        AST* node = new AST("Assignment");
        node->kv["target"] = id.lexeme;
        node->children.push_back(expr);
        node->line = id.line; node->col = id.col;
        return node;
    }
    AST* parsePrint(){
        Token p = cur(); expect(TokenType::PRINT, "print esperado");
        AST* node = new AST("Print"); node->line = p.line; node->col = p.col;
        expect(TokenType::LPAREN, "esperado '(' despues de print");
        AST* ex = parseExpressionNode();
        node->children.push_back(ex);
        expect(TokenType::RPAREN, "esperado ')' despues de print expr");
        expect(TokenType::SEMICOLON, "falta ';' despues de print()");
        return node;
    }
    AST* parseIf(){
        Token tk = cur(); expect(TokenType::IF, "if esperado");
        expect(TokenType::LPAREN, "esperado '(' despues de if");
        AST* cond = parseExpressionNode();
        expect(TokenType::RPAREN, "esperado ')' luego de condicion");
        AST* node = new AST("If"); node->children.push_back(cond);
        AST* thenBlock = parseBlock();
        node->children.push_back(thenBlock);
        if (match(TokenType::ELSE)) {
            AST* elseBlock = parseBlock();
            node->children.push_back(elseBlock);
        }
        return node;
    }
    AST* parseWhile(){
        Token tk = cur(); expect(TokenType::WHILE, "while esperado");
        expect(TokenType::LPAREN, "esperado '(' despues de while");
        AST* cond = parseExpressionNode();
        expect(TokenType::RPAREN, "esperado ')'");
        AST* node = new AST("While");
        node->children.push_back(cond);
        node->children.push_back(parseBlock());
        return node;
    }
    AST* parseFor(){
        Token tk = cur(); expect(TokenType::FOR, "for esperado");
        expect(TokenType::LPAREN, "esperado '(' en for");
        AST* node = new AST("For");
       
        if (cur().type == TokenType::IDENT) {
            node->children.push_back(parseAssignment()); 
        } else {
            expect(TokenType::SEMICOLON, "esperado ';' en for (init)");
        }
        
        if (cur().type != TokenType::SEMICOLON) {
            node->children.push_back(parseExpressionNode());
        }
        expect(TokenType::SEMICOLON, "esperado ';' en for (cond)");
        
        if (cur().type != TokenType::RPAREN) {
            if (cur().type == TokenType::IDENT) node->children.push_back(parseAssignment());
            else { /* simple skip */ }
        }
        expect(TokenType::RPAREN, "esperado ')' en for");
        node->children.push_back(parseBlock());
        return node;
    }
    AST* parseReturn(){
        Token tk = cur(); expect(TokenType::RETURN, "return esperado");
        AST* node = new AST("Return");
        if (cur().type != TokenType::SEMICOLON) {
            node->children.push_back(parseExpressionNode());
        }
        expect(TokenType::SEMICOLON, "falta ';' en return");
        return node;
    }
    AST* parseCall(){
        Token id = cur(); expect(TokenType::IDENT, "identificador esperado en llamada");
        AST* node = new AST("Call"); node->kv["name"] = id.lexeme; node->line = id.line; node->col = id.col;
        expect(TokenType::LPAREN, "esperado '(' en llamada");
        if (cur().type != TokenType::RPAREN) {
            vector<AST*> args;
            args.push_back(parseExpressionNode());
            while (match(TokenType::COMMA)) args.push_back(parseExpressionNode());
            for (auto a: args) node->children.push_back(a);
        }
        expect(TokenType::RPAREN, "esperado ')' en llamada");
        expect(TokenType::SEMICOLON, "falta ';' despues de llamada");
        return node;
    }
    


    // FIX #2: nueva lógica dentro de parseAttribute()
    AST* parseAttribute() {
        Token tipoTok = cur(); idx++;
        string tipoLex = tokenTypeName(tipoTok.type);
        // caso especial: string [] id = lista ;
        if (tipoTok.type == TokenType::STRING && cur().type == TokenType::LBRACKET) {
            match(TokenType::LBRACKET);
            expect(TokenType::RBRACKET, "Se esperaba ']' después de '[' en atributo string[]");
            Token name = cur(); expect(TokenType::IDENT, "Nombre de atributo esperado después de string[]");
            AST* a = new AST("Attribute");
            a->kv["name"] = name.lexeme;
            a->kv["type"] = "string[]";
            a->line = name.line; a->col = name.col;
            expect(TokenType::EQUAL, "Se esperaba '=' en atributo string[]");
            AST* listNode = parsePrimary();
            AST* init = new AST("Init");
            init->children.push_back(listNode);
            a->children.push_back(init);
            expect(TokenType::SEMICOLON, "Se esperaba ';' al final del atributo string[]");
            return a;
        }

        // caso general: tipo id [= expr] ;
        Token name = cur(); expect(TokenType::IDENT, "Nombre de atributo esperado");
        AST* a = new AST("Attribute");
        a->kv["name"] = name.lexeme;
        a->kv["type"] = (tipoLex=="STRING" ? "string" : (tipoLex=="INT" ? "int" : (tipoLex=="BOOL"? "bool": tipoLex)));
        a->line = name.line; a->col = name.col;
        if (match(TokenType::EQUAL)) {
            AST* expr = parseExpressionNode();
            AST* init = new AST("Init");
            init->children.push_back(expr);
            a->children.push_back(init);
        }
        expect(TokenType::SEMICOLON, "Falta ';' después del atributo");
        return a;
    }


void writeJSON(AST* node, ostream &out, int indent=0) {
    string ind(indent,' ');
    out << ind << "{\n";
    out << ind << "  \"node\": \"" << node->nodeType << "\"";
    if (!node->kv.empty()) {
        out << ",\n";
        // properties
        out << ind << "  \"props\": {\n";
        bool first = true;
        for (auto &kv : node->kv) {
            if (!first) out << ",\n";
            out << ind << "    \"" << kv.first << "\": \"" << kv.second << "\"";
            first = false;
        }
        out << "\n" << ind << "  }";
    }
    if (!node->children.empty()) {
        out << ",\n" << ind << "  \"children\": [\n";
        for (size_t i=0;i<node->children.size();++i) {
            writeJSON(node->children[i], out, indent+4);
            if (i+1<node->children.size()) out << ",\n";
            else out << "\n";
        }
        out << ind << "  ]\n";
        out << ind << "}";
    } else {
        out << "\n" << ind << "}";
    }
}

int main(int argc, char** argv) {
    string filename = "mini-lenguaje.brik";
    if (argc >= 2) filename = argv[1];
    ifstream in(filename, ios::binary);
    if (!in) {
        cerr << "No se pudo abrir el archivo: " << filename << endl;
        return 1;
    }
    stringstream buffer; buffer << in.rdbuf();
    string source = buffer.str();

    
    Lexer lx(source);
    lx.tokenize();

    
    ofstream tokout("tokens.txt");
    for (auto &t : lx.tokens) {
        tokout << t.line << ":" << t.col << " " << tokenTypeName(t.type) << " -> " << t.lexeme << "\n";
    }
    tokout.close();

    try {
        Parser p(lx.tokens);
        AST* ast = p.parseProgram();
        ofstream fout("arbol.ast");
        writeJSON(ast, fout, 0);
        fout << "\n";
        fout.close();
        cout << "AST generado: arbol.ast\n";
    } catch (const exception &e) {
        cerr << "Parsing fallido: " << e.what() << "\n";
        return 1;
    }
    return 0;
}