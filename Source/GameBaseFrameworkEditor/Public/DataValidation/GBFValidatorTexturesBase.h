#pragma once

#include <CoreMinimal.h>
#include <EditorValidatorBase.h>

#include "GBFValidatorTexturesBase.generated.h"

UCLASS( Abstract )
class GAMEBASEFRAMEWORKEDITOR_API UGBFValidatorTexturesBase : public UEditorValidatorBase
{
    GENERATED_BODY()

public:
    UGBFValidatorTexturesBase();

    bool CanValidateAsset_Implementation( UObject * in_asset ) const override;
    EDataValidationResult ValidateLoadedAsset_Implementation( UObject * in_asset, TArray< FText > & validation_errors ) override;

protected:
    enum class ECheckFlag : uint8
    {
        DontCheck,
        CheckItIsOn,
        CheckItIsOff
    };

    struct FTextureSettings
    {
        ECheckFlag GetCheckSRGB() const
        {
            return CheckSRGB;
        }

        ECheckFlag GetCheckNeverStream() const
        {
            return CheckNeverStream;
        }

        TextureGroup RequiredTextureGroup = TextureGroup::TEXTUREGROUP_World;
        TArray< TextureCompressionSettings > RequiredCompressionSettings = { TextureCompressionSettings::TC_Default };

        void SetCheckSRGB( const ECheckFlag new_value )
        {
            if ( ensureAlwaysMsgf( CheckSRGB == ECheckFlag::DontCheck, TEXT( "You're trying to set the value of CheckSRGB but it was already set" ) ) )
            {
                CheckSRGB = new_value;
            }
        }

        void SetCheckNeverStream( const ECheckFlag new_value )
        {
            if ( ensureAlwaysMsgf( CheckNeverStream == ECheckFlag::DontCheck, TEXT( "You're trying to set the value of CheckNeverStream but it was already set" ) ) )
            {
                CheckNeverStream = new_value;
            }
        }

    private:
        ECheckFlag CheckSRGB = ECheckFlag::DontCheck;
        ECheckFlag CheckNeverStream = ECheckFlag::DontCheck;
    };

    struct FTextureSettingsApplicator
    {
        FTextureSettingsApplicator( const TFunction< bool( const FString &, const FString & ) > & predicate, const TFunction< void( FTextureSettings & ) > & update_settings ) :
            Predicate( predicate ),
            UpdateSettings( update_settings ),
            Priority( 0 ),
            IsExclusive( false ) {};

        FTextureSettingsApplicator( const TFunction< bool( const FString &, const FString & ) > & predicate, const TFunction< void( FTextureSettings & ) > & update_settings, const int priority, const bool is_exclusive ) :
            Predicate( predicate ),
            UpdateSettings( update_settings ),
            Priority( priority ),
            IsExclusive( is_exclusive ) {};

        TFunction< bool( const FString &, const FString & ) > Predicate;
        TFunction< void( FTextureSettings & ) > UpdateSettings;
        int Priority;
        bool IsExclusive;
    };

    void RegisterTextureSettingsApplicator( const FTextureSettingsApplicator & applicator );

private:
    TArray< FTextureSettingsApplicator > TextureSettingsApplicators;
    bool ItHasSortedApplicators;
};
