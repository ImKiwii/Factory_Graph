#pragma once

#include <string>
#include <vector>

struct FRecipe;
struct FResource;

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
    
	// Destructor: ensures database is properly closed
	~FDatabase();

	//--------------------------//
	//		  Resources			//
	//--------------------------//
	void PrintAllResources(bool printIds) const;
	// return true if the insert is successful
	bool AddResourceToDatabase(std::string const & resourceName) const;
	bool RemoveResourceFromDatabase(int resourceID) const;
	bool RemoveResourceFromDatabase(std::string const & resourceName) const;
	
	bool DoesResourcesExist(
		std::vector<FResource> const & resources
	) const;
	
	//--------------------------//
	//			Recipes			//
	//--------------------------//
	void PrintAllRecipes(bool printIds, bool printInputResources, bool printOutputResources) const;
	// return true if the insert is successful
	bool AddRecipeToDatabase(
		std::string const & recipeName,
		std::vector<FResource> const & resourcesInput,
		std::vector<FResource> const & resourcesOutput
	) const;
	bool RemoveRecipeFromDatabase(int recipeID) const;
	bool RemoveRecipeFromDatabase(std::string const & recipeName) const;
	
	// Loads all recipes from the database
	// Returns a vector of FRecipe objects populated from the database
	std::vector<FRecipe> LoadRecipes();
	
private:
	class sqlite3 * m_pDatabase = nullptr;
    
	// Helper to get the last error message from SQLite
	std::string GetLastError() const;
};