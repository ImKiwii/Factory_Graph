#pragma once

#include <string>

//--------------------------------------------------
// Helper function to read a text file (for loading SQL scripts)
std::string ReadFileToString(const std::string & filePath);

//--------------------------------------------------
bool IsNumeric(const std::string & str);

//--------------------------------------------------
// Helper: check if string is a valid float > 0
bool IsPositiveFloat(const std::string & str, float & out);
