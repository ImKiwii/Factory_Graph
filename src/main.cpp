
#include <iostream>
#include <map>
#include <queue>
#include <unordered_map>

#include "FactoryGraph/Core/Recipe.h"
#include "FactoryGraph/Core/Resource.h"
#include "FactoryGraph/Database/Database.h"
#include "FactoryGraph/Utils/Utils.h"


//--------------------------------------------------
static void EditDatabase(FDatabase const & database)
{
	while (true)
	{
		std::string playerChoice_EditDatabase = "";
		std::cout << "---- Edit Database ----\n";
		std::cout << "1 - Add resource to Database\n";
		std::cout << "2 - Remove resource from Database\n";
		std::cout << "3 - Add recipe to Database (@todo - waiting for visual interface)\n";
		std::cout << "4 - Remove recipe from Database\n";
		
		
		std::cout << "7 - Setup Database from schema.sql\n";
		std::cout << "8 - Fill database using seeds.sql\n";
		
		std::cout << "0 - Back\n";
		std::cout << "$ ";
		std::cin >> playerChoice_EditDatabase;
	
		if (!IsNumeric(playerChoice_EditDatabase))
		{
			std::cerr << "Invalid player choice.\n";
			continue;
		}
		
		switch (stoi(playerChoice_EditDatabase))
		{
			case 1: // Add resource to Database
			{
				while (true)
				{
					std::string resourceName = "";
					
					std::cout << "Currently existing resources:\n";
					database.PrintAllResources(false);
					
					std::cout << "Resource to add (\"0\" to go back): ";
					std::cin >> resourceName;
					
					if (resourceName == "0") break;
					
					bool const success = database.AddResourceToDatabase(resourceName);
					if (!success)
					{
						std::cerr << "Failed to add resource to database.\n";
					}
					else
					{
						std::cout << "Resource added successfully\n";
					}
				}
			} break;
			case 2: // Remove resource from Database
			{
				while (true)
				{
					std::string resourceName = "";
					
					std::cout << "Currently existing resources:\n";
					database.PrintAllResources(true);
					
					std::cout << "Resource to remove (ID or resource name) (\"0\" to go back): ";
					std::cin >> resourceName;
					
					if (resourceName == "0") break;
					
					bool success = false;
					if (IsNumeric(resourceName))
					{
						success = database.RemoveResourceFromDatabase(stoi(resourceName));
					}
					else
					{
						success = database.RemoveResourceFromDatabase(resourceName);
					}
					
					if (!success)
					{
						std::cerr << "Failed to remove resource from database.\n";
					}
					else
					{
						std::cout << "Resource removed successfully\n";
					}
				}
			} break;
			case 3: // Add recipe to Database
			{
				
			} break;
			case 4: // Remove recipe from Database
			{
				while (true)
				{
					std::string recipeName = "";
					
					std::cout << "Currently existing recipes:\n";
					database.PrintAllRecipes(true, true, true);
					
					
					std::cout << "Recipe to remove (ID or recipe name) (\"0\" to go back): ";
					std::cin >> recipeName;
					
					if (recipeName == "0") break;
					
					bool success = false;
					if (IsNumeric(recipeName))
					{
						success = database.RemoveRecipeFromDatabase(stoi(recipeName));
					}
					else
					{
						success = database.RemoveRecipeFromDatabase(recipeName);
					}
					
					if (!success)
					{
						std::cerr << "Failed to remove recipe from database.\n";
					}
				}
			} break;
			case 7: // Setup Database from schema.sql
			{
				// Load and execute schema (CREATE TABLE statements)
				std::string const schemaSQL = ReadFileToString("schema.sql");
				if (!schemaSQL.empty() && !database.ExecuteScript(schemaSQL))
				{
					std::cerr << "Failed to create database schema.\n";
				}
				else
				{
					std::cout << "Schema created successfully\n";
				}
			} break;
			case 8: // Fill database using seeds.sql
			{
				// Load and execute seed data (INSERT statements)
				// Note: The IF NOT EXISTS in schema.sql and IGNORE in inserts means this is "safe" to run multiple times
				std::string const seedSQL = ReadFileToString("seeds.sql");
				if (!seedSQL.empty() && !database.ExecuteScript(seedSQL))
				{
					std::cerr << "Failed to seed database.\n";
				}
				else
				{
					std::cout << "Seed run successfully\n";
				}
			} break;
			case 0: return;
			default:
			{
				std::cerr << "Invalid player choice.\n";
			}
		}
	}				
}

//--------------------------------------------------
static void PrintFromDatabase(FDatabase const & database)
{
	while (true)
	{
		std::string playerChoice_PrintFromDatabase = "";
		std::cout << "---- Print From Database ----\n";
		std::cout << "1 - Print resource all resources\n";
		std::cout << "2 - Print all recipes (without input/output)\n";
		std::cout << "3 - Print all recipes (with input only)\n";
		std::cout << "4 - Print all recipes (with output only)\n";
		std::cout << "5 - Print all recipes (with input and output)\n";
		
		std::cout << "0 - Back\n";
		std::cout << "$ ";
		std::cin >> playerChoice_PrintFromDatabase;
	
		if (!IsNumeric(playerChoice_PrintFromDatabase))
		{
			std::cerr << "Invalid player choice.\n";
			continue;
		}
		
		switch (stoi(playerChoice_PrintFromDatabase))
		{
			case 1: // Print resource all resources
			{
				database.PrintAllResources(false);
			} break;
			case 2: // Print all recipes (without input/output)
			{
				database.PrintAllRecipes(false, false, false);
			} break;
			case 3: // Print all recipes (with input only)
			{
				database.PrintAllRecipes(false, true, false);
			} break;
			case 4: // Print all recipes (with output only)
			{
				database.PrintAllRecipes(false, false, true);
			} break;
			case 5: // Print all recipes (with input and output)
			{
				database.PrintAllRecipes(false, true, true);
			} break;
			case 0: return;
			default:
			{
				std::cerr << "Invalid player choice.\n";
			}
		}
	}				
}

//--------------------------------------------------
static void FindNeededResources(
	FDatabase const & database,
	std::vector<FRecipe> const & recipes,
	std::unordered_map<FResourceID, std::string> const & resourceNames_FromIDs,
	std::unordered_map<std::string, FResourceID> const & resourceIDs_FromNames,
	std::vector<std::pair<FResourceID, int>> const & resourceComplexities_Ordered
)
{
	while (true)
	{
		std::cout << "---------------------------------------\n";
		std::cout << "-------- Find Needed Resources --------\n";
		std::cout << "---------------------------------------\n";
		
		FResource resourceNeeded;
		
		// First step: get the resource name or id
		{
			// Reset the resource
			resourceNeeded = FResource();
			
			std::string playerChoice_NeededResource = "";
			
			std::cout << "All resources available:\n";
			database.PrintAllResources(true);
		
			std::cout << "0 - Back\n";
			std::cout << "ResourceName OR ResourceID: ";
			std::cin >> playerChoice_NeededResource;
			
			if (playerChoice_NeededResource == "0")
				return;
			
			if (IsNumeric(playerChoice_NeededResource))
			{
				resourceNeeded.m_ResourceID = stoi(playerChoice_NeededResource);
				if (!resourceNames_FromIDs.contains(resourceNeeded.m_ResourceID))
				{
					std::cerr << "Invalid resource ID.\n";
					continue;
				}
				resourceNeeded.m_ResourceName = resourceNames_FromIDs.at(resourceNeeded.m_ResourceID);
			}
			else
			{
				if (!resourceIDs_FromNames.contains(playerChoice_NeededResource))
				{
					std::cerr << "Invalid resource Name.\n";
					continue;
				}
				
				resourceNeeded.m_ResourceName = playerChoice_NeededResource;
				resourceNeeded.m_ResourceID = resourceIDs_FromNames.at(resourceNeeded.m_ResourceName);
			}
		}
		
		
		// Second step: get the resource amount
		{
			bool shouldExit = false;
			
			while (resourceNeeded.m_ResourceAmount <= 0.f)
			{
				std::string playerChoice_ResourceAmount = "";
				
				std::cout << "0 - Back\n";
				std::cout << "Quantity of \"" << resourceNeeded.m_ResourceName << "\" (> 0.0f): ";
				
				std::cin >> playerChoice_ResourceAmount;
				
				
				if (playerChoice_ResourceAmount == "0")
				{
					shouldExit = true;
					break;
				}
				
				float resourceAmount = 0.f;
				if (!IsPositiveFloat(playerChoice_ResourceAmount, resourceAmount))
				{
					std::cerr << "Invalid resource quantity, should be positive.\n";
					continue;
				}
				
				resourceNeeded.m_ResourceAmount = resourceAmount;
			}
			
			// Restart the process to the resource name.
			if (shouldExit)
				continue;
		}
		
		
		// Third step: get the all the necessary resources
		{
			std::vector<FResource> const output = GatherAllNecessaryResources(recipes, resourceNeeded);
			PrintAllNeededResources(resourceNeeded, output, resourceComplexities_Ordered, resourceNames_FromIDs);
		}
	}
	
	
}

//--------------------------------------------------
int main()
{
	FDatabase database;
	if (!database.Open("factory_recipes.db"))
	{
		std::cerr << "Failed to open database.\n";
		return 1;
	}
	
	// database.AddRecipeToDatabase(
	// 	"testRecipe",
	// 	{
	// 		FResource(3, 10),
	// 		FResource(4, 20),
	// 		FResource(5, 30)
	// 	},
	// 	{
	// 		FResource(6, 10),
	// 		FResource(3, 10)
	// 	}
	// );
	//
	// database.AddRecipeToDatabase(
	// 	"testRecipe",
	// 	{
	// 		FResource(7, 10),
	// 		FResource(4, 20),
	// 		FResource(5, 30)
	// 	},
	// 	{
	// 		FResource(6, 10)
	// 	}
	// );
	
	
	
	// Load recipes from the database
	std::vector<FRecipe> recipes = database.LoadRecipes();
	std::unordered_map<FResourceID, std::string> resourceNames_FromIDs;
	std::unordered_map<std::string, FResourceID> resourceIDs_FromNames;
	database.LoadResourceNames(resourceNames_FromIDs, resourceIDs_FromNames);
	std::vector<std::pair<FResourceID, int>> resourceComplexities_Ordered = ComputeResourcesComplexity(recipes);
	

	
	// {
	// 	FResource const resourceNeeded(3, 10, "Iron_Plate");
	// 	std::vector<FResource> const output = GatherAllNecessaryResources(recipes, resourceNeeded);
	// 	PrintAllNeededResources(resourceNeeded, output, resourceComplexities_Ordered, resourceNames);
	// }
	// {
	// 	FResource const resourceNeeded(5, 10, "Iron_Screw");
	// 	std::vector<FResource> const output = GatherAllNecessaryResources(recipes, resourceNeeded);
	// 	PrintAllNeededResources(resourceNeeded, output, resourceComplexities_Ordered, resourceNames);
	// }
	// {
	// 	FResource const resourceNeeded(6, 10, "Reinforced_Plate");
	// 	std::vector<FResource> const output = GatherAllNecessaryResources(recipes, resourceNeeded);
	// 	PrintAllNeededResources(resourceNeeded, output, resourceComplexities_Ordered, resourceNames);
	// }
	
	
	while (true)
	{
		std::string playerChoice = "";
		
		std::cout << "What do you want to do?\n";
		std::cout << "1 - Edit Database\n";
		std::cout << "2 - Print from Database\n";
		
		std::cout << "4 - Find needed resources for needed resources\n";
		
		
		
		std::cout << "0 - Quit\n";
		std::cout << "$ ";
		std::cin >> playerChoice;
		
		std::cout << "\033[2J\033[1;1H";
		std::cout.flush();
		
		if (!IsNumeric(playerChoice))
		{
			std::cerr << "Invalid player choice.\n";
			continue;
		}
		
		switch (stoi(playerChoice))
		{
			case 1:
			{
				EditDatabase(database);
			} break;
			case 2:
			{
				PrintFromDatabase(database);
			} break;
			case 4:
			{
				FindNeededResources(
					database,
					recipes,
					resourceNames_FromIDs,
					resourceIDs_FromNames,
					resourceComplexities_Ordered
				);
			} break;
			case 0: return 0;
			default:
			{
				std::cerr << "Invalid player choice.\n";
			}
		}
	}
}