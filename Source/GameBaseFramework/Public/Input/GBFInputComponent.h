#pragma once

#include "GBFInputConfig.h"

#include <CoreMinimal.h>
#include <EnhancedInputComponent.h>

#include "GBFInputComponent.generated.h"

class UEnhancedInputLocalPlayerSubsystem;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFInputComponent : public UEnhancedInputComponent
{
    GENERATED_BODY()

public:
    explicit UGBFInputComponent( const FObjectInitializer & object_initializer );

    void AddInputMappings( const UGBFInputConfig * input_config, UEnhancedInputLocalPlayerSubsystem * input_system ) const;
    void RemoveInputMappings( const UGBFInputConfig * input_config, UEnhancedInputLocalPlayerSubsystem * input_system ) const;

    template < class _USER_CLASS_, typename _FUNC_TYPE_ >
    void BindNativeAction( const UGBFInputConfig * input_config, const FGameplayTag & input_tag, ETriggerEvent trigger_event, _USER_CLASS_ * object, _FUNC_TYPE_ func, bool log_if_not_found );

    template < class _USER_CLASS_, typename _PRESSED_FUNC_, typename _RELEASED_FUNC_ >
    void BindAbilityActions( const UGBFInputConfig * input_config, _USER_CLASS_ * object, _PRESSED_FUNC_ pressed_func, _RELEASED_FUNC_ released_func, TArray< uint32 > & bind_handles );

    template < class _USER_CLASS_, typename _PRESSED_FUNC_, typename _RELEASED_FUNC_ >
    void BindAbilityAction( const UInputAction * input_action, _USER_CLASS_ * object, _PRESSED_FUNC_ pressed_func, _RELEASED_FUNC_ released_func, TArray< uint32 > & bind_handles );

    void RemoveBinds( TArray< uint32 > & bind_handles );
};

template < class _USER_CLASS_, typename _FUNC_TYPE_ >
void UGBFInputComponent::BindNativeAction( const UGBFInputConfig * input_config, const FGameplayTag & input_tag, ETriggerEvent trigger_event, _USER_CLASS_ * object, _FUNC_TYPE_ func, bool log_if_not_found )
{
    check( input_config != nullptr );
    if ( const auto * input_action = input_config->FindNativeInputActionForTag( input_tag, log_if_not_found ) )
    {
        BindAction( input_action, trigger_event, object, func );
    }
}

template < class _USER_CLASS_, typename _PRESSED_FUNC_, typename _RELEASED_FUNC_ >
void UGBFInputComponent::BindAbilityActions( const UGBFInputConfig * input_config, _USER_CLASS_ * object, _PRESSED_FUNC_ pressed_func, _RELEASED_FUNC_ released_func, TArray< uint32 > & bind_handles )
{
    check( input_config != nullptr );

    for ( const auto & [ input_action, input_tag ] : input_config->AbilityInputActions )
    {
        if ( input_action && input_tag.IsValid() )
        {
            if ( pressed_func )
            {
                bind_handles.Add( BindAction( input_action, ETriggerEvent::Triggered, object, pressed_func, input_tag ).GetHandle() );
            }

            if ( released_func )
            {
                bind_handles.Add( BindAction( input_action, ETriggerEvent::Completed, object, released_func, input_tag ).GetHandle() );
            }
        }
    }
}

template < class _USER_CLASS_, typename _PRESSED_FUNC_, typename _RELEASED_FUNC_ >
void UGBFInputComponent::BindAbilityAction( const UInputAction * input_action, _USER_CLASS_ * object, _PRESSED_FUNC_ pressed_func, _RELEASED_FUNC_ released_func, TArray< uint32 > & bind_handles )
{
    if ( input_action == nullptr )
    {
        return;
    }

    if ( pressed_func )
    {
        bind_handles.Add( BindAction( input_action, ETriggerEvent::Triggered, object, pressed_func ).GetHandle() );
    }

    if ( released_func )
    {
        bind_handles.Add( BindAction( input_action, ETriggerEvent::Completed, object, released_func ).GetHandle() );
    }
}