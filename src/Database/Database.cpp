#include "FactoryGraph/Database/DataBase.h"

#include "FactoryGraph/Core/Recipe.h"
#include "FactoryGraph/Core/Resource.h"

#include <cassert>
#include <iostream>
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
			const char * pInputQuery = "SELECT resource_id FROM RecipeInput WHERE recipe_id = ?";
			sqlite3_stmt * pInputStatement = nullptr;
    
			int inputResult = sqlite3_prepare_v2(m_pDatabase, pInputQuery, -1, &pInputStatement, nullptr);
			if (inputResult != SQLITE_OK)
			{
				std::cerr << "Failed to prepare query: " << GetLastError() << "\n";
				return;
			}
			
			sqlite3_bind_int(pInputStatement, 1, recipeID);
			
			while (sqlite3_step(pInputStatement) == SQLITE_ROW)
			{
				const char * pInputResourceQuery = "SELECT name FROM Resource WHERE resource_id = ?";
				sqlite3_stmt * pInputResourceStatement = nullptr;
    
				int inputResourceResult = sqlite3_prepare_v2(m_pDatabase, pInputResourceQuery, -1, &pInputResourceStatement, nullptr);
				if (inputResourceResult != SQLITE_OK)
				{
					std::cerr << "Failed to prepare query: " << GetLastError() << "\n";
					return;
				}
				
				int const resourceID = sqlite3_column_int(pInputStatement, 0);
				sqlite3_bind_int(pInputResourceStatement, 1, resourceID);
				
				while (sqlite3_step(pInputResourceStatement) == SQLITE_ROW)
				{
					inputResources.push_back(reinterpret_cast<const char*>(sqlite3_column_text(pInputResourceStatement, 0)));
				}
				
				sqlite3_finalize(pInputResourceStatement);
			}
			
			sqlite3_finalize(pInputStatement);
		}
		
		std::vector<std::string> outputResources;
		if (printOutputResources)
		{
			const char * pOutputQuery = "SELECT resource_id FROM RecipeOutput WHERE recipe_id = ?";
			sqlite3_stmt * pOutputStatement = nullptr;
    
			int outputResult = sqlite3_prepare_v2(m_pDatabase, pOutputQuery, -1, &pOutputStatement, nullptr);
			if (outputResult != SQLITE_OK)
			{
				std::cerr << "Failed to prepare query: " << GetLastError() << "\n";
				return;
			}
			
			sqlite3_bind_int(pOutputStatement, 1, recipeID);
			
			while (sqlite3_step(pOutputStatement) == SQLITE_ROW)
			{
				const char * pOutputResourceQuery = "SELECT name FROM Resource WHERE resource_id = ?";
				sqlite3_stmt * pOutputResourceStatement = nullptr;
    
				int outputResourceResult = sqlite3_prepare_v2(m_pDatabase, pOutputResourceQuery, -1, &pOutputResourceStatement, nullptr);
				if (outputResourceResult != SQLITE_OK)
				{
					std::cerr << "Failed to prepare query: " << GetLastError() << "\n";
					return;
				}
				
				int const resourceID = sqlite3_column_int(pOutputStatement, 0);
				sqlite3_bind_int(pOutputResourceStatement, 1, resourceID);
				
				while (sqlite3_step(pOutputResourceStatement) == SQLITE_ROW)
				{
					outputResources.push_back(reinterpret_cast<const char*>(sqlite3_column_text(pOutputResourceStatement, 0)));
				}
				
				sqlite3_finalize(pOutputResourceStatement);
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
