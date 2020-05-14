#include "Engine/SubSystems/GBFGameInstanceSubsystemBase.h"

UGBFGameInstance * UGBFGameInstanceSubsystemBase::GetGBFGameInstance() const
{
    return Cast< UGBFGameInstance >( GetOuterUGameInstance() );
}
