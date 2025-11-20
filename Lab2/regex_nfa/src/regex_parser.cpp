#include "regex_parser.h"           
#include <string>
#include <stack>
#include <stdexcept>


using namespace regexnfa;


std::string regexnfa::add_concat(const std::string& regex) {
std::string out;
char prev = 0;
for (char c : regex) {
if (prev) {
bool prev_is_operator = (prev == '|' || prev == '(');
bool cur_is_operator = (c == '|' || c == ')' || c == '*');
if (!prev_is_operator && !cur_is_operator) {
out.push_back('.');
}
}
out.push_back(c);
prev = c;
}
return out;
}


std::string regexnfa::to_postfix(const std::string& regex) {
// precedence
auto prec = [](char op)->int{
if (op == '*') return 3;
if (op == '.') return 2;
if (op == '|') return 1;
return 0;
};
std::string out;
std::stack<char> st;
for (char c : regex) {
if (c == '(') st.push(c);
else if (c == ')') {
while (!st.empty() && st.top()!= '(') { out.push_back(st.top()); st.pop(); }
if (st.empty()) throw std::runtime_error("Mismatched parentheses");
st.pop();
} else if (c == '|' || c == '.' || c == '*') {
if (c == '*') {
// unary, higher precedence
while (!st.empty() && prec(st.top()) > prec(c)) { out.push_back(st.top()); st.pop(); }
st.push(c);
} else {
while (!st.empty() && prec(st.top()) >= prec(c)) { out.push_back(st.top()); st.pop(); }
st.push(c);
}
} else {
// symbol
out.push_back(c);
}
}
while (!st.empty()) {
if (st.top() == '(' || st.top() == ')') throw std::runtime_error("Mismatched parentheses at end");
out.push_back(st.top()); st.pop();
}
return out;
}
