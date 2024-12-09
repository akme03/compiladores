#include <iostream>
#include <string>
#include <sstream>
#include <stack>
#include <map>
#include <vector>
#include <cmath>
#include <cctype>

// Tipos de tokens
enum TokenType {
    NUMBER,
    VARIABLE,
    OPERATOR,
    FUNCTION,
    LEFT_PAREN,
    RIGHT_PAREN,
    ASSIGNMENT,
    PRINT,
    UNKNOWN
};

// Estructura para tokens
struct Token {
    TokenType type;
    std::string value;
};

// Funciones auxiliares para el análisis léxico
bool isNumber(const std::string& str) {
    bool decimalPoint = false;
    for (char c : str) {
        if (c == '.') {
            if (decimalPoint) return false;
            decimalPoint = true;
        } else if (!isdigit(c)) {
            return false;
        }
    }
    return !str.empty();
}

bool isVariable(const std::string& str) {
    if (str.empty() || !isalpha(str[0])) return false;
    for (char c : str) {
        if (!isalpha(c)) return false;
    }
    return true;
}

bool isOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
}

bool isFunction(const std::string& str) {
    return str == "root";
}

// Analizador léxico: divide la entrada en tokens
std::vector<Token> tokenize(const std::string& input) {
    std::vector<Token> tokens;
    std::istringstream stream(input);
    char ch;

    while (stream >> std::noskipws >> ch) {
        if (isdigit(ch) || ch == '.') {
            std::string num(1, ch);
            while (stream.peek() != EOF && (isdigit(stream.peek()) || stream.peek() == '.')) {
                stream >> ch;
                num += ch;
            }
            tokens.push_back({NUMBER, num});
        } else if (isalpha(ch)) {
            std::string var(1, ch);
            while (stream.peek() != EOF && isalpha(stream.peek())) {
                stream >> ch;
                var += ch;
            }
            if (var == "print") {
                tokens.push_back({PRINT, var});
            } else if (isFunction(var)) {
                tokens.push_back({FUNCTION, var});
            } else {
                tokens.push_back({VARIABLE, var});
            }
        } else if (isOperator(ch)) {
            tokens.push_back({OPERATOR, std::string(1, ch)});
        } else if (ch == '=') {
            tokens.push_back({ASSIGNMENT, std::string(1, ch)});
        } else if (ch == '(') {
            tokens.push_back({LEFT_PAREN, std::string(1, ch)});
        } else if (ch == ')') {
            tokens.push_back({RIGHT_PAREN, std::string(1, ch)});
        } else if (isspace(ch)) {
            continue;
        } else {
            tokens.push_back({UNKNOWN, std::string(1, ch)});
        }
    }

    return tokens;
}

// Prioridad de operadores
int precedence(const std::string& op) {
    if (op == "^") return 3;
    if (op == "*" || op == "/") return 2;
    if (op == "+" || op == "-") return 1;
    return 0;
}

// Conversión a notación posfija
std::vector<Token> toPostfix(const std::vector<Token>& tokens) {
    std::vector<Token> postfix;
    std::stack<Token> opStack;

    for (const auto& token : tokens) {
        if (token.type == NUMBER || token.type == VARIABLE) {
            postfix.push_back(token);
        } else if (token.type == FUNCTION) {
            opStack.push(token);
        } else if (token.type == OPERATOR) {
            while (!opStack.empty() && precedence(opStack.top().value) >= precedence(token.value)) {
                postfix.push_back(opStack.top());
                opStack.pop();
            }
            opStack.push(token);
        } else if (token.type == LEFT_PAREN) {
            opStack.push(token);
        } else if (token.type == RIGHT_PAREN) {
            while (!opStack.empty() && opStack.top().type != LEFT_PAREN) {
                postfix.push_back(opStack.top());
                opStack.pop();
            }
            if (!opStack.empty()) opStack.pop();
            if (!opStack.empty() && opStack.top().type == FUNCTION) {
                postfix.push_back(opStack.top());
                opStack.pop();
            }
        }
    }

    while (!opStack.empty()) {
        postfix.push_back(opStack.top());
        opStack.pop();
    }

    return postfix;
}

// Evaluador de expresiones en notación posfija
double evaluatePostfix(const std::vector<Token>& postfix, std::map<std::string, double>& variables) {
    std::stack<double> stack;

    for (const auto& token : postfix) {
        if (token.type == NUMBER) {
            stack.push(std::stod(token.value));
        } else if (token.type == VARIABLE) {
            if (variables.find(token.value) != variables.end()) {
                stack.push(variables[token.value]);
            } else {
                std::cerr << "Error: variable " << token.value << " no definida.\n";
                return 0;
            }
        } else if (token.type == OPERATOR) {
            double b = stack.top(); stack.pop();
            double a = stack.top(); stack.pop();
            if (token.value == "+") stack.push(a + b);
            else if (token.value == "-") stack.push(a - b);
            else if (token.value == "*") stack.push(a * b);
            else if (token.value == "/") stack.push(a / b);
            else if (token.value == "^") stack.push(std::pow(a, b));
        } else if (token.type == FUNCTION) {
            double a = stack.top(); stack.pop();
            if (token.value == "root") stack.push(std::sqrt(a));
        }
    }

    return stack.top();
}

// Interprete principal
void interpret(const std::string& input, std::map<std::string, double>& variables) {
    std::vector<Token> tokens = tokenize(input);

    if (tokens.empty()) return;

    std::cout << "Tokens: ";
    for (const auto& token : tokens) {
        std::cout << token.value << " ";
    }
    std::cout << std::endl;

    if (tokens[0].type == PRINT) {
        std::vector<Token> expr(tokens.begin() + 1, tokens.end());
        std::vector<Token> postfix = toPostfix(expr);
        double result = evaluatePostfix(postfix, variables);
        std::cout << "Imprimir: " << result << std::endl;
    } else if (tokens.size() > 2 && tokens[1].type == ASSIGNMENT) {
        std::string varName = tokens[0].value;
        std::vector<Token> expr(tokens.begin() + 2, tokens.end());
        std::vector<Token> postfix = toPostfix(expr);
        double result = evaluatePostfix(postfix, variables);
        variables[varName] = result;
        std::cout << "Asignacion: " << varName << " = " << result << std::endl;
    }
}

int main() {
    std::map<std::string, double> variables;
    std::string line;

    std::cout << "Ingrese sus operaciones separadas por comas:\n";
    while (std::getline(std::cin, line)) {
        if (line.empty()) break;

        std::istringstream stream(line);
        std::string expression;
        while (std::getline(stream, expression, ',')) {
            std::cout << "Procesando: " << expression << std::endl;
            interpret(expression, variables);
        }
    }

    return 0;
}