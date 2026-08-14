#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

struct FResource;
typedef int FResourceID;

//--------------------------------------------------
struct FRecipe
{
	void AddResourceToRecipe_Input(FResource const & resource);
	void AddResourceToRecipe_Output(FResource const & resource);

	std::string m_RecipeName = "";
	std::vector<FResource> m_Resources_Input;
	std::vector<FResource> m_Resources_Output;
};

//--------------------------------------------------
void FindCorrespondingRecipe_FromOutputResource(
	std::vector<FRecipe> const & recipes,
	FResource const & resourcesNeeded,
	FRecipe const * & outputRecipe
);

//--------------------------------------------------
std::ostream & operator<<(std::ostream & os, FRecipe const & recipe);



//--------------------------------------------------
void PrintAllNeededResources(
	FResource const & resourceNeeded,
	std::vector<FResource> const & allResourcesNeeded,
	std::vector<std::pair<FResourceID, int>> const & resourceComplexities,
	std::unordered_map<FResourceID, std::string> const & resourceNames
);

//--------------------------------------------------
std::vector<FResource> GatherAllNecessaryResources(
	std::vector<FRecipe> const & recipes,
	FResource const & resourcesNeeded
);

//--------------------------------------------------
void ComputeResourcesComplexity(
	std::map<FResourceID, int> & resultComplexity,
	FResourceID currentResourceID,
	std::map<FResourceID, std::vector<std::vector<FResourceID>>> const & allCraftingPossibilities
);

//--------------------------------------------------
std::vector<std::pair<FResourceID, int>> ComputeResourcesComplexity(std::vector<FRecipe> const & recipes);