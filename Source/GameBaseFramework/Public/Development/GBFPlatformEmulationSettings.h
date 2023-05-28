#pragma once

#include <CoreMinimal.h>
#include <Engine/DeveloperSettingsBackedByCVars.h>
#include <GameplayTagContainer.h>

#include "GBFPlatformEmulationSettings.generated.h"

UCLASS( config = EditorPerProjectUserSettings, MinimalAPI )
class UGBFPlatformEmulationSettings : public UDeveloperSettingsBackedByCVars
{
    GENERATED_BODY()

public:
    FName GetCategoryName() const override;

    FName GetPretendBaseDeviceProfile() const;
    FName GetPretendPlatformName() const;

#if WITH_EDITOR
    void PostEditChangeProperty( FPropertyChangedEvent & property_changed_event ) override;
    void PostReloadConfig( FProperty * property_that_was_loaded ) override;
    void PostInitProperties() override;
    // Called by the editor engine to let us pop reminder notifications when cheats are active
    GAMEBASEFRAMEWORK_API void OnPlayInEditorStarted() const;
#endif
private:
#if WITH_EDITOR
    void ApplySettings();
    void ChangeActivePretendPlatform( FName new_platform_name );
#endif

    UFUNCTION()
    TArray< FName > GetKnownPlatformIds() const;

    UFUNCTION()
    TArray< FName > GetKnownDeviceProfiles() const;

    void PickReasonableBaseDeviceProfile();

    UPROPERTY( EditAnywhere, config, Category = PlatformEmulation, meta = ( Categories = "Input,Platform.Trait" ) )
    FGameplayTagContainer AdditionalPlatformTraitsToEnable;

    UPROPERTY( EditAnywhere, config, Category = PlatformEmulation, meta = ( Categories = "Input,Platform.Trait" ) )
    FGameplayTagContainer AdditionalPlatformTraitsToSuppress;

    UPROPERTY( EditAnywhere, config, Category = PlatformEmulation, meta = ( GetOptions = GetKnownPlatformIds ) )
    FName PretendPlatform;

    // The base device profile to pretend we are using when emulating device-specific device profiles applied from ULyraSettingsLocal
    UPROPERTY( EditAnywhere, config, Category = PlatformEmulation, meta = ( GetOptions = GetKnownDeviceProfiles, EditCondition = bApplyDeviceProfilesInPIE ) )
    FName PretendBaseDeviceProfile;

    // Do we apply desktop-style frame rate settings in PIE?
    // (frame rate limits are an engine-wide setting so it's not always desirable to have enabled in the editor)
    // You may also want to disable the editor preference "Use Less CPU when in Background" if testing background frame rate limits
    UPROPERTY( EditAnywhere, config, Category = PlatformEmulation, meta = ( ConsoleVariable = "GBF.Settings.ApplyFrameRateSettingsInPIE" ) )
    bool bApplyFrameRateSettingsInPIE = false;

    // Do we apply front-end specific performance options in PIE?
    // Most engine performance/scalability settings they drive are global, so if one PIE window
    // is in the front-end and the other is in-game one will win and the other gets stuck with those settings
    UPROPERTY( EditAnywhere, config, Category = PlatformEmulation, meta = ( ConsoleVariable = "GBF.Settings.ApplyFrontEndPerformanceOptionsInPIE" ) )
    bool bApplyFrontEndPerformanceOptionsInPIE = false;

    // Should we apply experience/platform emulated device profiles in PIE?
    UPROPERTY( EditAnywhere, config, Category = PlatformEmulation, meta = ( InlineEditConditionToggle, ConsoleVariable = "GBF.Settings.ApplyDeviceProfilesInPIE" ) )
    bool bApplyDeviceProfilesInPIE = false;

#if WITH_EDITOR
    // The last pretend platform we applied
    FName LastAppliedPretendPlatform;
#endif
};
