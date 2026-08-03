#include "FactoryGraph/Recipe.h"
#include "FactoryGraph/Resource.h"

#include <assert.h>
#include <iostream>

//--------------------------------------------------
void FRecipe::AddResourceToRecipe_Input(EResourceType const resourceType, int const resourceAmount)
{
	m_Resources_Input.push_back(FResource(resourceType, resourceAmount));
}

//--------------------------------------------------
void FRecipe::AddResourceToRecipe_Output(EResourceType const resourceType, int const resourceAmount)
{
	m_Resources_Output.push_back(FResource(resourceType, resourceAmount));
}

//--------------------------------------------------
FRecipe const & FindCorrespondingRecipe_FromOutputResource(
	std::vector<FRecipe> const & recipes,
	FResource const & resourcesNeeded
)
{
	assert(!recipes.empty());
	for (FRecipe const & recipe : recipes)
	{
		for (FResource const & resource : recipe.m_Resources_Output)
		{
			if (resource.m_ResourceType != resourcesNeeded.m_ResourceType)
				continue;
			
			return recipe;
		}
	}
	
	std::cerr << "There is no available recipe for this resource: " << ToString(resourcesNeeded.m_ResourceType) << "/n";
	assert(false);
	return recipes[0];
}