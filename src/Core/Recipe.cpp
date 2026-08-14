#include "FactoryGraph/Core/Recipe.h"
#include "FactoryGraph/Core/Resource.h"

#include <assert.h>
#include <iostream>
#include <queue>

//--------------------------------------------------
void FRecipe::AddResourceToRecipe_Input(FResource const & resource)
{
	m_Resources_Input.push_back(resource);
}

//--------------------------------------------------
void FRecipe::AddResourceToRecipe_Output(FResource const & resource)
{
	m_Resources_Output.push_back(resource);
}

//--------------------------------------------------
void FindCorrespondingRecipe_FromOutputResource(
	std::vector<FRecipe> const & recipes,
	FResource const & resourcesNeeded,
	FRecipe const * & outputRecipe
)
{
	assert(!recipes.empty());
	for (FRecipe const & recipe : recipes)
	{
		for (FResource const & resource : recipe.m_Resources_Output)
		{
			if (resource != resourcesNeeded)
				continue;
			
			outputRecipe = &recipe;
			return;
		}
	}
	
	std::cerr << "There is no available recipe for this resource: " << resourcesNeeded.m_ResourceName << "/n";
	assert(false);
}

//--------------------------------------------------
std::ostream & operator<<(std::ostream & os, FRecipe const & recipe)
{
	os << recipe.m_RecipeName << ":\n";
	
	int inputSize = recipe.m_Resources_Input.size();
	int outputSize = recipe.m_Resources_Output.size();
	for (int iResource = 0; iResource < std::max(inputSize, outputSize); iResource++)
	{
		if (iResource < inputSize)
		{
			os << "\t " << recipe.m_Resources_Input[iResource];
		}
		else
		{
			os << "\t\t\t";
		}
			
		if (iResource == 0)
		{
			os << "\t\t => \t";
		}
		else
		{
			os << "\t\t\t\t";
		}
			
		if (iResource < outputSize)
		{
			os << recipe.m_Resources_Output[iResource];
		}
		os << "\n";
	}
	return os;
}



//--------------------------------------------------
void PrintAllNeededResources(
	FResource const & resourceNeeded,
	std::vector<FResource> const & allResourcesNeeded,
	std::vector<std::pair<FResourceID, int>> const & resourceComplexities,
	std::unordered_map<FResourceID, std::string> const & resourceNames
)
{
	std::cout << "All resources needed for: " << resourceNeeded << "\n";
	for (auto const& [resourceID, complexity] : resourceComplexities)
	{
		float neededResourcesCount = 0.f;
		for (int iResource = 0; iResource < allResourcesNeeded.size(); ++iResource)
		{
			if (allResourcesNeeded[iResource].m_ResourceID != resourceID)
				continue;
			
			neededResourcesCount += allResourcesNeeded[iResource].m_ResourceAmount;
		}
		
		if (neededResourcesCount == 0.f)
			continue;
		
		std::cout << FResource(resourceID, neededResourcesCount, resourceNames.at(resourceID)) << std::endl;
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

	// Warning, infinite loop possibility depending on the recipes.
	while (!resourcesNeeded_Queue.empty())
	{
		FResource const & currentResource = resourcesNeeded_Queue.front();
		FRecipe const * pCorrespondingRecipe = nullptr;
		FindCorrespondingRecipe_FromOutputResource(recipes, currentResource, pCorrespondingRecipe);
		
		if (!pCorrespondingRecipe)
		{
			resourcesNeeded_Queue.pop();
			continue;
		}

		float recipeCount = 0.f;
		for (FResource const & resource : pCorrespondingRecipe->m_Resources_Output)
		{
			if (resource != currentResource)
				continue;
			
			if (resource <= 0.f)
				continue;
			
			recipeCount = currentResource.m_ResourceAmount / resource.m_ResourceAmount;
			break;
		}
		
		
		for (FResource neededResource : pCorrespondingRecipe->m_Resources_Input)
		{
			neededResource.m_ResourceAmount = neededResource.m_ResourceAmount * recipeCount;
			
			resourcesNeeded_Queue.push(neededResource);
			output.push_back(neededResource);
		}
		
		resourcesNeeded_Queue.pop();
	}
	
	return output;
}

//--------------------------------------------------
static void ComputeResourcesComplexity(
	std::map<FResourceID, int> & resultComplexity,
	FResourceID currentResourceID,
	std::map<FResourceID, std::vector<std::vector<FResourceID>>> const & allCraftingPossibilities
)
{
	if (resultComplexity.contains(currentResourceID))
	{
		return;
	}
	
	assert(allCraftingPossibilities.contains(currentResourceID));
	
	int minComplexity = std::numeric_limits<int>::max();
	for (std::vector<FResourceID> const & in : allCraftingPossibilities.at(currentResourceID))
	{
		int complexity = 0;
		for (FResourceID const & inputResource : in)
		{
			if (!resultComplexity.contains(inputResource))
			{
				if (inputResource == currentResourceID)
				{
					// skip this recipe, this will lead into an infinite 
					// We don't want to craft a resource from itself resource.
					complexity = std::numeric_limits<int>::max();
					break;
				}

				ComputeResourcesComplexity(resultComplexity, inputResource, allCraftingPossibilities);
			}
			assert(resultComplexity.contains(inputResource));
			complexity += resultComplexity[inputResource];
			if (complexity >= minComplexity)
				break; // early return if the complexity is higher than the min complexity
		}
		minComplexity = std::min(minComplexity, complexity);
	}
	
	resultComplexity.insert({currentResourceID, minComplexity + 1});
}

//--------------------------------------------------
std::vector<std::pair<FResourceID, int>> ComputeResourcesComplexity(std::vector<FRecipe> const & recipes)
{
	std::map<FResourceID, int> resultComplexity;
	std::map<FResourceID, std::vector<std::vector<FResourceID>>> allCraftingPossibilities;
	
	for (FRecipe const & recipe : recipes)
	{
		std::vector<FResourceID> input;
		input.reserve(recipe.m_Resources_Input.size());
		for (FResource const & inputResource : recipe.m_Resources_Input)
		{
			input.push_back(inputResource.m_ResourceID);
		}
		for (FResource const & outputResource : recipe.m_Resources_Output)
		{
			allCraftingPossibilities[outputResource.m_ResourceID].push_back(input);
		}
	}
	
	for (const auto & [resourceID, _] : allCraftingPossibilities)
	{
		ComputeResourcesComplexity(resultComplexity, resourceID, allCraftingPossibilities);		
	}
	
	std::vector<std::pair<FResourceID, int>> resourcesOrdered(
	resultComplexity.begin(),
	resultComplexity.end());

	std::ranges::sort(resourcesOrdered,
					[](auto const & a, auto const & b)
					{
						return a.second > b.second; // Highest complexity first
					});
	
	return resourcesOrdered;
}
