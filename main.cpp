#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

using namespace std;

typedef vector<pair<string, string>> TokenList;

TokenList analyzeCode(const string& code) {
    vector<string> keywords = {"int", "float", "const", "char", "if"};
    vector<char> operators = {'<', '>', '(', ')', '{', '}', '*', '=', ';', '!', ',', '-', '+', '*', '/', '%', '&', '|', '~', ';', ':', '[', ']', '\"'};

    TokenList result;

    string word = "";
    for (char ch : code) {
        if (isspace(ch)) {
            if (!word.empty()) {
                auto it = find(keywords.begin(), keywords.end(), word);
                if (it != keywords.end()) {
                    result.push_back(make_pair(word, "keyword"));
                } else {
                    result.push_back(make_pair(word, "identifier"));
                }
                word.clear();
            }
        } else if (find(operators.begin(), operators.end(), ch) != operators.end()) {
            if (!word.empty()) {
                auto it = find(keywords.begin(), keywords.end(), word);
                if (it != keywords.end()) {
                    result.push_back(make_pair(word, "keyword"));
                } else {
                    result.push_back(make_pair(word, "identifier"));
                }
                word.clear();
            }
            result.push_back(make_pair(string(1, ch), "operator and separator"));
        } else {
            word += ch;
        }
    }

    return result;
}

int main() {
    string code = "int main() {\n"
                  "  int x = 5;\n"
                  "  float y = 3.14;\n"
                  "  const char* str = \"Hello, World!\";\n"
                  "  if (x < 10) {\n"
                  "    cout << str << endl;\n"
                  "  }\n"
                  "}\n";

    TokenList token_list = analyzeCode(code);

    // Вивести весь список лексем <лексема, тип_лексеми>
    for (const auto& token : token_list) {
        cout << "<" << token.first << ", " << token.second << ">" << endl;
    }

    return 0;
}