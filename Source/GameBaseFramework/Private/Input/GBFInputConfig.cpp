#include "Input/GBFInputConfig.h"

#include "GBFLog.h"

namespace
{
    const UInputAction * FindInputActionForTag( const FGameplayTag & input_tag, const TArray< FGBFInputAction > & actions )
    {
        for ( const auto & [ input_action, tag ] : actions )
        {
            if ( input_action && tag == input_tag )
            {
                return input_action;
            }
        }

        return nullptr;
    }
}

const UInputAction * UGBFInputConfig::FindNativeInputActionForTag( const FGameplayTag & input_tag, bool log_not_found ) const
{
    auto * action = FindInputActionForTag( input_tag, NativeInputActions );

    if ( action == nullptr && log_not_found )
    {
        UE_LOG( LogGBF_Input, Error, TEXT( "Can't find NativeInputAction for InputTag [%s] on InputConfig [%s]." ), *input_tag.ToString(), *GetNameSafe( this ) );
    }

    return action;
}

const UInputAction * UGBFInputConfig::FindAbilityInputActionForTag( const FGameplayTag & input_tag, bool log_not_found ) const
{
    auto * action = FindInputActionForTag( input_tag, AbilityInputActions );

    if ( action == nullptr && log_not_found )
    {
        UE_LOG( LogGBF_Input, Error, TEXT( "Can't find AbilityInputAction for InputTag [%s] on InputConfig [%s]." ), *input_tag.ToString(), *GetNameSafe( this ) );
    }

    return action;
}