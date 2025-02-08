#pragma once

#include <fstream>
#include <string>
#include <set>

enum class ReadingMode { NEW_LINE, READ_SECT, READ_VAR, READ_VAL, SKIP, END_SECT, END_VAR, WAIT_VAL };

struct parse_syntax_error : public std::exception {
    int row;
    std::string message;
    std::string fullMessage;

    parse_syntax_error(int row, const std::string& message);
    const char* what() const override;
};

class ini_parser {
private:
    std::string fileName;
    ReadingMode readingMode;
    std::string requiredSect, requiredVar, requiredValue;
    std::set<std::string> possibleVars;
    int rowsCount;

private:
    void set_requirements(const std::string& varName);

    void new_line(char symb, std::string& section, std::string& var);

    void read_sect(char symb, std::string& section);

    bool read_var(char symb, std::string& var);

    bool read_val(char symb, std::string& val);

    void skip(char symb);

    void end_sect(char symb);

    void end_var(char symb);

    void wait_val(char symb, std::string& val);

public:
    ini_parser(const std::string& fileName);

    template<typename T>
    T get_value(const std::string& varName);
};

std::string itos(int number);