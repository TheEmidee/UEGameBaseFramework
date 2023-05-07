#include "UI/SubSystems/GBFGameUIManagerSubsystem.h"

#include "CommonLocalPlayer.h"
#include "GameUIPolicy.h"
#include "PrimaryGameLayout.h"

#include <Components/SlateWrapperTypes.h>
#include <Engine/LocalPlayer.h>
#include <GameFramework/HUD.h>
#include <GameFramework/PlayerController.h>

void UGBFGameUIManagerSubsystem::Initialize( FSubsystemCollectionBase & collection )
{
    Super::Initialize( collection );

    TickHandle = FTSTicker::GetCoreTicker().AddTicker( FTickerDelegate::CreateUObject( this, &ThisClass::Tick ), 0.0f );
}

void UGBFGameUIManagerSubsystem::Deinitialize()
{
    Super::Deinitialize();

    FTSTicker::GetCoreTicker().RemoveTicker( TickHandle );
}

bool UGBFGameUIManagerSubsystem::Tick( float /*delta_time*/ )
{
    SyncRootLayoutVisibilityToShowHUD();

    return true;
}

void UGBFGameUIManagerSubsystem::SyncRootLayoutVisibilityToShowHUD()
{
    if ( const auto * policy = GetCurrentUIPolicy() )
    {
        for ( const auto * local_player : GetGameInstance()->GetLocalPlayers() )
        {
            bool should_show_ui = true;

            if ( const auto * pc = local_player->GetPlayerController( GetWorld() ) )
            {
                if ( const auto * hud = pc->GetHUD();
                     hud != nullptr && !hud->bShowHUD )
                {
                    should_show_ui = false;
                }
            }

            if ( auto * root_layout = policy->GetRootLayout( CastChecked< UCommonLocalPlayer >( local_player ) ) )
            {
                if ( const auto desired_visibility = should_show_ui ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
                     desired_visibility != root_layout->GetVisibility() )
                {
                    root_layout->SetVisibility( desired_visibility );
                }
            }
        }
    }
}
