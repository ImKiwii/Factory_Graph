#include <FactoryGraph/Buildings.h>

//--------------------------------------------------
const char * ToString(EBuildingType const buildingType)
{
	switch (buildingType)
	{
		case EBuildingType::Minor: return "Minor";
		case EBuildingType::Foundry: return "Foundry";
		case EBuildingType::Constructor: return "Constructor";
		case EBuildingType::Assembler: return "Assembler";
	}
	
	static_assert(static_cast<int>(EBuildingType::COUNT) == 4);
	return "Unknown";
}

//--------------------------------------------------
FBuilding::FBuilding(EBuildingType const buildingType)
{
	m_BuildingType = buildingType;
}

