#include "FactoryGraph/Core/Resource.h"

#include <iostream>

//--------------------------------------------------
std::ostream & operator<<(std::ostream & os, FResource const & resource)
{
	os << resource.m_ResourceAmount << " - " << resource.m_ResourceName << "\n";
	return os;
}

//--------------------------------------------------
bool operator==(FResource const & resourceA, FResource const & resourceB)
{
	return resourceA.m_ResourceID == resourceB.m_ResourceID;
}

//--------------------------------------------------
bool operator>(FResource const & resource, float const resourceCount)
{
	return resource.m_ResourceAmount > resourceCount;
}

//--------------------------------------------------
bool operator>=(FResource const & resource, float const resourceCount)
{
	return resource.m_ResourceAmount >= resourceCount;
}

//--------------------------------------------------
bool operator<(FResource const & resource, float const resourceCount)
{
	return !(resource >= resourceCount);
}

//--------------------------------------------------
bool operator<=(FResource const & resource, float const resourceCount)
{
	return !(resource > resourceCount);
}

