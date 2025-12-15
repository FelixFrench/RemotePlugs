#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cctype>
#include <cstring>
#include <cstdio>
#include "LightsInstructions.h"

namespace {

    // Recursive-descent parser
    struct Parser {
        const std::string& s;
        size_t pos = 0;
        uint16_t T, D, R, S;

        Parser(const std::string& str, uint16_t t, uint16_t d, uint16_t r, uint16_t s_)
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
        bool parseInt(uint16_t& y) {
            skip();

            if (pos >= s.size())
                return false;

            // Parse a numeric literal
            if (std::isdigit(s[pos])) {
                uint16_t v = 0;
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
                uint16_t a, b;
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

            char op = s[pos++];

            if (!expect('(')) return false;

            // bool -> bool
            if (op == '!') {
                bool v;
                if (!parseBool(v)
                    || !expect(')')) return false;
                y = !v;
                return true;
            }

            // int, int -> bool
            if (op == '=' || op == '<' || op == '>') {
                uint16_t a, b;
                bool dayComparison = s[pos] == 'D';
                if (!parseInt(a)
                    || !expect(',')
                    || !parseInt(b)
                    || !expect(')')) return false;

                // Equality operator
                if (op == '=') {
                    // If comparing days, handle the weekday/weekend special cases
                    if (dayComparison && b == 8) {
                        // True if D is a weekday
                        y = (D >= 1 && D <= 5);
                    }
                    else if (dayComparison && b == 9) {
                        // True if D is a weekend day
                        y = (D == 0 || D == 6);
                    }
                    else {
                        // 
                        y = (a == b);
                    }
                }
                else {
                    y = (op == '<') ? (a < b) : (a > b);
                }
                return true;
            }

            // bool, bool -> bool
            if (op == '&' || op == '|') {
                bool a, b;
                if (!parseBool(a)
                    || !expect(',')
                    || !parseBool(b)
                    || !expect(')')) return false;
                y = (op == '&') ? (a && b) : (a || b);
                return true;
            }

            return false;
        }
    };

} // namespace

bool solveProblem(
    const std::string& problem,
    uint16_t T, uint16_t D, uint16_t R, uint16_t S,
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

void RunTestCase(uint16_t T, uint16_t D, uint16_t R, uint16_t S,
    const std::string& problem,
    bool expectedBool,
    const char* expectedCmd = nullptr
) {
    char returnedCmd[16] = "\0";

    bool returnedBool = solveProblem(problem, T, D, R, S, returnedCmd);
}


int main() {

    RunTestCase(0, 0, 0, 0, "=(123,123),xyz", true, "xyz");



    // Synthetic values
    uint16_t T = 726;   // minutes since midnight
    uint16_t D = 3;     // days since Sunday (3 = Wednesday)
    uint16_t R = 400;   // sunrise time
    uint16_t S = 1100;  // sunset time

    // Example problems
    std::vector<std::string> problems = {
        // Equality
        "=(T,726),PASS",
        "=(T,1),FAIL",
        // Not
        "!(=(T,1)),PASS",
        "!(=(T,726)),FAIL",
        // Greater than
        ">(2,1), PASS",
        ">(1,1), FAIL",
        // Less than
        "<(1,2), PASS",
        "<(1,1), FAIL",
        // Sunrise
        ">(T,R), PASS",
        "<(T,R), FAIL",
        // Sunset
        "<(T,S), PASS",
        ">(T,S), FAIL",
        // Logical and
        "&(>(T,R),=(T,726)),PASS",
        "&(<(T,R),=(T,726)),FAIL",
        "&(>(T,R),=(T,725)),FAIL",
        "&(<(T,R),=(T,725)),FAIL",
        // Logical or
        "|(>(T,R),=(T,726)),PASS",
        "|(<(T,R),=(T,726)),PASS",
        "|(>(T,R),=(T,725)),PASS",
        "|(<(T,R),=(T,725)),FAIL",
        // Maximum
        "=(T,M(726,725)),PASS",
        "=(T,M(726,727)),FAIL",
        // Minimum
        "=(T,m(726,727)),PASS",
        "=(T,m(726,725)),FAIL",
        // Weekday/weekend
        "=(D,W),PASS",
        "=(D,w),FAIL",

        "=(,FAIL",              // Missing )
        "(1,1),FAIL",           // No function given
        "=(!(=(1,1)),1),FAIL",  // Invalid use of bool in =
        "!(1)",                 // Invalid use of integer literal in !
    };

    for (const auto& p : problems) {
        char cmdBuf[16] = "initial12345678";

        const char* cmd = cmdBuf;
        ((char*)cmd)[15] = '\0';

        std::cout << cmd;

        bool send = solveProblem(p, T, D, R, S, cmd);

        std::cout << "Problem: " << p << "\n";
        if (send) {
            std::cout << "  SEND COMMAND: " << cmd << "\n";
        }
        else {
            std::cout << "  NO ACTION\n";
        }
        std::cout << "---------------------------------\n";
    }

    return 0;
}