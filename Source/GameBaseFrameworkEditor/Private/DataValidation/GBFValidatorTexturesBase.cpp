#include "DataValidation/GBFValidatorTexturesBase.h"

#include "DataValidation/GBFDataValidationMacros.h"
#include "Path/GBFPathFilter.h"

UGBFValidatorTexturesBase::UGBFValidatorTexturesBase()
{
    ItHasSortedApplicators = false;

    RegisterTextureSettingsApplicator(
        { []( const FString & path, const FString & /* name */ ) {
             FGBFPathFilter path_filter;
             path_filter.MustContain( TEXT( "/UI/" ) );

             return path_filter.Matches( path );
         },
            []( FTextureSettings & texture_settings ) {
                texture_settings.RequiredTextureGroup = TextureGroup::TEXTUREGROUP_UI;
                texture_settings.RequiredCompressionSettings = { TextureCompressionSettings::TC_EditorIcon };
                texture_settings.SetCheckSRGB( ECheckFlag::CheckItIsOn );
                texture_settings.SetCheckNeverStream( ECheckFlag::CheckItIsOn );
                texture_settings.OptionalMipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
            } } );

    RegisterTextureSettingsApplicator(
        { []( const FString & /*path*/, const FString & name ) {
             FGBFPathFilter name_filter;
             name_filter.MustEndWith( TEXT( "_N" ) );

             return name_filter.Matches( name );
         },
            []( FTextureSettings & texture_settings ) {
                switch ( texture_settings.RequiredTextureGroup )
                {
                    case TextureGroup::TEXTUREGROUP_World:
                    {
                        texture_settings.RequiredTextureGroup = TextureGroup::TEXTUREGROUP_WorldNormalMap;
                    }
                    break;
                    case TextureGroup::TEXTUREGROUP_Weapon:
                    {
                        texture_settings.RequiredTextureGroup = TextureGroup::TEXTUREGROUP_WeaponNormalMap;
                    }
                    break;
                    case TextureGroup::TEXTUREGROUP_Character:
                    {
                        texture_settings.RequiredTextureGroup = TextureGroup::TEXTUREGROUP_CharacterNormalMap;
                    }
                    break;
                    default:
                        break;
                }
            },
            // -5 so it runs after everything else. This allows to refine the texture group of a normal map texture after it has been set to a non-normal map texture group by another applicator
            -5,
            false } );
}

bool UGBFValidatorTexturesBase::CanValidateAsset_Implementation( UObject * in_asset ) const
{
    return in_asset->GetClass()->IsChildOf( UTexture::StaticClass() );
}

EDataValidationResult UGBFValidatorTexturesBase::ValidateLoadedAsset_Implementation( UObject * in_asset, TArray< FText > & validation_errors )
{
    auto * texture = Cast< UTexture >( in_asset );

    FTextureSettings texture_settings;

    const auto width = static_cast< int32 >( texture->GetSurfaceWidth() );
    const auto height = static_cast< int32 >( texture->GetSurfaceHeight() );

    const auto path_name = in_asset->GetPathName();
    const auto package_name = in_asset->GetName();

    if ( !ItHasSortedApplicators )
    {
        TextureSettingsApplicators.Sort( []( const auto & left, const auto & right ) {
            return left.Priority > right.Priority;
        } );
        ItHasSortedApplicators = true;
    }

    for ( const auto & applicator : TextureSettingsApplicators )
    {
        if ( applicator.Predicate( in_asset->GetPathName(), in_asset->GetName() ) )
        {
            applicator.UpdateSettings( texture_settings );

            if ( applicator.IsExclusive )
            {
                break;
            }
        }
    }

    TArray< FString > name_parts;
    package_name.ParseIntoArray( name_parts, TEXT( "_" ) );

    if ( name_parts.Last() == TEXT( "N" ) )
    {
        texture_settings.RequiredCompressionSettings = { TextureCompressionSettings::TC_Normalmap };
        texture_settings.SetCheckSRGB( ECheckFlag::CheckItIsOff );
    }
    else if ( name_parts.Last() == TEXT( "M" ) )
    {
        texture_settings.RequiredCompressionSettings = { TextureCompressionSettings::TC_Masks, TextureCompressionSettings::TC_Grayscale };
        // :NOTE: sRGB is checked later for those compression settings
    }
    else if ( name_parts.Last() == TEXT( "DM" ) )
    {
        texture_settings.RequiredCompressionSettings = { TextureCompressionSettings::TC_Displacementmap };
        texture_settings.SetCheckSRGB( ECheckFlag::CheckItIsOff );
    }
    else if ( name_parts.Last() == TEXT( "VDM" ) )
    {
        texture_settings.RequiredCompressionSettings = { TextureCompressionSettings::TC_VectorDisplacementmap };
        texture_settings.SetCheckSRGB( ECheckFlag::CheckItIsOff );
    }
    else if ( name_parts.Last() == TEXT( "HDR" ) )
    {
        texture_settings.RequiredCompressionSettings = { TextureCompressionSettings::TC_HDR };
    }
    else if ( name_parts.Last() == TEXT( "AM" ) ) // AlphaMask
    {
        texture_settings.RequiredCompressionSettings = { TextureCompressionSettings::TC_Alpha };
        texture_settings.SetCheckSRGB( ECheckFlag::CheckItIsOff );
    }
    else if ( name_parts.Last() == TEXT( "DFF" ) )
    {
        texture_settings.RequiredCompressionSettings = { TextureCompressionSettings::TC_DistanceFieldFont };
    }

    switch ( texture->CompressionSettings )
    {
        case TextureCompressionSettings::TC_Masks:
        {
            texture_settings.SetCheckSRGB( ECheckFlag::CheckItIsOff );        
        }
        break;
        case TextureCompressionSettings::TC_Grayscale:
        {
            texture_settings.SetCheckSRGB( ECheckFlag::CheckItIsOn );
        }
        break;
        default:
            break;
    }

    const auto check_power_of_two = [ &validation_errors, texture ]( const int32 dimension, const FString & dimension_str ) {
        if ( !FMath::IsPowerOfTwo( dimension )
            && texture->PowerOfTwoMode == ETexturePowerOfTwoSetting::None )
        {
            validation_errors.Emplace(
                FText::FromString(
                    FString::Printf(
                        TEXT( "%s must a power of two %s. Current : %i. You can also change PowerOfTwoMode." ),
                        *GetNameSafe( texture ),
                        *dimension_str,
                        dimension ) ) );
        }
    };

#define CHECK_POWER_OF_TWO( dimension ) check_power_of_two( dimension, TEXT( #dimension ) );

    CHECK_POWER_OF_TWO( width );
    CHECK_POWER_OF_TWO( height );

#undef CHECK_POWER_OF_TWO

    if ( texture->LODGroup != texture_settings.RequiredTextureGroup )
    {
        validation_errors.Emplace(
            FText::FromString(
                FString::Printf(
                    TEXT( "%s must have the texture group set to %s" ),
                    *GetNameSafe( in_asset ),
                    *UEnum::GetValueAsString( texture_settings.RequiredTextureGroup ) ) ) );
    }

    if ( texture_settings.RequiredCompressionSettings.Num() > 0 && !texture_settings.RequiredCompressionSettings.Contains( TextureCompressionSettings::TC_Default ) && !texture_settings.RequiredCompressionSettings.Contains( texture->CompressionSettings ) )
    {
        FString required_compression_settings_str;

        for ( const auto compression_setting : texture_settings.RequiredCompressionSettings )
        {
            if ( required_compression_settings_str.Len() != 0 )
            {
                required_compression_settings_str += TEXT( "," );
            }
            required_compression_settings_str += UEnum::GetValueAsString( compression_setting );
        }

        validation_errors.Emplace(
            FText::FromString(
                FString::Printf(
                    TEXT( "%s must have the texture compression settings set to one of those values : %s" ),
                    *GetNameSafe( in_asset ),
                    *required_compression_settings_str ) ) );
    }

    const auto check_flag_lambda = [ &validation_errors, in_asset ]( const bool flag, const FString & flag_name, const ECheckFlag check_flag ) {
        switch ( check_flag )
        {
            case ECheckFlag::CheckItIsOff:
            {
                if ( flag )
                {
                    validation_errors.Emplace(
                        FText::FromString(
                            FString::Printf(
                                TEXT( "%s must not have %s checked" ),
                                *GetNameSafe( in_asset ),
                                *flag_name ) ) );
                }
            }
            break;
            case ECheckFlag::CheckItIsOn:
            {
                if ( !flag )
                {
                    validation_errors.Emplace(
                        FText::FromString(
                            FString::Printf(
                                TEXT( "%s must have %s checked" ),
                                *GetNameSafe( in_asset ),
                                *flag_name ) ) );
                }
            }
            break;
            default:
                break;
        };
    };

#define CHECK_FLAG( flag_name, flag_value ) check_flag_lambda( texture->flag_name, TEXT( #flag_name ), flag_value );

    CHECK_FLAG( SRGB, texture_settings.GetCheckSRGB() )
    CHECK_FLAG( NeverStream, texture_settings.GetCheckNeverStream() )

#undef CHECK_FLAG

    DATA_VALIDATION_RETURN_FROM_VALIDATOR( in_asset );
}

void UGBFValidatorTexturesBase::RegisterTextureSettingsApplicator( const FTextureSettingsApplicator & applicator )
{
    TextureSettingsApplicators.Emplace( applicator );
}