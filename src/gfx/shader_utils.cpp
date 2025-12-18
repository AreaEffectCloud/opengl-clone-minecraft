#include "shader_utils.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <cctype>

// UTF-8 BOM を先頭から除去
static void strip_utf8_bom(std::string &s) {
    if (s.size() >= 3 &&
        static_cast<unsigned char>(s[0]) == 0xEF &&
        static_cast<unsigned char>(s[1]) == 0xBB &&
        static_cast<unsigned char>(s[2]) == 0xBF) {
        s.erase(0, 3);
    }
}

std::string loadShaderSourceFromFile(const std::string& path) {
    std::ifstream ifs(path, std::ios::in | std::ios::binary);
    if (!ifs) {
        std::cerr << "[shader_utils] failed to open shader file: " << path << std::endl;
        return std::string();
    }
    std::ostringstream ss;
    ss << ifs.rdbuf();
    std::string src = ss.str();
    if (src.empty()) {
        std::cerr << "[shader_utils] shader file is empty: " << path << std::endl;
        return std::string();
    }
    strip_utf8_bom(src);
    return src;
}