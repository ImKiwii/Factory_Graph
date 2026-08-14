#include <algorithm>

#include "FactoryGraph/Database/DataBase.h"

#include "FactoryGraph/Core/Recipe.h"
#include "FactoryGraph/Core/Resource.h"

#include <cassert>
#include <iostream>
#include <set>
#include <sqlite3.h>
#include <sstream>

//--------------------------------------------------
bool FDatabase::Open(std::string const & databasePath)
{
	if (m_pDatabase)
	{
		std::cerr << "Database already open. Close it first.\n";
		return false;
	}
    
	int const result = sqlite3_open(databasePath.c_str(), &m_pDatabase);
    
	if (result != SQLITE_OK)
	{
		std::cerr << "Failed to open database at " << databasePath << ": " << GetLastError() << "\n";
		m_pDatabase = nullptr;
		return false;
	}
    
	std::cout << "Database opened successfully at " << databasePath << "\n";
	return true;
}

//--------------------------------------------------
void FDatabase::Close()
{
	if (m_pDatabase)
	{
		sqlite3_close(m_pDatabase);
		m_pDatabase = nullptr;
		std::cout << "Database closed successfully.\n";
	}
}

//--------------------------------------------------
bool FDatabase::ExecuteScript(std::string const & sqlScript) const
{
	if (!m_pDatabase)
	{
		std::cerr << "Database is not open. Call Open() first.\n";
		return false;
	}
    
	char * pErrorMessage = nullptr;
	int const result = sqlite3_exec(m_pDatabase, sqlScript.c_str(), nullptr, nullptr, &pErrorMessage);
    
	if (result != SQLITE_OK)
	{
		std::cerr << "SQL error: " << (pErrorMessage ? pErrorMessage : "Unknown error") << "\n";
		if (pErrorMessage)
			sqlite3_free(pErrorMessage);
		return false;
	}
    
	std::cout << "SQL script executed successfully.\n";
	return true;
}

//--------------------------------------------------
FDatabase::~FDatabase()
{
    Close();
}




//--------------------------------------------------//
//													//
//				RESOURCE MANAGEMENT					//
//													//
//--------------------------------------------------//


//--------------------------------------------------
void FDatabase::PrintAllResources(bool const printIds) const
{    
    // Query all resources ordered by name
    const char * pQuery = printIds ? 
		"SELECT name, resource_id FROM Resource ORDER BY resource_id" :
		"SELECT name FROM Resource ORDER BY name";
	
    sqlite3_stmt * pStatement = nullptr;
    
    int const result = sqlite3_prepare_v2(m_pDatabase, pQuery, -1, &pStatement, nullptr);
    if (result != SQLITE_OK)
    {
        std::cerr << "Failed to prepare query: " << GetLastError() << "\n";
        return;
    }
    
	
    while (sqlite3_step(pStatement) == SQLITE_ROW)
    {
		const char * resourceName = reinterpret_cast<const char*>(sqlite3_column_text(pStatement, 0));
    	
    	if (printIds)
    	{
	    	int const resourceID = sqlite3_column_int(pStatement, 1);
	    	std::cout << resourceID << ": " << resourceName << ",\n";
    	}
		else
		{
	    	std::cout << resourceName << ",\n";
		}
    }
    
    sqlite3_finalize(pStatement);
}

//--------------------------------------------------
bool FDatabase::AddResourceToDatabase(std::string const & resourceName) const
{
	const char * pQuery = "INSERT INTO Resource (name) VALUES (?)";
	sqlite3_stmt * pStatement = nullptr;
	
	int const result = sqlite3_prepare_v2(m_pDatabase, pQuery, -1, &pStatement, nullptr);
	if (result != SQLITE_OK)
	{
		std::cout << "Failed to prepare query: " << GetLastError() << "\n";
		return false;
	}
	
	sqlite3_bind_text(pStatement, 1, resourceName.c_str(), -1, nullptr);
	
	bool const success = (sqlite3_step(pStatement) == SQLITE_DONE);
	if (!success)
	{
		std::cerr << "Insert failed: " << sqlite3_errmsg(m_pDatabase) << "\n";
	}
    
	sqlite3_finalize(pStatement);
	return success;
}

//--------------------------------------------------
bool FDatabase::RemoveResourceFromDatabase(int const resourceID) const
{
	const char * pQuery = "DELETE FROM Resource WHERE resource_id = ?";
	sqlite3_stmt * pStatement = nullptr;
	
	int const result = sqlite3_prepare_v2(m_pDatabase, pQuery, -1, &pStatement, nullptr);
	if (result != SQLITE_OK)
	{
		std::cout << "Failed to prepare query: " << GetLastError() << "\n";
		return false;
	}
	
	sqlite3_bind_int(pStatement, 1, resourceID);
	
	bool const success = (sqlite3_step(pStatement) == SQLITE_DONE);
	if (!success)
	{
		std::cerr << "Delete failed: " << sqlite3_errmsg(m_pDatabase) << "\n";
	}
    
	sqlite3_finalize(pStatement);
	return success;
}

//--------------------------------------------------
bool FDatabase::RemoveResourceFromDatabase(std::string const & resourceName) const
{
	const char * pQuery = "DELETE FROM Resource WHERE name = ?";
	sqlite3_stmt * pStatement = nullptr;
	
	int const result = sqlite3_prepare_v2(m_pDatabase, pQuery, -1, &pStatement, nullptr);
	if (result != SQLITE_OK)
	{
		std::cout << "Failed to prepare query: " << GetLastError() << "\n";
		return false;
	}
	
	sqlite3_bind_text(pStatement, 1, resourceName.c_str(), -1, nullptr);
	
	bool const success = (sqlite3_step(pStatement) == SQLITE_DONE);
	if (!success)
	{
		std::cerr << "Delete failed: " << sqlite3_errmsg(m_pDatabase) << "\n";
	}
    
	sqlite3_finalize(pStatement);
	return success;
}

//--------------------------------------------------
bool FDatabase::DoesResourcesExist(
	std::vector<FResource> const & resources
) const
{
	// Insert inside a set to avoid duplicates
	std::set<int> uniqueIds;
	for (FResource const & resource : resources)
	{
		uniqueIds.insert(resource.m_ResourceID);
	}

	// Create the query here
	std::string query = "SELECT COUNT(*) FROM Resource WHERE resource_id IN (";
	for (int i = 0; i < uniqueIds.size(); ++i)
	{
		query += "?";
		if (i < uniqueIds.size() - 1)
		{
			query += ",";
		}
	}
	query += ")";
	sqlite3_stmt * pStatement = nullptr;
    
	int const result = sqlite3_prepare_v2(m_pDatabase, query.c_str(), -1, &pStatement, nullptr);
	if (result != SQLITE_OK)
	{
		std::cerr << "Failed to prepare query: " << GetLastError() << "\n";
		return false;
	}
	
	int argIndex = 1;
	for (int id : uniqueIds)
	{
		sqlite3_bind_int(pStatement, argIndex++, id);
	}
	
	bool isValid = false;
	if (sqlite3_step(pStatement) == SQLITE_ROW)
	{
		int count = sqlite3_column_int(pStatement, 0);
		isValid = (count == static_cast<int>(uniqueIds.size()));
	}
	
    sqlite3_finalize(pStatement);
	return isValid;
}




//--------------------------------------------------//
//													//
//				  RECIPE MANAGEMENT  				//
//													//
//--------------------------------------------------//


//--------------------------------------------------
void FDatabase::PrintAllRecipes(
	bool printIds,
	bool printInputResources,
	bool printOutputResources
) const
{
	// Query all recipes ordered by name
	std::string query = "";
	
	if (printIds)
	{
		query = "SELECT name, recipe_id FROM Recipe ORDER BY recipe_id";
	}
	else
	{
		query = "SELECT name, recipe_id FROM Recipe ORDER BY name";
	}
	sqlite3_stmt * pStatement = nullptr;
    
	int const result = sqlite3_prepare_v2(m_pDatabase, query.c_str(), -1, &pStatement, nullptr);
	if (result != SQLITE_OK)
	{
		std::cerr << "Failed to prepare query: " << GetLastError() << "\n";
		return;
	}
    
	
	while (sqlite3_step(pStatement) == SQLITE_ROW)
	{
		const char * recipeName = reinterpret_cast<const char*>(sqlite3_column_text(pStatement, 0));
		int const recipeID = sqlite3_column_int(pStatement, 1);
    	
		if (printIds)
		{
			std::cout << recipeID << ": " << recipeName;
		}
		else
		{
			std::cout << recipeName;
		}
		
		if (!printInputResources && !printOutputResources)
		{
			std::cout << ",\n";
		}
		else
		{
			std::cout << ":\n";
		}
		
		std::vector<std::string> inputResources;
		if (printInputResources)
		{
			const char * pInputResourceNameQuery = 
				"SELECT name FROM Resource JOIN RecipeInput ON Resource.resource_id=RecipeInput.resource_id WHERE recipe_id=?";
			sqlite3_stmt * pInputStatement = nullptr;
    
			int inputResult = sqlite3_prepare_v2(m_pDatabase, pInputResourceNameQuery, -1, &pInputStatement, nullptr);
			if (inputResult != SQLITE_OK)
			{
				std::cerr << "Failed to prepare query: " << GetLastError() << "\n";
				return;
			}
			
			sqlite3_bind_int(pInputStatement, 1, recipeID);
			
			while (sqlite3_step(pInputStatement) == SQLITE_ROW)
			{
				inputResources.push_back(reinterpret_cast<const char*>(sqlite3_column_text(pInputStatement, 0)));
			}
			
			sqlite3_finalize(pInputStatement);
		}
		
		std::vector<std::string> outputResources;
		if (printOutputResources)
		{
			const char * pOutputResourceNameQuery = 
				"SELECT name FROM Resource JOIN RecipeOutput ON Resource.resource_id=RecipeOutput.resource_id WHERE recipe_id=?";
			sqlite3_stmt * pOutputStatement = nullptr;
    
			int outputResult = sqlite3_prepare_v2(m_pDatabase, pOutputResourceNameQuery, -1, &pOutputStatement, nullptr);
			if (outputResult != SQLITE_OK)
			{
				std::cerr << "Failed to prepare query: " << GetLastError() << "\n";
				return;
			}
			
			sqlite3_bind_int(pOutputStatement, 1, recipeID);
			
			while (sqlite3_step(pOutputStatement) == SQLITE_ROW)
			{
				outputResources.push_back(reinterpret_cast<const char*>(sqlite3_column_text(pOutputStatement, 0)));
			}
			
			sqlite3_finalize(pOutputStatement);
		}
		
		int inputSize = inputResources.size();
		int outputSize = outputResources.size();
		for (int iResource = 0; iResource < std::max(inputSize, outputSize); iResource++)
		{
			if (iResource < inputSize)
			{
				std::cout << "\t " << inputResources[iResource];
			}
			else
			{
				std::cout << "\t\t\t";
			}
			
			if (iResource == 0)
			{
				std::cout << "\t\t => \t";
			}
			else
			{
				std::cout << "\t\t\t\t";
			}
			
			if (iResource < outputSize)
			{
				std::cout << outputResources[iResource];
			}
			std::cout << "\n";
		}
		std::cout << "---------------------------------------------------\n";
	}
    
	sqlite3_finalize(pStatement);
}

//--------------------------------------------------
bool FDatabase::AddRecipeToDatabase(
	std::string const & recipeName,
	std::vector<FResource> const & resourcesInput,
	std::vector<FResource> const & resourcesOutput
) const
{
	// we first have to check if all the resources exist
	
	std::vector<FResource> allResources = resourcesInput;
	allResources.insert(allResources.end(), resourcesOutput.begin(), resourcesOutput.end());
	
	if (!DoesResourcesExist(allResources))
	{
		std::cerr << "All listed resources doesn't exist in the database\n";
		return false;
	}
	
	// Start transaction
	if (sqlite3_exec(m_pDatabase, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr) != SQLITE_OK) {
		std::cerr << "Transaction begin failed: " << GetLastError() << "\n";
		return false;
	}
	
	// Add the recipe into the Recipe table
	{
		const char * pQuery = "INSERT INTO Recipe (name) VALUES (?)";
		sqlite3_stmt * pStatement = nullptr;
	
		int const result = sqlite3_prepare_v2(m_pDatabase, pQuery, -1, &pStatement, nullptr);
		if (result != SQLITE_OK)
		{
			std::cout << "Failed to prepare query: " << GetLastError() << "\n";
			sqlite3_exec(m_pDatabase, "ROLLBACK;", nullptr, nullptr, nullptr);
			return false;
		}
	
		sqlite3_bind_text(pStatement, 1, recipeName.c_str(), -1, nullptr);
	
		if (sqlite3_step(pStatement) != SQLITE_DONE)
		{
			std::cerr << "Insert failed: " << sqlite3_errmsg(m_pDatabase) << "\n";
			sqlite3_exec(m_pDatabase, "ROLLBACK;", nullptr, nullptr, nullptr);
			sqlite3_finalize(pStatement);
			return false;
		}
		sqlite3_finalize(pStatement);
	}
	
	int recipeID = sqlite3_last_insert_rowid(m_pDatabase);
	
	// Add the input resources into the RecipeInput table
	{		
		const char * pQuery = "INSERT INTO RecipeInput (recipe_id, resource_id, amount) VALUES (?, ?, ?)";
		sqlite3_stmt * pStatement = nullptr;
	
		int const result = sqlite3_prepare_v2(m_pDatabase, pQuery, -1, &pStatement, nullptr);
		if (result != SQLITE_OK)
		{
			std::cout << "Failed to prepare query: " << GetLastError() << "\n";
			sqlite3_exec(m_pDatabase, "ROLLBACK;", nullptr, nullptr, nullptr);
			return false;
		}
		
		for (FResource const & resource : resourcesInput)
		{
			sqlite3_bind_int(pStatement, 1, recipeID);
        	sqlite3_bind_int(pStatement, 2, resource.m_ResourceID);
        	sqlite3_bind_double(pStatement, 3, resource.m_ResourceCount);
        	
        	if (sqlite3_step(pStatement) != SQLITE_DONE)
        	{
        		std::cerr << "Insert failed: " << sqlite3_errmsg(m_pDatabase) << "\n";
        		sqlite3_exec(m_pDatabase, "ROLLBACK;", nullptr, nullptr, nullptr);
        		sqlite3_finalize(pStatement);
        		return false;
        	}
			
			// reset statement for the next iteration
			sqlite3_reset(pStatement);
		}
		
		sqlite3_finalize(pStatement);
	}
	
	// Add the output resources into the RecipeOutput table
	{		
		const char * pQuery = "INSERT INTO RecipeOutput (recipe_id, resource_id, amount) VALUES (?, ?, ?)";
		sqlite3_stmt * pStatement = nullptr;
	
		int const result = sqlite3_prepare_v2(m_pDatabase, pQuery, -1, &pStatement, nullptr);
		if (result != SQLITE_OK)
		{
			std::cout << "Failed to prepare query: " << GetLastError() << "\n";
			sqlite3_exec(m_pDatabase, "ROLLBACK;", nullptr, nullptr, nullptr);
			return false;
		}
		
		for (FResource const & resource : resourcesOutput)
		{
			sqlite3_bind_int(pStatement, 1, recipeID);
        	sqlite3_bind_int(pStatement, 2, resource.m_ResourceID);
        	sqlite3_bind_double(pStatement, 3, resource.m_ResourceCount);
        	
        	if (sqlite3_step(pStatement) != SQLITE_DONE)
        	{
        		std::cerr << "Insert failed: " << sqlite3_errmsg(m_pDatabase) << "\n";
        		sqlite3_exec(m_pDatabase, "ROLLBACK;", nullptr, nullptr, nullptr);
        		sqlite3_finalize(pStatement);
        		return false;
        	}
			
			// reset statement for the next iteration
			sqlite3_reset(pStatement);
		}
		
		sqlite3_finalize(pStatement);
	}
	
	// Commit transaction - all or nothing
	if (sqlite3_exec(m_pDatabase, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK) {
		std::cerr << "Commit failed: " << GetLastError() << "\n";
		sqlite3_exec(m_pDatabase, "ROLLBACK;", nullptr, nullptr, nullptr);
		return false;
	}
    
	std::cout << "Recipe added successfully with ID: " << recipeID << "\n";
	return true;
}

//--------------------------------------------------
bool FDatabase::RemoveRecipeFromDatabase(int recipeID) const
{
	// Start transaction
	if (sqlite3_exec(m_pDatabase, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr) != SQLITE_OK) {
		std::cerr << "Transaction begin failed: " << GetLastError() << "\n";
		return false;
	}
	
	// Remove the recipe from the Recipe table
	{
		const char * pQuery = "DELETE FROM Recipe WHERE recipe_id = ?";
		sqlite3_stmt * pStatement = nullptr;
	
		int const result = sqlite3_prepare_v2(m_pDatabase, pQuery, -1, &pStatement, nullptr);
		if (result != SQLITE_OK)
		{
			std::cout << "Failed to prepare query: " << GetLastError() << "\n";
			sqlite3_exec(m_pDatabase, "ROLLBACK;", nullptr, nullptr, nullptr);
			return false;
		}
	
		sqlite3_bind_int(pStatement, 1, recipeID);
	
		if (sqlite3_step(pStatement) != SQLITE_DONE)
		{
			std::cerr << "Delete failed: " << sqlite3_errmsg(m_pDatabase) << "\n";
			sqlite3_exec(m_pDatabase, "ROLLBACK;", nullptr, nullptr, nullptr);
			sqlite3_finalize(pStatement);
			return false;
		}
		sqlite3_finalize(pStatement);
	}
	
	// Remove the input resources from the RecipeInput table
	{		
		const char * pQuery = "DELETE FROM RecipeInput WHERE recipe_id = ?";
		sqlite3_stmt * pStatement = nullptr;
	
		int const result = sqlite3_prepare_v2(m_pDatabase, pQuery, -1, &pStatement, nullptr);
		if (result != SQLITE_OK)
		{
			std::cout << "Failed to prepare query: " << GetLastError() << "\n";
			sqlite3_exec(m_pDatabase, "ROLLBACK;", nullptr, nullptr, nullptr);
			return false;
		}
		
		sqlite3_bind_int(pStatement, 1, recipeID);
		
		if (sqlite3_step(pStatement) != SQLITE_DONE)
		{
			std::cerr << "Delete failed: " << sqlite3_errmsg(m_pDatabase) << "\n";
			sqlite3_exec(m_pDatabase, "ROLLBACK;", nullptr, nullptr, nullptr);
			sqlite3_finalize(pStatement);
			return false;
		}
		sqlite3_finalize(pStatement);
	}
	
	// Remove the output resources from the RecipeOutput table
	{		
		const char * pQuery = "DELETE FROM RecipeOutput WHERE recipe_id = ?";
		sqlite3_stmt * pStatement = nullptr;
	
		int const result = sqlite3_prepare_v2(m_pDatabase, pQuery, -1, &pStatement, nullptr);
		if (result != SQLITE_OK)
		{
			std::cout << "Failed to prepare query: " << GetLastError() << "\n";
			sqlite3_exec(m_pDatabase, "ROLLBACK;", nullptr, nullptr, nullptr);
			return false;
		}
		
		sqlite3_bind_int(pStatement, 1, recipeID);
		
		if (sqlite3_step(pStatement) != SQLITE_DONE)
		{
			std::cerr << "Delete failed: " << sqlite3_errmsg(m_pDatabase) << "\n";
			sqlite3_exec(m_pDatabase, "ROLLBACK;", nullptr, nullptr, nullptr);
			sqlite3_finalize(pStatement);
			return false;
		}
		sqlite3_finalize(pStatement);
	}
	
	// Commit transaction - all or nothing
	if (sqlite3_exec(m_pDatabase, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK) {
		std::cerr << "Commit failed: " << GetLastError() << "\n";
		sqlite3_exec(m_pDatabase, "ROLLBACK;", nullptr, nullptr, nullptr);
		return false;
	}
    
	std::cout << "Recipe deleted successfully with ID: " << recipeID << "\n";
	return true;
}

//--------------------------------------------------
bool FDatabase::RemoveRecipeFromDatabase(std::string const & recipeName) const
{
	bool success = false;
	// Get the recipe id
	{
		const char * pQuery = "SELECT recipe_id FROM Recipe WHERE name = ?";
		sqlite3_stmt * pStatement = nullptr;
	
		int const result = sqlite3_prepare_v2(m_pDatabase, pQuery, -1, &pStatement, nullptr);
		if (result != SQLITE_OK)
		{
			std::cout << "Failed to prepare query: " << GetLastError() << "\n";
			return false;
		}
	
		sqlite3_bind_text(pStatement, 1, recipeName.c_str(), -1, nullptr);
	
		while (sqlite3_step(pStatement) == SQLITE_ROW)
		{
			int const recipeID = sqlite3_column_int(pStatement, 0);
			success = RemoveRecipeFromDatabase(recipeID);
		}

		sqlite3_finalize(pStatement);
	}
	
	return success;
}

//--------------------------------------------------
std::vector<FRecipe> FDatabase::LoadRecipes()
{
	return {};
}



//--------------------------------------------------
std::string FDatabase::GetLastError() const
{
	if (m_pDatabase == nullptr)
		return "No database connection";
    
	return std::string(sqlite3_errmsg(m_pDatabase));
}
