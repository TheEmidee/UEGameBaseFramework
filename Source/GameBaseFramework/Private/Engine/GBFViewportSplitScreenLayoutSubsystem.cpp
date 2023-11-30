#include "Engine/GBFViewportSplitScreenLayoutSubsystem.h"

#include "Engine/GBFGameViewportClient.h"

#include <Engine/World.h>

void UGBFViewportSplitScreenLayoutSubsystem::ApplySplitScreenOffset( FGBFViewPortPlayerOffset offset )
{
    if ( const auto * world = GetWorld() )
    {
        if ( auto * viewport = Cast< UGBFGameViewportClient >( world->GetGameViewport() ) )
        {
            viewport->SetSplitScreenLayoutOffset( offset );
        }
    }
}
