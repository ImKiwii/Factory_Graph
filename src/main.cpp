
#include <assert.h>
#include <iostream>
#include <queue>
#include <map>

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
	std::vector<FResource> allResourcesNeeded,
	std::vector<std::pair<EResourceType, int>> const & resourceComplexities
)
{
	std::cout << "All resources needed for: " << resourceNeeded << std::endl;
	for (auto const& [resource, complexity] : resourceComplexities)
	{
		float neededResourcesCount = 0.f;
		std::vector<int> resourceIndexToRemove;
		for (int iResource = 0; iResource < allResourcesNeeded.size(); ++iResource)
		{
			if (allResourcesNeeded[iResource].m_ResourceType != resource)
				continue;
			
			neededResourcesCount += allResourcesNeeded[iResource].m_ResourceCount;
			resourceIndexToRemove.push_back(iResource);
		}
		
		if (neededResourcesCount == 0.f)
		{
			continue;
		}
		
		for (int iToRemove = resourceIndexToRemove.size() -1; iToRemove >= 0; --iToRemove)
		{
			allResourcesNeeded.erase(allResourcesNeeded.begin() + resourceIndexToRemove[iToRemove]);
		}
		
		std::cout << FResource(resource, neededResourcesCount) << std::endl;
		
		if (allResourcesNeeded.empty())
		{
			break;
		}
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
	
	int minComplexity = 0;
	std::vector<std::vector<EResourceType>> const & inputPossible = allCraftingPossibilities.at(resourceType);
	
	for (std::vector<EResourceType> const & in : inputPossible)
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
		if (complexity < minComplexity || minComplexity == 0)
		{
			minComplexity = complexity;
		}
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

	std::sort(resourcesOrdered.begin(), resourcesOrdered.end(),
		[](auto const& a, auto const& b)
		{
			return a.second > b.second; // Highest complexity first
		});
	
	return resourcesOrdered;
}

//--------------------------------------------------
int main()
{
	std::vector<FRecipe> recipes;
	SetupRecipes(recipes);

	std::vector<std::pair<EResourceType, int>> resourceComplexities_Ordered = ComputeResourcesComplexity(recipes);

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