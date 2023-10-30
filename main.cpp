#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <sstream>
#include <regex>

using namespace std;

// Define the token type constants
const string TT_KEYWORD = "keyword";
const string TT_IDENTIFIER = "identifier";
const string TT_SEPARATOR = "separator";
const string TT_OPERATOR = "operator";
const string TT_LITERAL = "literal";
const string TT_ERROR = "error: unrecognized symbol";

typedef vector<pair<string, string>> TokenList;

bool isHexadecimal(const string& token) {
    regex hex_regex("0[xX][0-9a-fA-F]+");
    return regex_match(token, hex_regex);
}

bool isFloatingPoint(const string& token) {
    regex float_regex("[-+]?[0-9]*\\.[0-9]+([eE][-+]?[0-9]+)?");
    return regex_match(token, float_regex);
}

bool isDecimal(const string& token) {
    regex decimal_regex("[-+]?\\d+");
    return regex_match(token, decimal_regex);
}

bool isIdentifier(const string& token) {
    if (token.empty()) {
        return false;
    }

    if (!((token[0] >= 'A' && token[0] <= 'Z') || (token[0] >= 'a' && token[0] <= 'z') || token[0] == '_')) {
        return false;
    }

    for (size_t i = 1; i < token.size(); ++i) {
        if (!((token[i] >= 'A' && token[i] <= 'Z') || (token[i] >= 'a' && token[i] <= 'z') || (token[i] >= '0' && token[i] <= '9') || token[i] == '_')) {
            return false;
        }
    }

    return true;
}

bool isStringLiteral(const string& token) {
    size_t open_quote = token.find('\"');
    if (open_quote == string::npos) {
        return false;
    }

    size_t close_quote = token.rfind('\"');
    if (close_quote == string::npos || close_quote == 0 || close_quote <= open_quote) {
        return false;
    }

    bool even_slashes_before = true;
    for (size_t i = close_quote; i >= 1; --i) {
        if (token[i - 1] != '\\') {
            break;
        }
        even_slashes_before = !even_slashes_before;
    }

    if (!even_slashes_before) {
        return false;
    }

    return true;
}

bool isCharacterLiteral(const string& token) {
    regex char_literal_regex(R"('(?:\\.|[^\\'])')");
    return regex_match(token, char_literal_regex);
}

bool isSeparator(char c) {
    return c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' || c == ';' || c == ',' || c == '.';
}

bool isKeyword(const string& token) {
    vector<string> keywords = {"abstract", "assert", "boolean", "break", "byte", "case",
                               "catch", "char", "class", "const", "continue", "default",
                               "do", "double", "else", "enum", "extends", "final",
                               "finally", "float", "for", "if", "goto", "implements",
                               "import", "instanceof", "int", "interface", "long",
                               "native", "new", "package", "private", "protected",
                               "public", "return", "short", "static", "strictfp",
                               "super", "switch", "synchronized", "this", "throw",
                               "throws", "transient", "try", "void", "volatile", "while"};

    return find(keywords.begin(), keywords.end(), token) != keywords.end();
}

bool isOperator(const string& token) {
    vector<string> operators = {
            "==", "!==", "<=", ">=", "&&", "||", "!=", "!",
            "+", "-", "*", "/", "%", "&", "|", "^", "~",
            "<<", ">>", "="
    };

    return find(operators.begin(), operators.end(), token) != operators.end();
}

vector<string> splitDots(const string& token) {
    vector<string> tokens;
    size_t start = 0, end = 0;

    while ((end = token.find('.', start)) != string::npos) {
        tokens.push_back(token.substr(start, end - start));
        tokens.push_back(".");
        start = end + 1;
    }

    tokens.push_back(token.substr(start));

    return tokens;
}

TokenList analyzeToken(const string& curr_token) {
    TokenList result;
    vector<string> parts = splitDots(curr_token);

    for (const string& token : parts) {
        if (isSeparator(token[0])) {
            result.push_back(make_pair(token, TT_SEPARATOR));
        } else if (isOperator(token)) {
            result.push_back(make_pair(token, TT_OPERATOR));
        } else if (isHexadecimal(token) || isFloatingPoint(token) ||
                   isDecimal(token) || isStringLiteral(token) ||
                   isCharacterLiteral(token)) {
            result.push_back(make_pair(token, TT_LITERAL));
        } else if (isKeyword(token)) {
            result.push_back(make_pair(token, TT_KEYWORD));
        } else if (isIdentifier(token)) {
            result.push_back(make_pair(token, TT_IDENTIFIER));
        } else {
            result.push_back(make_pair(token, TT_ERROR));
        }
    }
    return result;
}

TokenList analyzeCode(const string& code) {
    TokenList result;
    istringstream stringBuffer(code);

    bool in_multiline_comment = false;
    bool in_string_literal = false;

    // Main analyzer loop
    while (!stringBuffer.eof()) {
        char sym;
        string curr_token;
        stringBuffer.get(sym);

        // Skip whitespaces
        while (isspace(sym) && !stringBuffer.eof()) {
            stringBuffer.get(sym);
        }

        // Check if reached the end of the file
        if (stringBuffer.eof()) {
            break;
        }

        // Process specific symbols and collect current token
        if (sym == '"' || in_string_literal) {
            in_string_literal = !in_string_literal;
            curr_token = "\"";
            while (in_string_literal && !stringBuffer.eof()) {
                curr_token += sym;
                stringBuffer.get(sym);
                if (sym == '\\') {
                    curr_token += sym;
                    stringBuffer.get(sym);
                }
                if (sym == '"') {
                    in_string_literal = false;
                }
            }
            curr_token += sym;
        } else if (isalnum(sym) || sym == '_' || sym == '.') {
            while ((isalnum(sym) || sym == '_' || sym == '.') && !stringBuffer.eof()) {
                curr_token += sym;
                stringBuffer.get(sym);
            }
        } else if (ispunct(sym)) {
            if (sym == '/' && stringBuffer.peek() == '/') {
                stringBuffer.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            } else if (sym == '/' && stringBuffer.peek() == '*') {
                in_multiline_comment = true;
                stringBuffer.ignore(1);
                stringBuffer.get(sym);
            } else if (sym == '*' && stringBuffer.peek() == '/') {
                in_multiline_comment = false;
                stringBuffer.ignore(1);
                stringBuffer.get(sym);
                continue;
            } else {
                curr_token += sym;
                stringBuffer.get(sym);
            }
        }

        if (!in_multiline_comment) {
            TokenList part_result = analyzeToken(curr_token);
            result.insert(result.end(), part_result.begin(), part_result.end());
        }
    }

    return result;
}

int main() {
    string code = "import java.util.Scanner;\n"
                  "public class HelloWorld {\n"
                  "    public static void main(String[] args) {\n"
                  "        // This is a single-line comment\n"
                  "        Scanner scanner = new Scanner(System.in);\n"
                  "        System.out.println(\"Enter a number:\");\n"
                  "        int number = scanner.nextInt();\n"
                  "        /* This is a\n"
                  "           multiline comment */\n"
                  "        System.out.println(number);\n"
                  "        scanner.close();\n"
                  "    }\n"
                  "}\n";

    TokenList token_list = analyzeCode(code);

    for (const auto& token : token_list) {
        cout << "<" << token.first << ", " << token.second << ">" << endl;
    }

    return 0;
}