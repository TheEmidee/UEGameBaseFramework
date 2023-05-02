#include "Engine/GBFPlatformInfosSubsystem.h"

#include <CommonUISettings.h>
#include <ICommonUIModule.h>

void UGBFPlatformInfosSubsystem::Initialize( FSubsystemCollectionBase & collection )
{
    Super::Initialize( collection );

    PlatformTraits = ICommonUIModule::GetSettings().GetPlatformTraits();
}

void UGBFPlatformInfosSubsystem::AddPlatformTraits( const FGameplayTagContainer & tags )
{
    PlatformTraits.AppendTags( tags );
}

void UGBFPlatformInfosSubsystem::RemovePlatformTraits( const FGameplayTagContainer & tags )
{
    PlatformTraits.RemoveTags( tags );
}
