#include "Engine/SubSystems/GBFGameInstanceControllerSubsystem.h"

#include "Components/GBFUIDialogManagerComponent.h"
#include "Engine/GBFGameInstance.h"
#include "Engine/GBFLocalPlayer.h"
#include "GBFLog.h"
#include "GameFramework/GBFPlayerController.h"

#include <Engine/LocalPlayer.h>
#include <Misc/CoreDelegates.h>
#include <OnlineSubsystem.h>
#include <OnlineSubsystemUtils.h>

UGBFGameInstanceControllerSubsystem::UGBFGameInstanceControllerSubsystem() :
    IgnorePairingChangeForControllerId( -1 )
{
}

void UGBFGameInstanceControllerSubsystem::Initialize( FSubsystemCollectionBase & collection )
{
    Super::Initialize( collection );

    const auto oss = IOnlineSubsystem::Get();
    check( oss != nullptr );

    const auto identity_interface = oss->GetIdentityInterface();
    check( identity_interface.IsValid() );
}