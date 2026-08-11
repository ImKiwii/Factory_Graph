#pragma once

#include <string>
#include <vector>

struct FRecipe;

//--------------------------------------------------
struct FDatabase
{
	// Open the database at the given path
	// return true if successful
	bool Open(std::string const & databasePath);
	
	// Closes the database connection
	void Close();
	
	// Executes a SQL script
	// Returns true if successful, false otherwise
	bool ExecuteScript(std::string const & sqlScript) const;
	
	// Loads all recipes from the database
	// Returns a vector of FRecipe objects populated from the database
	std::vector<FRecipe> LoadRecipes();
    
	// Destructor: ensures database is properly closed
	~FDatabase();

	void PrintAllResources(bool printIds) const;
	// return true if the insert is successful
	bool AddResourceToDatabase(std::string const & resourceName) const;
	bool RemoveResourceFromDatabase(int resourceID) const;
	bool RemoveResourceFromDatabase(std::string const & resourceName) const;
	
	void PrintAllRecipes(bool printIds, bool printInputResources, bool printOutputResources) const;
	
private:
	class sqlite3 * m_pDatabase = nullptr;
    
	// Helper to get the last error message from SQLite
	std::string GetLastError() const;
};