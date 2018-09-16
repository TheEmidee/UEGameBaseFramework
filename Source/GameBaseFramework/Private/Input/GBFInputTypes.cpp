#include "GBFInputTypes.h"

#if WITH_EDITOR

void UGBFPlatformInputTextures::PostEditChangeProperty( FPropertyChangedEvent & property_change_event )
{
    if ( property_change_event.Property != nullptr )
    {
        PlatformInputTexturesChangedDelegate.Broadcast( property_change_event.Property->GetName(), this );
    }
}

UGBFPlatformInputTextures::FOnPlatformInputTexturesChanged & UGBFPlatformInputTextures::OnPlatformInputTexturesChanged()
{
    return PlatformInputTexturesChangedDelegate;
}

UGBFPlatformInputTextures::FOnPlatformInputTexturesChanged UGBFPlatformInputTextures::PlatformInputTexturesChangedDelegate;
#endif
