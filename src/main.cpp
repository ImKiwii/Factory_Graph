
#include <assert.h>
#include <iostream>
#include <queue>

#include "FactoryGraph/Recipe.h"
#include "FactoryGraph/Resource.h"

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
	
	FRecipe recipe_IronScrew;
	recipe_IronScrew.AddResourceToRecipe_Input(EResourceType::Iron_Bar, 1.f);
	recipe_IronScrew.AddResourceToRecipe_Output(EResourceType::Iron_Screw, 4.f);
	outRecipes.push_back(recipe_IronScrew);
	
	FRecipe recipe_ReinforcedPlate;
	recipe_ReinforcedPlate.AddResourceToRecipe_Input(EResourceType::Iron_Plate, 6.f);
	recipe_ReinforcedPlate.AddResourceToRecipe_Input(EResourceType::Iron_Screw, 12.f);
	recipe_ReinforcedPlate.AddResourceToRecipe_Output(EResourceType::Reinforced_Plate, 1.f);
	outRecipes.push_back(recipe_ReinforcedPlate);
}


//--------------------------------------------------
static void PrintAllNeededResources(
	FResource const & resourceNeeded,
	std::vector<FResource> const & allResourcesNeeded
)
{
	std::cout << "All resources needed for: " << resourceNeeded << "\n";
	
	for (FResource const & resource : allResourcesNeeded)
	{
		std::cout << resource << "\n";
	}
}




//--------------------------------------------------
std::vector<FResource> GatherAllNecessaryResources(
	std::vector<FRecipe> const & recipes,
	FResource const & resourcesNeeded
)
{
	std::queue<FResource> resourcesNeeded_Queue;
	resourcesNeeded_Queue.push(resourcesNeeded);
	
	std::vector<FResource> output;

	while (!resourcesNeeded_Queue.empty())
	{
		FResource const & currentResource = resourcesNeeded_Queue.front();
		FRecipe const & correspondingRecipe = FindCorrespondingRecipe_FromOutputResource(recipes, currentResource);
		
		float recipeCount = 0.f;
		for (FResource const & resource : correspondingRecipe.m_Resources_Output)
		{
			if (resource.m_ResourceType != currentResource.m_ResourceType)
				continue;
			
			recipeCount = currentResource.m_ResourceCount / resource.m_ResourceCount;
		}
		
		
		for (FResource const & resource : correspondingRecipe.m_Resources_Input)
		{
			FResource neededResource(resource.m_ResourceType, resource.m_ResourceCount * recipeCount);
			
			resourcesNeeded_Queue.push(neededResource);
			output.push_back(neededResource);
		}		
		
		resourcesNeeded_Queue.pop();
	}
	
	return output;
}



//--------------------------------------------------
int main()
{
	std::vector<FRecipe> recipes;
	SetupRecipes(recipes);

	{
		FResource const resourceNeeded(EResourceType::Iron_Plate, 10);
		std::vector<FResource> const output = GatherAllNecessaryResources(recipes, resourceNeeded);
		PrintAllNeededResources(resourceNeeded, output);
	}
	{
		FResource const resourceNeeded(EResourceType::Iron_Screw, 10);
		std::vector<FResource> const output = GatherAllNecessaryResources(recipes, resourceNeeded);
		PrintAllNeededResources(resourceNeeded, output);
	}
	{
		FResource const resourceNeeded(EResourceType::Reinforced_Plate, 10);
		std::vector<FResource> const output = GatherAllNecessaryResources(recipes, resourceNeeded);
		PrintAllNeededResources(resourceNeeded, output);
	}
	
	return 0;
}