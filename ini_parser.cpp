#include "ini_parser.h"

std::string itos(int number) {
    std::string result;
    if (number == 0) {
        result += '0';
    }
    while (number != 0) {
        result.insert(result.cbegin(), number % 10 + '0');
        number /= 10;
    }
    return result;
}

parse_syntax_error::parse_syntax_error(int row, const std::string& message) : std::exception("syntax error"), row{ row }, message{ message } {
    fullMessage = "row " + itos(row) + ": " + message;
}

const char* parse_syntax_error::what() const {
    return fullMessage.c_str();
}

ini_parser::ini_parser(const std::string& fileName) : fileName{ fileName }, readingMode{ ReadingMode::NEW_LINE }, rowsCount{ 0 } {}

void ini_parser::new_line(char symb, std::string& section, std::string& var) {
    if (symb == '\n') {
        ++rowsCount;
    }
    else if (symb == ';') {
        readingMode = ReadingMode::SKIP;
    }
    else if (symb == '[') {
        readingMode = ReadingMode::READ_SECT;
        section.clear();
    }
    else if (isalpha(symb)) {
        if (section.size() != 0) {
            readingMode = ReadingMode::READ_VAR;
            var.clear();
            var += symb;
        }
        else {
            throw parse_syntax_error(rowsCount, "no section for variable");
        }
    }
    else if (isspace(symb)) {
        return;
    }
    else {
        throw parse_syntax_error(rowsCount, "wrong syntax");
    }
}

void ini_parser::read_sect(char symb, std::string& section) {
    if (symb==']') {
        readingMode = ReadingMode::END_SECT;
    }
    else if (isalnum(symb) || symb=='_') {
        section += symb;
    }
    else {
        throw parse_syntax_error(rowsCount, "wrong section syntax");
    }
}

bool ini_parser::read_var(char symb, std::string& var) {
    if (symb == '=') {
        readingMode = ReadingMode::WAIT_VAL;
        return true;
    }
    else if (symb == ' ') {
        readingMode = ReadingMode::END_VAR;
        return true;
    }
    else if (isalnum(symb) || symb == '_') {
        var += symb;
        return false;
    }
    else {
        throw parse_syntax_error(rowsCount, "wrong variable syntax");
    }
}

bool ini_parser::read_val(char symb, std::string& val) {
    if (symb == '\n') {
        ++rowsCount;
        readingMode = ReadingMode::NEW_LINE;
        return true;
    }
    else if (symb == ';') {
        readingMode = ReadingMode::SKIP;
        return true;
    }
    else {
        val += symb;
        return false;
    }
}

void ini_parser::skip(char symb) {
    if (symb == '\n') {
        ++rowsCount;
        readingMode = ReadingMode::NEW_LINE;
    }
}

void ini_parser::end_sect(char symb) {
    if (symb == '\n') {
        ++rowsCount;
        readingMode = ReadingMode::NEW_LINE;
    }
    else if (symb == ';') {
        readingMode = ReadingMode::SKIP;
    }
    else if (isspace(symb)) {
        return;
    }
    else {
        throw parse_syntax_error(rowsCount, "wrong section syntax");
    }
}

void ini_parser::end_var(char symb) {
    if (symb == '=') {
        readingMode = ReadingMode::WAIT_VAL;
    }
    else if (symb == ' ') {
        return;
    }
    else {
        throw parse_syntax_error(rowsCount, "wrong variable syntax");
    }
}

void ini_parser::wait_val(char symb, std::string& val) {
    if (symb == '\n') {
        ++rowsCount;
        readingMode = ReadingMode::NEW_LINE;
    }
    else if (symb == ';') {
        readingMode = ReadingMode::SKIP;
    }
    else if (symb == ' ') {
        return;
    }
    else {
        readingMode = ReadingMode::READ_VAL;
        val.clear();
        val += symb;
    }
}

std::string ini_parser::get_value_string(const std::string& aSection, const std::string& aVar) {
    if (data.empty()) {
        std::ifstream file(fileName);
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file");
        }
        std::istreambuf_iterator<char> fileIter(file);
        std::istreambuf_iterator<char> eofIter;
        readingMode = ReadingMode::NEW_LINE;
        rowsCount = 1;
        std::string section;
        std::string var;
        std::string value;
        try {
            for (; fileIter != eofIter; ++fileIter) {
                switch (readingMode) {
                case ReadingMode::NEW_LINE:
                    new_line(*fileIter, section, var);
                    break;

                case ReadingMode::READ_SECT:
                    read_sect(*fileIter, section);
                    break;

                case ReadingMode::READ_VAR:
                    if (read_var(*fileIter, var)) {
                        data[section][var] = "";
                    }
                    break;

                case ReadingMode::READ_VAL:
                    if (read_val(*fileIter, value)) {
                        data[section][var] = value;
                    }
                    break;

                case ReadingMode::SKIP:
                    skip(*fileIter);
                    break;

                case ReadingMode::END_SECT:
                    end_sect(*fileIter);
                    break;

                case ReadingMode::END_VAR:
                    end_var(*fileIter);
                    break;

                case ReadingMode::WAIT_VAL:
                    wait_val(*fileIter, value);
                    break;
                }
            }
        }
        catch (parse_syntax_error& err) {
            file.close();
            throw err;
        }
        file.close();
    }
    
    if (data.find(aSection) != data.end()) {
        if (data[aSection].find(aVar) != data[aSection].end()) {
            return data[aSection][aVar];
        }
        else {
            std::string error = "variable is not found; maybe you meant:";
            for (auto var : data[aSection]) {
                error += '\n' + var.first;                
            }
            throw std::runtime_error(error);
        }
    }
    else {
        throw std::runtime_error("section is not found");
    }
}