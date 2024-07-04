#include "Experiences/GBFUserFacingExperienceDefinition.h"

#include "BlueprintLibraries/CoreExtHelperBlueprintLibrary.h"

#include <CommonSessionSubsystem.h>
#include <CommonUISettings.h>
#include <ICommonUIModule.h>
#include <Misc/CommandLine.h>
#include <NativeGameplayTags.h>
#include <UObject/Package.h>

namespace GameBaseFramework::Experience
{
    UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_Platform_Trait_ReplaySupport, "Platform.Trait.ReplaySupport" );
}

FPrimaryAssetId UGBFUserFacingExperienceDefinition::GetPrimaryAssetId() const
{
    return FPrimaryAssetId( GetPrimaryAssetType(), GetPackage()->GetFName() );
}

FPrimaryAssetType UGBFUserFacingExperienceDefinition::GetPrimaryAssetType()
{
    static const FPrimaryAssetType PrimaryAssetType( TEXT( "UserFacingExperienceDefinition" ) );
    return PrimaryAssetType;
}

UCommonSession_HostSessionRequest * UGBFUserFacingExperienceDefinition::CreateHostingRequest( const UObject * context ) const
{
    const auto experience_name = ExperienceID.PrimaryAssetName.ToString();
    const auto user_facing_experience_name = GetPrimaryAssetId().PrimaryAssetName.ToString();

    UCommonSession_HostSessionRequest * result = nullptr;

    if ( auto * world = GEngine->GetWorldFromContextObject( context, EGetWorldErrorMode::ReturnNull ) )
    {
        if ( auto * game_instance = world->GetGameInstance() )
        {
            if ( auto * sub_system = game_instance->GetSubsystem< UCommonSessionSubsystem >() )
            {
                result = sub_system->CreateOnlineHostSessionRequest();
            }
        }
    }

    if ( result == nullptr )
    {
        result = NewObject< UCommonSession_HostSessionRequest >();
        result->OnlineMode = OnlineMode;
        result->bUseLobbies = true;
    }

    result->MapID = MapID;
    result->ModeNameForAdvertisement = user_facing_experience_name;
    result->ModeNameForAdvertisement = user_facing_experience_name;
    result->ExtraArgs = ExtraArgs;

    UCoreExtHelperBlueprintLibrary::ParseOptionsFromString( result->ExtraArgs, FCommandLine::Get() );

    result->ExtraArgs.Add( TEXT( "Experience" ), experience_name );
    result->MaxPlayerCount = MaxPlayerCount;

    if ( ICommonUIModule::GetSettings().GetPlatformTraits().HasTag( GameBaseFramework::Experience::TAG_Platform_Trait_ReplaySupport.GetTag() ) )
    {
        if ( bRecordReplay )
        {
            result->ExtraArgs.Add( TEXT( "DemoRec" ), FString() );
        }
    }

    return result;
}