#include "GameFramework/Experiences/GBFUserFacingExperienceDefinition.h"

#include "BlueprintLibraries/CoreExtHelperBlueprintLibrary.h"

#include <CommonSessionSubsystem.h>
#include <CommonUISettings.h>
#include <ICommonUIModule.h>
#include <Misc/CommandLine.h>
#include <NativeGameplayTags.h>

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

UCommonSession_HostSessionRequest * UGBFUserFacingExperienceDefinition::CreateHostingRequest() const
{
    const auto experience_name = ExperienceID.PrimaryAssetName.ToString();
    const auto user_facing_experience_name = GetPrimaryAssetId().PrimaryAssetName.ToString();
    auto * result = NewObject< UCommonSession_HostSessionRequest >();
    result->OnlineMode = OnlineMode;
    result->bUseLobbies = true;
    result->MapID = MapID;
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