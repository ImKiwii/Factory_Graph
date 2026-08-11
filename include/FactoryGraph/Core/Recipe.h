#pragma once

#include <vector>

enum class EResourceType : int;
struct FResource;

//--------------------------------------------------
struct FRecipe
{
	void AddResourceToRecipe_Input(EResourceType const resourceType, float const resourceAmount);
	void AddResourceToRecipe_Output(EResourceType const resourceType, float const resourceAmount);

	std::vector<FResource> m_Resources_Input;
	std::vector<FResource> m_Resources_Output;
};

//--------------------------------------------------
void FindCorrespondingRecipe_FromOutputResource(
	std::vector<FRecipe> const & recipes,
	FResource const & resourcesNeeded,
	FRecipe const * & outputRecipe
);