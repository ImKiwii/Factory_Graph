#pragma once
#include <iosfwd>

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
inline const char * ToString(EResourceType const resourceType);



//--------------------------------------------------
class FResource
{
public:
    explicit FResource(EResourceType const resourceType, float const resourceCount);

    EResourceType m_ResourceType = EResourceType::COUNT;
    float m_ResourceCount = 0.f;
};

//--------------------------------------------------
std::ostream & operator<<(std::ostream & os, FResource const & resource);