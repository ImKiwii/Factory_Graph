#pragma once
#include <iosfwd>
#include <string>

typedef int FResourceID;

//--------------------------------------------------
struct FResource
{
    FResourceID m_ResourceID = 0;
    float m_ResourceAmount = 0.f;
    std::string m_ResourceName = "";
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