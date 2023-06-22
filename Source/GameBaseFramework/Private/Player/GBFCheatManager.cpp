#include "Player/GBFCheatManager.h"

#include "Characters/Components/GBFHealthComponent.h"
#include "GameFramework/GBFPlayerController.h"

#include <GameFramework/Pawn.h>
#include <GameFramework/PlayerController.h>

void UGBFCheatManager::InitCheatManager()
{
    Super::InitCheatManager();

#if WITH_EDITOR
    if ( GIsEditor )
    {
        auto * pc = GetOuterAPlayerController();
        for ( const auto & CheatRow : GetDefault< UGameBaseFrameworkSettings >()->CheatsToRun )
        {
            if ( CheatRow.Phase == ECheatExecutionTime::OnCheatManagerCreated )
            {
                pc->ConsoleCommand( CheatRow.Cheat, /*bWriteToLog=*/true );
            }
        }
    }
#endif
}

void UGBFCheatManager::Cheat( const FString & message )
{
    if ( auto * pc = Cast< AGBFPlayerController >( GetOuterAPlayerController() ) )
    {
        pc->ServerCheat( message.Left( 128 ) );
    }
}

void UGBFCheatManager::CheatAll( const FString & message )
{
    if ( auto * pc = Cast< AGBFPlayerController >( GetOuterAPlayerController() ) )
    {
        pc->ServerCheatAll( message.Left( 128 ) );
    }
}

void UGBFCheatManager::SelfDestructLocalPlayer()
{
    if ( const auto * pc = Cast< AGBFPlayerController >( GetOuterAPlayerController() ) )
    {
        if ( const auto * pawn = pc->GetPawn() )
        {
            if ( const auto * health_component = UGBFHealthComponent::FindHealthComponent( pawn ) )
            {
                health_component->DamageSelfDestruct( nullptr );
            }
        }
    }
}
