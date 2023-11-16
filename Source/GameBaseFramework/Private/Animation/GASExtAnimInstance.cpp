#include "Animation/GASExtAnimInstance.h"

#include <AbilitySystemGlobals.h>

#if WITH_EDITOR
#include <Misc/DataValidation.h>
#endif

void UGASExtAnimInstance::InitializeWithAbilitySystem( UAbilitySystemComponent * asc )
{
    check( asc );

    GameplayTagPropertyMap.Initialize( this, asc );
}

#if WITH_EDITOR
EDataValidationResult UGASExtAnimInstance::IsDataValid( FDataValidationContext & context ) const
{
    Super::IsDataValid( context );

    GameplayTagPropertyMap.IsDataValid( this, context );

    return ( ( context.GetNumErrors() > 0 ) ? EDataValidationResult::Invalid : EDataValidationResult::Valid );
}
#endif // WITH_EDITOR

void UGASExtAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    if ( const auto * owning_actor = GetOwningActor() )
    {
        if ( auto * asc = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor( owning_actor ) )
        {
            InitializeWithAbilitySystem( asc );
        }
    }
}