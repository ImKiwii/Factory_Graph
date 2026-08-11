#pragma once

#include <vector>
#include "FactoryGraph/Core/Resource.h"

//--------------------------------------------------
enum class EBuildingType : int
{
    Minor,
    Foundry,
    Constructor,
    Assembler,
    COUNT
};
inline const char * ToString(EBuildingType const buildingType);



//--------------------------------------------------
class FBuilding
{
public:
    explicit FBuilding(EBuildingType const buildingType);

    EBuildingType m_BuildingType = EBuildingType::COUNT;
    
    std::vector<FResource> m_Resources_Input;
    std::vector<FResource> m_Resources_Output;
    
    float m_WorkDuration = 0.0f;
};
