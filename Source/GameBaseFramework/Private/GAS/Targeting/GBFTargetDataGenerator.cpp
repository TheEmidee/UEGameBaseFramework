#include "GAS/Targeting/GBFTargetDataGenerator.h"

#include <Abilities/GameplayAbility.h>
#include <Abilities/GameplayAbilityTypes.h>
#include <AbilitySystemBlueprintLibrary.h>
#include <Kismet/KismetSystemLibrary.h>

namespace
{
    void AddActorsFromSource( const EGBFTargetDataGeneratorActorSource source, const FGameplayEffectContext * gameplay_effect_context, TFunctionRef< void( AActor * ) > functor )
    {
        if ( ( source & EGBFTargetDataGeneratorActorSource::AbilityAvatar ) == EGBFTargetDataGeneratorActorSource::AbilityAvatar )
        {
            if ( auto * ability = gameplay_effect_context->GetAbilityInstance_NotReplicated() )
            {
                if ( auto * current_actor_info = ability->GetCurrentActorInfo() )
                {
                    functor( current_actor_info->AvatarActor.Get() );
                }
            }
        }
        if ( ( source & EGBFTargetDataGeneratorActorSource::EffectCauser ) == EGBFTargetDataGeneratorActorSource::EffectCauser )
        {
            functor( gameplay_effect_context->GetEffectCauser() );
        }
        if ( ( source & EGBFTargetDataGeneratorActorSource::Instigator ) == EGBFTargetDataGeneratorActorSource::Instigator )
        {
            functor( gameplay_effect_context->GetInstigator() );
        }
        if ( ( source & EGBFTargetDataGeneratorActorSource::OriginalInstigator ) == EGBFTargetDataGeneratorActorSource::OriginalInstigator )
        {
            functor( gameplay_effect_context->GetOriginalInstigator() );
        }
        if ( ( source & EGBFTargetDataGeneratorActorSource::SourceObject ) == EGBFTargetDataGeneratorActorSource::SourceObject )
        {
            if ( auto * actor = Cast< AActor >( gameplay_effect_context->GetSourceObject() ) )
            {
                functor( actor );
            }
        }
        if ( ( source & EGBFTargetDataGeneratorActorSource::HitResult ) == EGBFTargetDataGeneratorActorSource::HitResult )
        {
            if ( const auto * hit_result = gameplay_effect_context->GetHitResult() )
            {
                if ( auto * actor = hit_result->GetActor() )
                {
                    functor( actor );
                }
            }
        }
    }
}

bool UGBFTargetDataGenerator::IsSupportedForNetworking() const
{
    return true;
}
FGameplayAbilityTargetDataHandle UGBFTargetDataGenerator_EventData::GetTargetData( const FGameplayEffectContext * /*gameplay_effect_context*/, const FGameplayEventData & event_data ) const
{
    return FGameplayAbilityTargetDataHandle( event_data.TargetData );
}

FGameplayAbilityTargetDataHandle UGBFTargetDataGenerator_GetActor::GetTargetData( const FGameplayEffectContext * gameplay_effect_context, const FGameplayEventData & /*event_data*/ ) const
{
    auto * new_data = new FGameplayAbilityTargetData_ActorArray();

    AddActorsFromSource( static_cast< EGBFTargetDataGeneratorActorSource >( Source ), gameplay_effect_context, [ new_data ]( AActor * actor ) {
        new_data->TargetActorArray.Add( actor );
    } );

    return FGameplayAbilityTargetDataHandle( new_data );
}

UGBFTargetDataGenerator_SphereOverlap::UGBFTargetDataGenerator_SphereOverlap()
{
    Source = EGBFTargetDataGeneratorActorSource::EffectCauser;
    SphereRadius = 1.0f;
    ObjectTypes.Add( UEngineTypes::ConvertToObjectType( ECC_Pawn ) );
    ObjectTypes.Add( UEngineTypes::ConvertToObjectType( ECC_Destructible ) );
    bDrawsDebug = false;
    bMustHaveLineOfSight = true;
    DrawDebugDuration = 2.0f;
    SphereCenterOffset = FVector::ZeroVector;
}

FGameplayAbilityTargetDataHandle UGBFTargetDataGenerator_SphereOverlap::GetTargetData( const FGameplayEffectContext * gameplay_effect_context, const FGameplayEventData & event_data ) const
{
    FGameplayAbilityTargetDataHandle result;

    const auto source_location = GetSourceLocation( gameplay_effect_context );

    if ( source_location.IsSet() )
    {
        TArray< AActor * > hit_actors;

        const auto * world_context = gameplay_effect_context->GetInstigator();
        const auto sphere_center = source_location.GetValue() + SphereCenterOffset;

        TArray< AActor * > actors_to_ignore;
        AddActorsFromSource( static_cast< EGBFTargetDataGeneratorActorSource >( ActorsToIgnoreDuringSphereOverlap ), gameplay_effect_context, [ &actors_to_ignore ]( AActor * actor ) {
            actors_to_ignore.Add( actor );
        } );

        UKismetSystemLibrary::SphereOverlapActors( world_context, sphere_center, SphereRadius.GetValue(), ObjectTypes, nullptr, actors_to_ignore, hit_actors );

        if ( bDrawsDebug )
        {
            UKismetSystemLibrary::DrawDebugSphere( world_context, sphere_center, SphereRadius.GetValue(), 12, FLinearColor::Red, DrawDebugDuration, 1.0f );
        }

        if ( bMustHaveLineOfSight )
        {
            const auto hit_actors_copy = hit_actors;
            for ( auto index = hit_actors.Num() - 1; index >= 0; --index )
            {
                auto * hit_actor = hit_actors[ index ];

                auto ignore_actors = hit_actors_copy;
                ignore_actors.RemoveSwap( hit_actor );

                FHitResult line_trace_hit;
                UKismetSystemLibrary::LineTraceSingle( world_context,
                    sphere_center,
                    hit_actor->GetActorLocation(),
                    UEngineTypes::ConvertToTraceType( ECC_Visibility ),
                    false,
                    ignore_actors,
                    bDrawsDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
                    line_trace_hit,
                    true,
                    FLinearColor::Red,
                    FLinearColor::Green,
                    DrawDebugDuration );

                if ( line_trace_hit.bBlockingHit && line_trace_hit.GetActor() != hit_actor )
                {
                    hit_actors.RemoveAtSwap( index );
                }
            }
        }

        auto * new_data = new FGameplayAbilityTargetData_ActorArray();
        new_data->TargetActorArray.Append( hit_actors );
        result.Add( new_data );
    }

    return result;
}

TOptional< FVector > UGBFTargetDataGenerator_SphereOverlap::GetSourceLocation( const FGameplayEffectContext * gameplay_effect_context ) const
{
    TOptional< FVector > source_location;

    const auto set_source_location_from_actor = [ &source_location ]( const AActor * actor ) {
        if ( IsValid( actor ) )
        {
            source_location = actor->GetActorLocation();
        }
    };

    switch ( static_cast< EGBFTargetDataGeneratorActorSource >( Source ) )
    {
        case EGBFTargetDataGeneratorActorSource::AbilityAvatar:
        {
            if ( auto * ability = gameplay_effect_context->GetAbilityInstance_NotReplicated() )
            {
                if ( auto * current_actor_info = ability->GetCurrentActorInfo() )
                {
                    set_source_location_from_actor( current_actor_info->AvatarActor.Get() );
                }
            }
        }
        break;
        case EGBFTargetDataGeneratorActorSource::EffectCauser:
        {
            set_source_location_from_actor( gameplay_effect_context->GetEffectCauser() );
        }
        break;
        case EGBFTargetDataGeneratorActorSource::Instigator:
        {
            set_source_location_from_actor( gameplay_effect_context->GetInstigator() );
        }
        break;
        case EGBFTargetDataGeneratorActorSource::OriginalInstigator:
        {
            set_source_location_from_actor( gameplay_effect_context->GetOriginalInstigator() );
        }
        break;
        case EGBFTargetDataGeneratorActorSource::SourceObject:
        {
            set_source_location_from_actor( Cast< AActor >( gameplay_effect_context->GetSourceObject() ) );
        }
        break;
        case EGBFTargetDataGeneratorActorSource::HitResult:
        {
            if ( const auto * hit_result = gameplay_effect_context->GetHitResult() )
            {
                source_location = hit_result->ImpactPoint;
            }
        }
        break;
        default:
        {
            check( false );
        };
    }

    return source_location;
}