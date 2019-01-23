#include "GBFGameModeBase.h"
#include "GBFGameInstance.h"

void AGBFGameModeBase::StartPlay()
{
    Super::StartPlay();

    GetGameInstance< UGBFGameInstance >()->PushSoundMixModifier();
}

void AGBFGameModeBase::EndPlay( EEndPlayReason::Type reason )
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
