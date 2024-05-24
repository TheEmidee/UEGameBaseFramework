#include "Input/Modifiers/GBFInputModifier_SettingsBased_Scalar3.h"

#include "Input/Modifiers/GBFInputModifierHelpers.h"

FInputActionValue UGBFInputModifier_SettingsBased_Scalar3::ModifyRaw_Implementation( const UEnhancedPlayerInput * player_input, const FInputActionValue current_value, float delta_time )
{
    if ( ensureMsgf( current_value.GetValueType() != EInputActionValueType::Boolean, TEXT( "Setting Based Scalar modifier doesn't support boolean values." ) ) )
    {
        if ( const auto * local_player = GBFInputModifiersHelpers::GetLocalPlayer( player_input ) )
        {
            const auto * settings_class = UGBFSettingsShared::StaticClass();
            auto * shared_settings = local_player->GetSharedSettings();

            const auto has_cached_property = PropertyCache.Num() == 3;

            const auto * x_axis_value = has_cached_property ? PropertyCache[ 0 ] : settings_class->FindPropertyByName( XAxisScalarSettingName );
            const auto * y_axis_value = has_cached_property ? PropertyCache[ 1 ] : settings_class->FindPropertyByName( YAxisScalarSettingName );
            const auto * z_axis_value = has_cached_property ? PropertyCache[ 2 ] : settings_class->FindPropertyByName( ZAxisScalarSettingName );

            if ( PropertyCache.IsEmpty() )
            {
                PropertyCache.Emplace( x_axis_value );
                PropertyCache.Emplace( y_axis_value );
                PropertyCache.Emplace( z_axis_value );
            }

            auto scalar_to_use = FVector( 1.0, 1.0, 1.0 );

            switch ( current_value.GetValueType() )
            {
                case EInputActionValueType::Axis3D:
                {
                    scalar_to_use.Z = z_axis_value ? *z_axis_value->ContainerPtrToValuePtr< double >( shared_settings ) : 1.0;
                }
                //[[fallthrough]];
                case EInputActionValueType::Axis2D:
                {
                    scalar_to_use.Y = y_axis_value ? *y_axis_value->ContainerPtrToValuePtr< double >( shared_settings ) : 1.0;
                }
                //[[fallthrough]];
                case EInputActionValueType::Axis1D:
                {
                    scalar_to_use.X = x_axis_value ? *x_axis_value->ContainerPtrToValuePtr< double >( shared_settings ) : 1.0;
                }
                break;
                default:
                {
                    checkNoEntry();
                };
            }

            scalar_to_use.X = FMath::Clamp( scalar_to_use.X, MinValueClamp.X, MaxValueClamp.X );
            scalar_to_use.Y = FMath::Clamp( scalar_to_use.Y, MinValueClamp.Y, MaxValueClamp.Y );
            scalar_to_use.Z = FMath::Clamp( scalar_to_use.Z, MinValueClamp.Z, MaxValueClamp.Z );

            return current_value.Get< FVector >() * scalar_to_use;
        }
    }

    return current_value;
}
