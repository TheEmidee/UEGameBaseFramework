#include "GameFramework/GBFGameModeBase.h"
#include "Engine/GBFGameInstance.h"
#include "Online/GBFGameSession.h"

AGBFGameModeBase::AGBFGameModeBase()
{
    GameSessionClass = AGBFGameSession::StaticClass();
}

void AGBFGameModeBase::StartPlay()
{
    Super::StartPlay();

    GetGameInstance< UGBFGameInstance >()->PushSoundMixModifier();
}

void AGBFGameModeBase::EndPlay( const EEndPlayReason::Type reason )
{
    Super::EndPlay( reason );

    GetGameInstance< UGBFGameInstance >()->PopSoundMixModifier();
}

bool AGBFGameModeBase::CanPauseGame() const
{
    return true;
}

void AGBFGameModeBase::HandleAppSuspended()
{
}

void AGBFGameModeBase::HandleAppResumed()
{
}
