#include "Engine/SubSystems/GBFGameInstanceSubsystemBase.h"
#include "Engine/GBFGameInstance.h"

UGBFGameInstance * UGBFGameInstanceSubsystemBase::GetGBFGameInstance() const
{
    return Cast< UGBFGameInstance >( GetOuterUGameInstance() );
}
