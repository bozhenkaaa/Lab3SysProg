
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <sstream>
#include <regex>

using namespace std;

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
    regex identifier_regex(R"(([_a-zA-Z][_a-zA-Z0-9]*\.?)+)");
    return regex_match(token, identifier_regex);
}

bool isStringLiteral(const string& token) {
    regex string_literal_regex(R"("(\\"|[^"])*")");
    return regex_match(token, string_literal_regex);
}

bool isCharacterLiteral(const string& token) {
    regex char_literal_regex(R"('(?:\\.|[^\\'])')");
    return regex_match(token, char_literal_regex);
}

TokenList analyzeCode(const string& code) {
    vector<string> keywords = {"abstract", "assert", "boolean", "break", "byte", "case",
                               "catch", "char", "class", "const", "continue", "default",
                               "do", "double", "else", "enum", "extends", "final",
                               "finally", "float", "for", "if", "goto", "implements",
                               "import", "instanceof", "int", "interface", "long",
                               "native", "new", "package", "private", "protected",
                               "public", "return", "short", "static", "strictfp",
                               "super", "switch", "synchronized", "this", "throw",
                               "throws", "transient", "try", "void", "volatile", "while"};

    string operators = "<>(){}[]*-+=/%^&|!;?:,\\";

    TokenList result;
    stringstream stringBuffer(code);
    string line;
    bool in_multiline_comment = false;
    bool in_string_literal = false;

    while (getline(stringBuffer, line)){
        string token;
        stringstream lineStream(line);
        bool in_singleline_comment = false;

        while (lineStream >> token){
            size_t comment_start = token.find("//");
            size_t multiline_comment_start = token.find("/*");
            size_t string_literal_start = token.find('\"');

            if (string_literal_start != string::npos) {
                in_string_literal = !in_string_literal;
                if (!in_singleline_comment && !in_multiline_comment) {
                    token.erase(string_literal_start, 1);
                } else {
                    // якщо ми перебуваємо в коментарі, не враховуємо знак екранування
                    token.erase(string_literal_start, 2);
                }
            }

            if (comment_start != string::npos && !in_string_literal) {
                token = token.substr(0, comment_start);
                in_singleline_comment = true;
            }

            if (!in_multiline_comment && multiline_comment_start != string::npos && multiline_comment_start < comment_start && !in_string_literal) {
                in_multiline_comment = true;
                result.push_back(make_pair("/*", "start of multiline comment"));
            }

            if (in_multiline_comment) {
                size_t multiline_comment_end = token.find("*/");
                if (multiline_comment_end != string::npos) {
                    result.push_back(make_pair("*/", "end of multiline comment"));
                    in_multiline_comment = false;
                    token.erase(0, multiline_comment_end + 2);
                } else {
                    token.clear();
                }
            }

            if (!token.empty() && !in_multiline_comment && !in_singleline_comment) {
                // Встановити межі для ідентифікаторів та числових літералів
                auto notOperator = [&](char c) { return find(operators.begin(), operators.end(), c) == operators.end(); };

                for (size_t i = 0; i < token.size();) {
                    string curr_token;

                    if (find(operators.begin(), operators.end(), token[i]) != operators.end()) {
                        curr_token.push_back(token[i++]);
                        result.push_back(make_pair(curr_token, "operator or separator"));
                    } else if (token[i] == '@') {
                        curr_token.push_back(token[i++]);
                        result.push_back(make_pair(curr_token, "annotation"));
                    } else {
                        curr_token.push_back(token[i++]);

                        // Цикл для ідентифікаторів та числових літералів
                        while (i < token.size() && notOperator(token[i])) {
                            if (token[i] == '.') {
                                if (i > 0 && isalnum(token[i - 1])) {
                                    // якщо це крапка, яка розділяє пакети
                                    curr_token.push_back(token[i++]);
                                } else {
                                    break; // це десяткова крапка, закінчуємо
                                }
                            } else {
                                curr_token.push_back(token[i++]);
                            }
                        }

                        if (isHexadecimal(curr_token)) {
                            result.push_back(make_pair(curr_token, "hexadecimal literal"));
                        } else if (isFloatingPoint(curr_token)) {
                            result.push_back(make_pair(curr_token, "floating-point literal"));
                        } else if (isDecimal(curr_token)) {
                            result.push_back(make_pair(curr_token, "decimal literal"));
                        } else if (isStringLiteral(curr_token)) {
                            result.push_back(make_pair(curr_token, "string literal"));
                        } else if (isCharacterLiteral(curr_token)) {
                            result.push_back(make_pair(curr_token, "character literal"));
                        } else if (find(keywords.begin(), keywords.end(), curr_token) != keywords.end()) {
                            result.push_back(make_pair(curr_token, "keyword"));
                        } else if (isIdentifier(curr_token)) {
                            result.push_back(make_pair(curr_token, "identifier"));
                        } else {
                            result.push_back(make_pair(curr_token, "error: unrecognized symbol"));
                        }
                    }
                }
            }

            if (in_singleline_comment) {
                token.clear();
            }
        }

        // Add a detected single-line comment at the end of the line
        if (in_singleline_comment) {
            result.push_back(make_pair("//", "comment"));
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

    // Вивести весь список лексем <лексема, тип_лексеми>
    for (const auto& token : token_list) {
        cout << "<" << token.first << ", " << token.second << ">" << endl;
    }

    return 0;
}