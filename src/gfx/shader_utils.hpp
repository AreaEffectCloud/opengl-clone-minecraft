#pragma once
#include <string>

// ファイルからシェーダソースを読み込み、BOM を除去して返す。
// 失敗したら空文字列を返す。
std::string loadShaderSourceFromFile(const std::string& path);