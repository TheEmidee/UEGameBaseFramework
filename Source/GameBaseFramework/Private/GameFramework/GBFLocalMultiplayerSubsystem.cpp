#include "GameFramework/GBFLocalMultiplayerSubsystem.h"

#include "Engine/GBFGameViewportClient.h"

#include <Engine/LocalPlayer.h>
#include <Engine/World.h>
#include <EnhancedInputSubsystems.h>
#include <InputMappingContext.h>

UGBFLocalMultiplayerSubsystem::UGBFLocalMultiplayerSubsystem() :
    LerpToTargetOffsetRemainingTime( 0.0f ),
    LerpToTargetOffsetDuration( 0.0f )
{
}

void UGBFLocalMultiplayerSubsystem::ApplySplitScreenOffset( FGBFViewPortPlayerOffset offset, FGBFViewportSplitScreenOffsetLerpEndedDelegate on_lerp_ended_delegate, float duration )
{
    if ( const auto * world = GetWorld() )
    {
        if ( auto * viewport = Cast< UGBFGameViewportClient >( world->GetGameViewport() ) )
        {
            if ( duration <= 0.0f )
            {
                viewport->SetSplitScreenLayoutOffset( offset );
                on_lerp_ended_delegate.ExecuteIfBound();
                return;
            }

            OriginalOffset = viewport->GetSplitScreenLayoutOffset();
            TargetOffset = offset;
            LerpToTargetOffsetDuration = duration;
            LerpToTargetOffsetRemainingTime = duration;
        }
    }
}

void UGBFLocalMultiplayerSubsystem::UpdatePlayerSplitScreenType( ETwoPlayerSplitScreenType::Type two_players_split_type, EThreePlayerSplitScreenType::Type three_players_split_type, EFourPlayerSplitScreenType four_players_split_type )
{
    auto * game_settings = GetMutableDefault< UGameMapsSettings >();
    game_settings->TwoPlayerSplitscreenLayout = two_players_split_type;
    game_settings->ThreePlayerSplitscreenLayout = three_players_split_type;
    game_settings->FourPlayerSplitscreenLayout = four_players_split_type;
}

void UGBFLocalMultiplayerSubsystem::AddMappingContextToAllPlayers( TSoftObjectPtr< UInputMappingContext > input_mapping_context, int priority, FModifyContextOptions options )
{
    for ( auto lp_iterator = GetWorld()->GetGameInstance()->GetLocalPlayerIterator(); lp_iterator; ++lp_iterator )
    {
        if ( const auto * lp = *lp_iterator )
        {
            if ( auto * input_system = lp->GetSubsystem< UEnhancedInputLocalPlayerSubsystem >() )
            {
                if ( const auto * imc = input_mapping_context.Get() )
                {
                    input_system->AddMappingContext( imc, priority, options );
                }
            }
        }
    }
}

void UGBFLocalMultiplayerSubsystem::RemoveMappingContextToAllPlayers( TSoftObjectPtr< UInputMappingContext > input_mapping_context, FModifyContextOptions options )
{
    for ( auto lp_iterator = GetWorld()->GetGameInstance()->GetLocalPlayerIterator(); lp_iterator; ++lp_iterator )
    {
        if ( const auto * lp = *lp_iterator )
        {
            if ( auto * input_system = lp->GetSubsystem< UEnhancedInputLocalPlayerSubsystem >() )
            {
                if ( const auto * imc = input_mapping_context.Get() )
                {
                    input_system->RemoveMappingContext( imc, options );
                }
            }
        }
    }
}

void UGBFLocalMultiplayerSubsystem::Tick( float delta_time )
{
    Super::Tick( delta_time );

    if ( const auto * world = GetWorld() )
    {
        if ( auto * viewport = Cast< UGBFGameViewportClient >( world->GetGameViewport() ) )
        {
            if ( LerpToTargetOffsetRemainingTime > 0.0f )
            {
                LerpToTargetOffsetRemainingTime -= delta_time;

                const auto offset = FGBFViewPortPlayerOffset::Lerp( OriginalOffset, TargetOffset, 1.0f - LerpToTargetOffsetRemainingTime / LerpToTargetOffsetDuration );
                viewport->SetSplitScreenLayoutOffset( offset );

                if ( LerpToTargetOffsetRemainingTime <= 0.0f )
                {
                    OnLerpEndedDelegate.ExecuteIfBound();
                }
            }
        }
    }
}

TStatId UGBFLocalMultiplayerSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT( UGBFViewportSplitScreenLayoutSubsystem, STATGROUP_Tickables );
}
