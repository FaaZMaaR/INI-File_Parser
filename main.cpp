#include <iostream>
#include <cstring>

#include "ini_parser.h"

int main() {
    ini_parser parser("test.ini");
    try {
        auto value = parser.get_value<std::string>("Section1.var1");
        auto int_value = parser.get_value<int>("Section1.var2");
        auto double_value = parser.get_value<double>("Section1.var3");
        std::cout << value << std::endl;
        std::cout << int_value << std::endl;
        std::cout << double_value << std::endl;
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}