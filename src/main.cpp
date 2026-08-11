
#include <assert.h>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>

#include "FactoryGraph/Core/Recipe.h"
#include "FactoryGraph/Core/Resource.h"
#include "FactoryGraph/Database/Database.h"

//--------------------------------------------------
static void SetupRecipes(std::vector<FRecipe> & outRecipes)
{
	assert(outRecipes.empty());
	outRecipes.reserve(6);
	
	FRecipe recipe_IronOre;
	recipe_IronOre.AddResourceToRecipe_Output(EResourceType::Iron_Ore, 1.f);
	outRecipes.push_back(recipe_IronOre);
	
	FRecipe recipe_IronIngot;
	recipe_IronIngot.AddResourceToRecipe_Input(EResourceType::Iron_Ore, 1.f);
	recipe_IronIngot.AddResourceToRecipe_Output(EResourceType::Iron_Ingot, 1.f);
	outRecipes.push_back(recipe_IronIngot);
	
	FRecipe recipe_IronPlate;
	recipe_IronPlate.AddResourceToRecipe_Input(EResourceType::Iron_Ingot, 3.f);
	recipe_IronPlate.AddResourceToRecipe_Output(EResourceType::Iron_Plate, 2.f);
	outRecipes.push_back(recipe_IronPlate);
	
	FRecipe recipe_IronBar;
	recipe_IronBar.AddResourceToRecipe_Input(EResourceType::Iron_Ingot, 1.f);
	recipe_IronBar.AddResourceToRecipe_Output(EResourceType::Iron_Bar, 1.f);
	outRecipes.push_back(recipe_IronBar);
	
	//FRecipe recipe_IronScrew;
	//recipe_IronScrew.AddResourceToRecipe_Input(EResourceType::Iron_Bar, 1.f);
	//recipe_IronScrew.AddResourceToRecipe_Output(EResourceType::Iron_Screw, 4.f);
	//outRecipes.push_back(recipe_IronScrew);
	
	FRecipe recipe_IronScrew_FromIngot;
	recipe_IronScrew_FromIngot.AddResourceToRecipe_Input(EResourceType::Iron_Ingot, 5.f);
	recipe_IronScrew_FromIngot.AddResourceToRecipe_Output(EResourceType::Iron_Screw, 20.f);
	outRecipes.push_back(recipe_IronScrew_FromIngot);
	
	FRecipe recipe_ReinforcedPlate;
	recipe_ReinforcedPlate.AddResourceToRecipe_Input(EResourceType::Iron_Plate, 6.f);
	recipe_ReinforcedPlate.AddResourceToRecipe_Input(EResourceType::Iron_Screw, 12.f);
	recipe_ReinforcedPlate.AddResourceToRecipe_Output(EResourceType::Reinforced_Plate, 1.f);
	outRecipes.push_back(recipe_ReinforcedPlate);
}


//--------------------------------------------------
static void PrintAllNeededResources(
	FResource const & resourceNeeded,
	std::vector<FResource> const & allResourcesNeeded,
	std::vector<std::pair<EResourceType, int>> const & resourceComplexities
)
{
	std::cout << "All resources needed for: " << resourceNeeded << std::endl;
	for (auto const& [resource, complexity] : resourceComplexities)
	{
		float neededResourcesCount = 0.f;
		for (int iResource = 0; iResource < allResourcesNeeded.size(); ++iResource)
		{
			if (allResourcesNeeded[iResource].m_ResourceType != resource)
				continue;
			
			neededResourcesCount += allResourcesNeeded[iResource].m_ResourceCount;
		}
		
		if (neededResourcesCount == 0.f)
			continue;
		
		std::cout << FResource(resource, neededResourcesCount) << std::endl;
	}
}




//--------------------------------------------------
static std::vector<FResource> GatherAllNecessaryResources(
	std::vector<FRecipe> const & recipes,
	FResource const & resourcesNeeded
)
{
	std::queue<FResource> resourcesNeeded_Queue;
	resourcesNeeded_Queue.push(resourcesNeeded);
	
	std::vector<FResource> output;

	// Warning, infinite loop possibility depending on the recipes.
	while (!resourcesNeeded_Queue.empty())
	{
		FResource const & currentResource = resourcesNeeded_Queue.front();
		FRecipe const * pCorrespondingRecipe = nullptr;
		FindCorrespondingRecipe_FromOutputResource(recipes, currentResource, pCorrespondingRecipe);
		
		if (pCorrespondingRecipe)
		{
			float recipeCount = 0.f;
			for (FResource const & resource : pCorrespondingRecipe->m_Resources_Output)
			{
				if (resource != currentResource)
					continue;
				
				if (resource <= 0.f)
					continue;
				
				recipeCount = currentResource.m_ResourceCount / resource.m_ResourceCount;
				break;
			}
			
			
			for (FResource const & resource : pCorrespondingRecipe->m_Resources_Input)
			{
				FResource neededResource(resource.m_ResourceType, resource.m_ResourceCount * recipeCount);
				
				resourcesNeeded_Queue.push(neededResource);
				output.push_back(neededResource);
			}		
		}
		
		resourcesNeeded_Queue.pop();
	}
	
	return output;
}

//--------------------------------------------------
static void ComputeResourcesComplexity(
	std::map<EResourceType, int> & resultComplexity,
	EResourceType resourceType,
	std::map<EResourceType, std::vector<std::vector<EResourceType>>> const & allCraftingPossibilities
)
{
	if (resultComplexity.contains(resourceType))
	{
		return;
	}
	
	assert(allCraftingPossibilities.contains(resourceType));
	
	int minComplexity = std::numeric_limits<int>::max();
	for (std::vector<EResourceType> const & in : allCraftingPossibilities.at(resourceType))
	{
		int complexity = 0;
		for (EResourceType const & inputResource : in)
		{
			if (!resultComplexity.contains(inputResource))
			{
				ComputeResourcesComplexity(resultComplexity, inputResource, allCraftingPossibilities);
			}
			assert(resultComplexity.contains(inputResource));
			complexity += resultComplexity[inputResource];
		}
		minComplexity = std::min(minComplexity, complexity);
	}
	
	resultComplexity.insert({resourceType, minComplexity + 1});
}

//--------------------------------------------------
static std::vector<std::pair<EResourceType, int>> ComputeResourcesComplexity(std::vector<FRecipe> const & recipes)
{
	std::map<EResourceType, int> resultComplexity;
	std::map<EResourceType, std::vector<std::vector<EResourceType>>> allCraftingPossibilities;
	
	for (FRecipe const & recipe : recipes)
	{
		std::vector<EResourceType> input;
		input.reserve(recipe.m_Resources_Input.size());
		for (FResource const & inputResource : recipe.m_Resources_Input)
		{
			input.push_back(inputResource.m_ResourceType);
		}
		for (FResource const & outputResource : recipe.m_Resources_Output)
		{
			allCraftingPossibilities[outputResource.m_ResourceType].push_back(input);
		}
	}
	
	for (const auto & [resourceType, _] : allCraftingPossibilities)
	{
		ComputeResourcesComplexity(resultComplexity, resourceType, allCraftingPossibilities);		
	}
	
	std::vector<std::pair<EResourceType, int>> resourcesOrdered(
	resultComplexity.begin(),
	resultComplexity.end());

	std::ranges::sort(resourcesOrdered,
					[](auto const & a, auto const & b)
					{
						return a.second > b.second; // Highest complexity first
					});
	
	return resourcesOrdered;
}


//--------------------------------------------------
// Helper function to read a text file (for loading SQL scripts)
static std::string ReadFileToString(const std::string& filePath)
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
static void EditDatabase(FDatabase const & database)
{
	while (true)
	{
		int playerChoice_EditDatabase = 0;
		std::cout << "---- Edit Database ----\n";
		std::cout << "1 - Add resource to Database\n";
		std::cout << "2 - Remove resource from Database\n";
		std::cout << "3 - Add recipe to Database\n";
		std::cout << "4 - Remove recipe from Database\n";
		
		std::cout << "0 - Back\n";
		std::cout << "$ ";
		std::cin >> playerChoice_EditDatabase;
	
		switch (playerChoice_EditDatabase)
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
			case 0: return;
		}
	}				
}

//--------------------------------------------------
static void PrintFromDatabase(FDatabase const & database)
{
	while (true)
	{
		int playerChoice_PrintFromDatabase = 0;
		std::cout << "---- Print From Database ----\n";
		std::cout << "1 - Print resource all resources\n";
		std::cout << "2 - Print all recipes (without input/output)\n";
		std::cout << "3 - Print all recipes (with input only)\n";
		std::cout << "4 - Print all recipes (with output only)\n";
		std::cout << "5 - Print all recipes (with input and output)\n";
		
		std::cout << "0 - Back\n";
		std::cout << "$ ";
		std::cin >> playerChoice_PrintFromDatabase;
	
		switch (playerChoice_PrintFromDatabase)
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
	
	while (true)
	{
		int playerChoice = 0;
		
		std::cout << "What do you want to do?\n";
		std::cout << "1 - Edit Database\n";
		std::cout << "2 - Print from Database\n";
		
		std::cout << "4 - Find needed resources for needed resources\n";
		
		
		
		std::cout << "0 - Quit\n";
		std::cout << "$ ";
		std::cin >> playerChoice;
		
		switch (playerChoice)
		{
			case 1:
			{
				EditDatabase(database);
			} break;
			case 2:
			{
				PrintFromDatabase(database);
			} break;
			case 0: return 0;
		}
	}
	
	
	
	
	
	
	// Load and execute schema (CREATE TABLE statements)
	std::string const schemaSQL = ReadFileToString("schema.sql");
	if (!schemaSQL.empty() && !database.ExecuteScript(schemaSQL))
	{
		std::cerr << "Failed to create database schema.\n";
		return 1;
	}
	
	// Load and execute seed data (INSERT statements)
	// Note: The IF NOT EXISTS in schema.sql and IGNORE in inserts means this is safe to run multiple times
	std::string const seedSQL = ReadFileToString("seeds.sql");
	if (!seedSQL.empty() && !database.ExecuteScript(seedSQL))
	{
		std::cerr << "Failed to seed database.\n";
		// Don't return here - the database might already have data from a previous run
	}
	
	
	// Load recipes from the database
	std::vector<FRecipe> const recipes = database.LoadRecipes();

	std::vector<std::pair<EResourceType, int>> const resourceComplexities_Ordered = ComputeResourcesComplexity(recipes);

	{
		FResource const resourceNeeded(EResourceType::Iron_Plate, 10);
		std::vector<FResource> const output = GatherAllNecessaryResources(recipes, resourceNeeded);
		PrintAllNeededResources(resourceNeeded, output, resourceComplexities_Ordered);
	}
	{
		FResource const resourceNeeded(EResourceType::Iron_Screw, 10);
		std::vector<FResource> const output = GatherAllNecessaryResources(recipes, resourceNeeded);
		PrintAllNeededResources(resourceNeeded, output, resourceComplexities_Ordered);
	}
	{
		FResource const resourceNeeded(EResourceType::Reinforced_Plate, 10);
		std::vector<FResource> const output = GatherAllNecessaryResources(recipes, resourceNeeded);
		PrintAllNeededResources(resourceNeeded, output, resourceComplexities_Ordered);
	}
	
	return 0;
}