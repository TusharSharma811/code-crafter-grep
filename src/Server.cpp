#include <iostream>
#include <string>
#include <vector>
// multiply strings by ints
std::string operator * (std::string a, unsigned int b) {
    std::string output = "";
    while (b--) {
        output += a;
    }
    return output;
}
bool f(std::string input, std::string characters) {
    for (int i = 0; i < input.length(); i++) {
        char c = input[i];
        for (int j = 0; j < characters.size(); j++) {
            char cchar = characters[j];
            if (cchar == c) {
                return true;
            }
        }
    }
    return false;
}
struct match {
    bool wasBackslash = false;
    bool readingCgroup = false;
    bool negCgroup = false;
    int readCount = 0;
    bool foundOverride = false;
    std::string prevExp;
};
std::vector<char> cgroup;
int match_pattern(const std::string& input_line, const std::string& pattern); // forward declaration, because isMatch needs to call this function
bool isMatch(int pos, int pattern_pos, const std::string& input_line, const std::string& pattern, match& currMatch) {
    char pattern_c = pattern[pattern_pos];
    // must be up here because the pos should be past the input_line length
    if (pattern_c == '$' && !currMatch.readingCgroup) {
        // end of line
        if (pos != input_line.length()) {
            return false;
        }
        if (pattern_pos != pattern.length() - 1) { // pattern abc$a should always fail, exception is abc$+
            for (int i = pattern_pos + 1; i < pattern.length(); i++) {
                char curr_c = pattern[i];
                if (curr_c != '+') {
                    return false;
                }
            }
        }
        currMatch.prevExp = '$';
        return true;
    }
    if (pos >= input_line.length() && ((pattern_c != '+' && pattern_c != '?') || currMatch.readingCgroup)) {
        // out of range of input line, must be invalid
        return false;
    }
    char c = input_line[pos];
    
    if (pattern_c == '[') {
        currMatch.readingCgroup = true;
        currMatch.negCgroup = false;
        return true;
    }
    if (pattern_c == ']') {
        currMatch.readingCgroup = false;
        std::string tempExp = "[";
        if (currMatch.negCgroup) {
            tempExp = "[^";
        }
        bool found = false;
        for (char cGroupC : cgroup) {
            tempExp += cGroupC;
            if (!found && cGroupC == c) {
                if (currMatch.negCgroup) {
                    cgroup.clear();
                    return false;
                }
                currMatch.readCount++;
                found = true;
            }
        }
        cgroup.clear();
        if (!currMatch.negCgroup && !found) return false;
        currMatch.prevExp += "]";
        currMatch.readCount++;
        return true;
    }
    if (pattern_c == '\\') {
        currMatch.wasBackslash = true;
        return true;
    }
    if (currMatch.readingCgroup) {
        if (pattern_c == '^' && cgroup.empty()) {
            currMatch.negCgroup = true;
            return true;
        }
        if (currMatch.wasBackslash) {
            currMatch.wasBackslash == false;
            if (pattern_c == 'd') {
                for (char num : "0123456789") {
                    cgroup.push_back(num);
                }
            }
            if (pattern_c == 'w') {
                for (char alpha : "abcdefghijklmnopqrstuvxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_") {
                    cgroup.push_back(alpha);
                }
            }
            return true;
        }
        cgroup.push_back(pattern_c);
        return true;
    }
    if (currMatch.wasBackslash) {
        currMatch.wasBackslash = false;
        if (pattern_c == 'd') {
            for (char num : "0123456789") {
                if (c == num) {
                    currMatch.readCount++;
                    currMatch.prevExp = "\\d";
                    return true;
                }
            }
            return false;
        } else if (pattern_c == 'w') {
            for (char alpha : "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_") {
                if (c == alpha) {
                    currMatch.readCount++;
                    currMatch.prevExp = "\\w";
                    return true;
                }
            }
        }
        return false;
    }
    if (pattern_c == '^') {
        if (pattern_pos != 0) {
            // by definition, there must be something before the ^, so it is impossible
            return false;
        }
        // handled in match_pattern, if it is not inside [], so ignore
        currMatch.prevExp = "^";
        return true;
    }
    if (pattern_c == '+') {
        std::string newInput = input_line.substr(pos);
        std::string newPattern = pattern.substr(pattern_pos + 1);
        int matchPos = match_pattern(newInput, newPattern);
        if (matchPos == -1) {
            return false;
        }
        if (currMatch.prevExp == "^" && matchPos != 0) {
            return false; // fail because it was not matched on the first character
        }
        // check if everything in-between matches
        newInput = newInput.substr(0, matchPos);
        newPattern = currMatch.prevExp * matchPos; // works because exp accounts for one character each
        matchPos = match_pattern(newInput, newPattern);
        if (matchPos == -1) {
            return false;
        }
        currMatch.foundOverride = true;
        return true;
    }
    if (pattern_c == '?') {
        std::string newInput = input_line.substr(pos - 1);
        std::string newPattern = '^' + pattern.substr(pattern_pos + 1);
        if (match_pattern(newInput, newPattern) == 0) {
            currMatch.foundOverride = true;
        }
        return true;
    }
   
    if (c == pattern_c || pattern_c == '.') {
        currMatch.readCount++;
        currMatch.prevExp = c;
        currMatch.prevExp = pattern_c; // use pattern_c instead of c because of the . wildcard
        return true;
    } else {
        return false;
    }
}
// returns the startline
int match_pattern(const std::string& input_line, const std::string& pattern) {
    if (pattern.empty()) {
        return 0;
    }
    for (int i = 0; i <= input_line.length(); i++) { // the = is so that it tries anyways if the input is empty, and to account for empty $s
        bool isFound = true;
        match currMatch;
        for (int j = 0; j < pattern.length(); j++) {
            int pos = i + currMatch.readCount;
            if (!isMatch(pos, j, input_line, pattern, currMatch)) {
                if (j != pattern.length() - 1) {
                    // as long as its not the last char
                    if (pattern[j + 1] == '?') {
                        j++; // skip the ?
                        continue;
                    }
                }
                isFound = false;
                break;
            }
            if (currMatch.foundOverride) {
                return i;
            }
        }
        if (isFound) return i;
        if (pattern[0] == '^'){ return -1;}
    }
    return -1;
}
int main(int argc, char* argv[]) {
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    if (argc != 3) {
        std::cerr << "Expected two arguments" << std::endl;
        return 1;
    }
    std::string flag = argv[1];
    std::string pattern = argv[2];
    if (flag != "-E") {
        std::cerr << "Expected first argument to be '-E'" << std::endl;
        return 1;
    }
    std::string input_line;
    std::getline(std::cin, input_line);
    try {
        if (match_pattern(input_line, pattern) != -1 && !pattern.empty()) { // match_pattern will return 0 if pattern.empty(), to account for the +, but that is not the behaviour of grep for some reason
            return 0;
        } else {
            return 1;
        }
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}