#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <string_view>
bool matchre(std::string_view input_line, std::string_view pattern)
{
    if (!pattern.size())
        return true;
    std::string_view::size_type i = 0;
    for (std::string_view::size_type j = 0; i < pattern.size() && j < input_line.size(); ++i, ++j)
    {
        if (pattern[i] == '\\' && i + 1 != pattern.size() && pattern[i + 1] == 'd')
        {
            if (!std::isdigit(input_line[j]))
                return false;
            ++i;
        }
        else if (pattern[i] == '\\' && i + 1 != pattern.size() && pattern[i + 1] == 'w')
        {
            if (!std::isalnum(input_line[j]))
                return false;
            ++i;
        }
        else if (pattern[i] != input_line[j])
        {
            return false;
        }
    }
    if (i != pattern.size())
        return false;
    return true;
}
bool match_pattern(std::string_view input_line, std::string_view pattern)
{
    if (pattern[0] == '^')
    {
        return input_line == std::string_view{pattern.data() + 1, pattern.size() - 1};
    }
    if (pattern.back() == '$')
    {
        return input_line == std::string_view{pattern.data(), pattern.size() - 1};
    }
    if (auto res = pattern.find('+'); res != pattern.npos)
    {
        std::string_view p1{pattern.begin(), pattern.begin() + res};
        std::string_view p2{pattern.begin() + res + 1, pattern.end()};
        return input_line.find(p1) != input_line.npos && input_line.rfind(p2) != input_line.npos;
    }
    if (auto res = pattern.find('?'); res != pattern.npos && res > 0)
    {
        std::string_view p1{pattern.begin(), pattern.begin() + res - 1};
        std::string_view p2{pattern.begin(), pattern.begin() + res};
        std::string_view p3{pattern.begin() + res + 1, pattern.end()};
        return input_line.find(std::string(p1) + std::string(p3)) != std::string::npos ||
            input_line.find(std::string(p2) + std::string(p3)) != std::string::npos;
    }
    if (pattern[0] == '\\' && pattern[1] == 'd')
    {
        auto is_digit = [](char c) { return std::isdigit(c); };
        if (auto res = std::ranges::find_if(input_line, is_digit); res != input_line.end())
            return matchre({res + 1, input_line.end()}, {pattern.data() + 2, pattern.size() - 2});
        return false;
    }
    if (pattern[0] == '\\' && pattern[1] == 'w')
    {
        auto is_alnum = [](char c) { return std::isalnum(c) || c == '_'; };
        if (auto res = std::ranges::find_if(input_line, is_alnum); res != input_line.end())
            return matchre({res + 1, input_line.end()}, {pattern.data() + 2, pattern.size() - 2});
        return false;
    }
    if (pattern[0] == '[' && pattern[1] != '^' && pattern.back() == ']')
    {
        return std::ranges::find_first_of(input_line, std::string_view{pattern.data() + 1, pattern.size() - 2}) !=
            input_line.end();
    }
    if (pattern[0] == '[' && pattern[1] == '^' && pattern.back() == ']')
    {
        return std::ranges::find_first_of(input_line, std::string_view{pattern.data() + 2, pattern.size() - 3}) ==
            input_line.end();
    }
    if (auto res = std::ranges::find(input_line, pattern[0]); res != input_line.end())
    {
        return matchre({res + 1, input_line.end()}, {pattern.data() + 1, pattern.size() - 1});
    }
    return false;
}
int main(int argc, char* argv[])
{
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    // You can use print statements as follows for debugging, they'll be visible when running tests.
    std::cout << "Logs from your program will appear here" << std::endl;
    if (argc != 3)
    {
        std::cerr << "Expected two arguments" << std::endl;
        return 1;
    }
    std::string flag = argv[1];
    std::string pattern = argv[2];
    if (flag != "-E")
    {
        std::cerr << "Expected first argument to be '-E'" << std::endl;
        return 1;
    }
    std::string input_line;
    std::getline(std::cin, input_line);
    try
    {
        if (match_pattern(input_line, pattern))
        {
            std::cout << "0 returned" << std::endl;
            return 0;
        }
        else
        {
            std::cerr << "1 returned" << std::endl;
            return 1;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}