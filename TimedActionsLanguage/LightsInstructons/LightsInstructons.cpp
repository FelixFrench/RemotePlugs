#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cctype>
#include <cstring>
#include <cstdio>

namespace {

    // Recursive-descent parser
    struct Parser {
        const std::string& s;
        size_t pos = 0;
        int T, D, R, S;

        Parser(const std::string& str, int t, int d, int r, int s_)
            : s(str), T(t), D(d), R(r), S(s_) {
        }

        // Advance the pointer until something other than space is found or the end is reached
        void skip() {
            while (pos < s.size() && std::isspace(s[pos])) pos++;
        }

        // Check that the next character is as expected, returning false if it's not
        bool expect(char c) {
            skip();
            if (pos >= s.size() || s[pos] != c)
                return false;
            pos++;
            return true;
        }

        // Parse an integer expression. This could be a numeric literal, one of the known T/D/R/S values, or a min/max function
        bool parseInt(int& y) {
            skip();

            if (pos >= s.size())
                return false;

            // Parse a numeric literal
            if (std::isdigit(s[pos])) {
                int v = 0;
                while (pos < s.size() && std::isdigit(s[pos])) {
                    v = v * 10 + (s[pos++] - '0');
                }
                y = v;
                return true;
            }

            char c = s[pos++];

            // Parse a known variable
            if (c == 'T') { y = T; return true; }
            if (c == 'D') { y = D; return true; }
            if (c == 'R') { y = R; return true; }
            if (c == 'S') { y = S; return true; }

            // Parse a min/max function
            if (c == 'M' || c == 'm') {
                int a, b;
                if (!expect('(')) return false;
                if (!parseInt(a)) return false;
                if (!expect(',')) return false;
                if (!parseInt(b)) return false;
                if (!expect(')')) return false;

                y = (c == 'M') ? (a > b ? a : b) : (a < b ? a : b);
                return true;
            }

            // Anything else is a syntax error
            return false;
        }

        // Parse a boolean-returning function
        // This could be =, >, <, &, | or !
        // N.B.: There are no boolean literals
        bool parseBool(bool& y) {
            skip();
            if (pos >= s.size())
                return false;

            if (!expect('(')) return false;

            char op = s[pos++];

            // bool -> bool
            if (op == '!') {
                bool v;
                if (!parseBool(v)) return false;
                if (!expect(')')) return false;
                y = !v;
                return true;
            }

            // int, int -> bool
            if (op == '=' || op == '<' || op == '>') {
                int a, b;
                if (!parseInt(a)) return false;
                if (!expect(',')) return false;
                if (!parseInt(b)) return false;
                if (!expect(')')) return false;

                if (op == '=') {
                    if (a == 8) y = (D >= 1 && D <= 5);
                    if (a == 9) y = (D == 0 || D == 6);
                    y = (a == b);
                }
                else {
                    y = (op == '<') ? (a < b) : (a > b);
                }
                return true;
            }

            // bool, bool -> bool
            if (op == '&' || op == '|') {
                bool a, b;
                if (!parseBool(a)) return false;
                if (!expect(',')) return false;
                if (!parseBool(b)) return false;
                if (!expect(')')) return false;
                y = (op == '&') ? (a && b) : (a || b);
                return true;
            }

            return false;
        }
    };

} // namespace

bool solveProblem(
    const std::string& problem,
    int T, int D, int R, int S,
    const char* cmd
) {
        size_t comma = problem.rfind(',');
        bool success = false;
        bool result;
        std::string expr, command;

        if (comma != std::string::npos) {
            expr = problem.substr(0, comma);
            command = problem.substr(comma + 1);

            Parser p(expr, T, D, R, S);
            success = p.parseBool(result);
        }

        // Try to parse the statement and catch any errors
        if (!success) {

            // If an error occurs, return true to say that cmd is valid, but cmd is "error"
            sprintf_s((char*)cmd, 16, "error");
            ((char*)cmd)[15] = '\0';
            return true;
        }

        if (result) {

            // The parser succeded and a command is required, copy it into cmd
            sprintf_s((char*)cmd, 16, "%s", command.c_str());
            ((char*)cmd)[15] = '\0';
            return true;
        }

        // The parser succeeded but no command is required. Just return false.
        return false;
}


int main() {
    // Hard-coded system values
    int T = 726;   // minutes since midnight
    int D = 3;     // days since Sunday (3 = Wednesday)
    int R = 400;   // sunrise time
    int S = 1100;  // sunset time

    // Example problems
    std::vector<std::string> problems = {
        "=(T,726),PASS",
        "=(T,1),FAIL",
        "!(=(T,1)),PASS",
        "!(=(T,726)),FAIL",
        ">(T,725), PASS",
        ">(T,726), FAIL",
        "<(T,727), PASS",
        "<(T,726), FAIL",
        ">(T,R), PASS",
        "<(T,R), FAIL",
        "<(T,S), PASS",
        ">(T,S), FAIL",
        "&(>(T,R),=(T,726)),PASS",
        "&(<(T,R),=(T,726)),FAIL",
        "&(>(T,R),=(T,725)),FAIL",
        "&(<(T,R),=(T,725)),FAIL",
        "|(>(T,R),=(T,726)),PASS",
        "|(<(T,R),=(T,726)),PASS",
        "|(>(T,R),=(T,725)),PASS",
        "|(<(T,R),=(T,725)),FAIL",
        "=(T,M(726,725)),PASS",
        "=(T,M(726,727)),FAIL",
        "=(T,m(726,727)),PASS",
        "=(T,m(726,725)),FAIL",
        
    };

    for (const auto& p : problems) {
        char cmdBuf[16] = "initial12345678";

        const char* cmd = cmdBuf;
        ((char*)cmd)[15] = '\0';

        std::cout << cmd;

        bool send = solveProblem(p, T, D, R, S, cmd);

        std::cout << "Problem: " << p << "\n";
        if (send) {
            std::cout << "  SEND COMMAND: ";
            std::cout << cmd;
            std::cout << "\n";
        }
        else {
            std::cout << "  NO ACTION\n";
        }
        std::cout << "---------------------------------\n";
    }

    return 0;
}