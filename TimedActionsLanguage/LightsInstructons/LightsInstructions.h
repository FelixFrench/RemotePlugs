#pragma 
#include <string>

// Evaluates a problem expression and possibly writes a command into `cmd`.
// Returns true if a command should be sent (including "error"), false otherwise.
// Evaluates a timed action statement, returning false if no action is required, or true if either an action is reuqied or an
//  error occurred.
// If an action is required then cmd is populated with the command code. If an error occurs then the command code is "error"
//    |Status              | return value | cmd           |
//    |--------------------|--------------|---------------|
//    |No action required  | false        | undefined     |
//    |Action required     | true         | command code  |
//    |Error               | true         | "error"       |
bool solveProblem(
    const std::string& problem,
    uint16_t T, uint16_t D, uint16_t R, uint16_t S,
    const char* cmd
);
