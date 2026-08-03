#include <FactoryGraph/Resource.h>

#include <iostream>

const char * ToString(EResourceType const resourceType)
{
	switch (resourceType)
	{
		case EResourceType::Iron_Ore: return "Iron_Ore";
		case EResourceType::Iron_Ingot: return "Iron_Ingot";
		case EResourceType::Iron_Plate: return "Iron_Plate";
		case EResourceType::Iron_Bar: return "Iron_Bar";
		case EResourceType::Iron_Screw: return "Iron_Screw";
		case EResourceType::Reinforced_Plate: return "Reinforced_Plate";
	}
	
	static_assert(static_cast<int>(EResourceType::COUNT) == 6);
	return "Unknown";
}

//--------------------------------------------------
FResource::FResource(EResourceType const resourceType, float const resourceCount)
{
    m_ResourceType = resourceType;
	m_ResourceCount = resourceCount;
}

//--------------------------------------------------
std::ostream & operator<<(std::ostream & os, FResource const & resource)
{
	os << resource.m_ResourceCount << " - " << ToString(resource.m_ResourceType) << "\n";
	return os;
}

