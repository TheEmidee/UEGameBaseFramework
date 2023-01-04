#include "Engine/SubSystems//GBFGameInstanceCoreDelegatesSubsystem.h"

#include "GBFLog.h"

#include <Engine/Canvas.h>
#include <Framework/Application/SlateApplication.h>
#include <Interfaces/OnlineIdentityInterface.h>
#include <Misc/CoreDelegates.h>
#include <OnlineSubsystem.h>
#include <OnlineSubsystemTypes.h>

UGBFGameInstanceCoreDelegatesSubsystem::UGBFGameInstanceCoreDelegatesSubsystem() :
    ItIsLicensed( true )
{
}

void UGBFGameInstanceCoreDelegatesSubsystem::Initialize( FSubsystemCollectionBase & collection )
{
    Super::Initialize( collection );

    const auto oss = IOnlineSubsystem::Get();
    check( oss != nullptr );

    const auto identity_interface = oss->GetIdentityInterface();
    check( identity_interface.IsValid() );

    for ( auto i = 0; i < MAX_LOCAL_PLAYERS; ++i )
    {
        identity_interface->AddOnLoginStatusChangedDelegate_Handle( i, FOnLoginStatusChangedDelegate::CreateUObject( this, &UGBFGameInstanceCoreDelegatesSubsystem::HandleUserLoginChanged ) );
    }

    FCoreDelegates::ApplicationWillDeactivateDelegate.AddUObject( this, &UGBFGameInstanceCoreDelegatesSubsystem::HandleAppWillDeactivate );
    FCoreDelegates::ApplicationHasReactivatedDelegate.AddUObject( this, &UGBFGameInstanceCoreDelegatesSubsystem::HandleAppHasReactivated );
    FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddUObject( this, &UGBFGameInstanceCoreDelegatesSubsystem::HandleAppWillEnterBackground );
    FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddUObject( this, &UGBFGameInstanceCoreDelegatesSubsystem::HandleAppHasEnteredForeground );
    FCoreDelegates::OnSafeFrameChangedEvent.AddUObject( this, &UGBFGameInstanceCoreDelegatesSubsystem::HandleSafeFrameChanged );
    FCoreDelegates::ApplicationLicenseChange.AddUObject( this, &UGBFGameInstanceCoreDelegatesSubsystem::HandleAppLicenseUpdate );
}

void UGBFGameInstanceCoreDelegatesSubsystem::HandleUserLoginChanged( int32 /* game_user_index */, ELoginStatus::Type /* previous_login_status */, ELoginStatus::Type /* login_status */, const FUniqueNetId & /* user_id */ )
{
    HandleAppLicenseUpdate();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void UGBFGameInstanceCoreDelegatesSubsystem::HandleAppWillDeactivate()
{
    UE_LOG( LogGBF_OSS, Warning, TEXT( "UGBFGameInstanceCoreDelegatesSubsystem::HandleAppWillDeactivate" ) );
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void UGBFGameInstanceCoreDelegatesSubsystem::HandleAppHasReactivated()
{
    UE_LOG( LogGBF_OSS, Warning, TEXT( "UGBFGameInstanceCoreDelegatesSubsystem::HandleAppHasReactivated" ) );
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void UGBFGameInstanceCoreDelegatesSubsystem::HandleAppWillEnterBackground()
{
    UE_LOG( LogGBF_OSS, Warning, TEXT( "UGBFGameInstanceCoreDelegatesSubsystem::HandleAppWillEnterBackground" ) );

#if PLATFORM_SWITCH
    HandleAppDeactivateOrBackground();
#endif
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void UGBFGameInstanceCoreDelegatesSubsystem::HandleAppHasEnteredForeground()
{
    UE_LOG( LogGBF_OSS, Log, TEXT( "UGBFGameInstanceCoreDelegatesSubsystem::HandleAppHasEnteredForeground" ) );

#if PLATFORM_SWITCH
    HandleAppReactivateOrForeground();
#endif
}

void UGBFGameInstanceCoreDelegatesSubsystem::HandleAppDeactivateOrBackground() const
{
    OnAppDeactivateOrBackgroundDelegate.Broadcast();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UGBFGameInstanceCoreDelegatesSubsystem::HandleAppReactivateOrForeground()
{
    OnAppReactivatedOrForegroundDelegate.Broadcast();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void UGBFGameInstanceCoreDelegatesSubsystem::HandleSafeFrameChanged()
{
    UCanvas::UpdateAllCanvasSafeZoneData();
}

void UGBFGameInstanceCoreDelegatesSubsystem::HandleAppLicenseUpdate()
{
    auto generic_application = FSlateApplication::Get().GetPlatformApplication();
    ItIsLicensed = generic_application->ApplicationLicenseValid();
}
