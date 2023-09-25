#pragma once

#include <CommonInputBaseTypes.h>
#include <GameplayTagContainer.h>
#include <PlayerMappableInputConfig.h>
#include <UObject/SoftObjectPtr.h>

#include "GBFMappableConfigPair.generated.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

/** A container to organize loaded player mappable configs to their CommonUI input type */
USTRUCT( BlueprintType )
struct UE_DEPRECATED( 5.3, "FGBFLoadedMappableConfigPair has been deprecated. Please use FInputMappingContextAndPriority instead." ) FGBFLoadedMappableConfigPair
{
    GENERATED_BODY()

    FGBFLoadedMappableConfigPair() = default;
    FGBFLoadedMappableConfigPair( const UPlayerMappableInputConfig * config, ECommonInputType type, const bool is_active ) :
        Config( config ),
        Type( type ),
        bIsActive( is_active )
    {}

    /** The player mappable input config that should be applied to the Enhanced Input subsystem */
    UPROPERTY( BlueprintReadOnly, VisibleAnywhere )
    TObjectPtr< const UPlayerMappableInputConfig > Config = nullptr;

    /** The type of device that this mapping config should be applied to */
    UPROPERTY( BlueprintReadOnly, VisibleAnywhere )
    ECommonInputType Type = ECommonInputType::Count;

    /** If this config is currently active. A config is marked as active when it's owning GFA is active */
    UPROPERTY( BlueprintReadOnly, VisibleAnywhere )
    bool bIsActive = false;
};

/** A container to organize potentially unloaded player mappable configs to their CommonUI input type */
USTRUCT()
struct UE_DEPRECATED( 5.3, "FGBFMappableConfigPair has been deprecated. Please use FInputMappingContextAndPriority instead." ) FGBFMappableConfigPair
{
    GENERATED_BODY()

    FGBFMappableConfigPair() = default;

    UPROPERTY( EditAnywhere )
    TSoftObjectPtr< UPlayerMappableInputConfig > Config;

    /**
     * The type of config that this is. Useful for filtering out configs by the current input device
     * for things like the settings screen, or if you only want to apply this config when a certain
     * input type is being used.
     */
    UPROPERTY( EditAnywhere )
    ECommonInputType Type = ECommonInputType::Count;

    /**
     * Container of platform traits that must be set in order for this input to be activated.
     *
     * If the platform does not have one of the traits specified it can still be registered, but cannot
     * be activated.
     */
    UPROPERTY( EditAnywhere )
    FGameplayTagContainer DependentPlatformTraits;

    /**
     * If the current platform has any of these traits, then this config will not be actived.
     */
    UPROPERTY( EditAnywhere )
    FGameplayTagContainer ExcludedPlatformTraits;

    /** If true, then this input config will be activated when it's associated Game Feature is activated.
     * This is normally the desirable behavior
     */
    UPROPERTY( EditAnywhere )
    bool bShouldActivateAutomatically = true;

    /** Returns true if this config pair can be activated based on the current platform traits and settings. */
    bool CanBeActivated() const;

    /**
     * Registers the given config mapping with the local settings
     */
    static bool RegisterPair( const FGBFMappableConfigPair & pair );

    /**
     * Unregisters the given config mapping with the local settings
     */
    static void UnregisterPair( const FGBFMappableConfigPair & pair );
};

PRAGMA_ENABLE_DEPRECATION_WARNINGS