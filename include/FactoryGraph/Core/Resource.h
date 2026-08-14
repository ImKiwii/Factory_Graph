#pragma once
#include <iosfwd>
#include <string>

//--------------------------------------------------
enum class EResourceType : int
{
    Iron_Ore,
    Iron_Ingot,
    Iron_Plate,
    Iron_Bar,
    Iron_Screw,
    Reinforced_Plate,

    COUNT
};
const char * ToString(EResourceType const resourceType);



//--------------------------------------------------
class FResource
{
public:
    explicit FResource(int resourceId, float resourceCount);

    int m_ResourceID = 0;
    float m_ResourceCount = 0.f;
};

//--------------------------------------------------
std::ostream & operator<<(std::ostream & os, FResource const & resource);
//--------------------------------------------------
// This == operator ignore the resource count.
bool operator==(FResource const & resourceA, FResource const & resourceB);

//--------------------------------------------------
bool operator>(FResource const & resource, float const resourceCount);
//--------------------------------------------------
bool operator>=(FResource const & resource, float const resourceCount);

//--------------------------------------------------
bool operator<(FResource const & resource, float const resourceCount);
//--------------------------------------------------
bool operator<=(FResource const & resource, float const resourceCount);