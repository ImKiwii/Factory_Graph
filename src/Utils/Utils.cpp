#include "FactoryGraph/Utils/Utils.h"

#include <sstream>
#include <fstream>
#include <iostream>


//--------------------------------------------------
// Helper function to read a text file (for loading SQL scripts)
std::string ReadFileToString(const std::string& filePath)
{
	std::ifstream file(filePath);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << filePath << "\n";
		return "";
	}
	
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

//--------------------------------------------------
bool IsNumeric(const std::string& str) {
	if (str.empty()) return false;
	for (char c : str) {
		if (!std::isdigit(c)) return false;
	}
	return true;
}

//--------------------------------------------------
// Helper: check if string is a valid float > 0
bool IsPositiveFloat(const std::string & str, float & out) {
	try {
		out = std::stof(str);
		return out > 0;
	} catch (...) {
		return false;
	}
}