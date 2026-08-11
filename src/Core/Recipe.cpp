#include "FactoryGraph/Core/Recipe.h"
#include "FactoryGraph/Core/Resource.h"

#include <assert.h>
#include <iostream>

//--------------------------------------------------
void FRecipe::AddResourceToRecipe_Input(EResourceType const resourceType, float const resourceAmount)
{
	m_Resources_Input.push_back(FResource(resourceType, resourceAmount));
}

//--------------------------------------------------
void FRecipe::AddResourceToRecipe_Output(EResourceType const resourceType, float const resourceAmount)
{
	m_Resources_Output.push_back(FResource(resourceType, resourceAmount));
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
	
	std::cerr << "There is no available recipe for this resource: " << ToString(resourcesNeeded.m_ResourceType) << "/n";
	assert(false);
}