#include "Template.hpp"

#include <iostream>

using Qentem::String;
using Qentem::Template;
using Qentem::Value;

namespace JSON = Qentem::JSON;

int main() {
    Value value;
    value["Equation"] = "1+4*2+1";
    value["one"]      = "1";
    value["three"]    = "3";

    String content(R"(
0.1+0.2 is: {math: 0.1  +   0.2 }
{var:Equation} = {math:{var:Equation}}; (1+8+1)
6^2 = {math:6^2}
--1 = {math:--1}
{var:one}+{var:three} = {math:{var:one}+{var:three}}
9 % 5 = {math:9 % 5}
)");

    std::cout << Template<>::Render(content, &value).Char() << '\n';

    /*
        Output:
            0.1+0.2 is: 0.3
            1+4*2+1 = 10; (1+8+1)
            6^2 = 36
            --1 = 1
            1+3 = 4
            9 % 5 = 4
    */
}
