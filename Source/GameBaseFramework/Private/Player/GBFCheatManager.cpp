#include "Player/GBFCheatManager.h"

#include "GameFramework/GBFPlayerController.h"

#include <GameFramework/PlayerController.h>

namespace Private
{
    static bool bStartInGodMode = false;
    static FAutoConsoleVariableRef CVarStartInGodMode(
        TEXT( "Cheat.StartInGodMode" ),
        bStartInGodMode,
        TEXT( "If true then the God cheat will be applied on begin play" ),
        ECVF_Cheat );
}

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

    if ( Private::bStartInGodMode )
    {
        God();
    }
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
