#include "Animation/GBFAnimInstance.h"

#include <AbilitySystemGlobals.h>

#if WITH_EDITOR
#include <Misc/DataValidation.h>
#endif

void UGBFAnimInstance::InitializeWithAbilitySystem( UAbilitySystemComponent * asc )
{
    check( asc );

    GameplayTagPropertyMap.Initialize( this, asc );
}

#if WITH_EDITOR
EDataValidationResult UGBFAnimInstance::IsDataValid( FDataValidationContext & context ) const
{
    Super::IsDataValid( context );

    GameplayTagPropertyMap.IsDataValid( this, context );

    return ( ( context.GetNumErrors() > 0 ) ? EDataValidationResult::Invalid : EDataValidationResult::Valid );
}
#endif // WITH_EDITOR

void UGBFAnimInstance::NativeInitializeAnimation()
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