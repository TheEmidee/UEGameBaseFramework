#include "GAS/Targeting/GASExtGameplayAbilityTargetActor.h"

#include "GAS/Targeting/GASExtTargetingHelperLibrary.h"

#include <AbilitySystemComponent.h>
#include <AbilitySystemLog.h>
#include <DrawDebugHelpers.h>
#include <GameFramework/PlayerController.h>

AGASExtGameplayAbilityTargetActor::AGASExtGameplayAbilityTargetActor()
{
    PrimaryActorTick.bStartWithTickEnabled = false;
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostUpdateWork;

    bDestroyOnConfirmation = false;
    bTraceComplex = false;
    bIgnoreBlockingHits = false;
    TraceMaxRange = 9999999.0f;
    MaxHitResults = 1;
    bAllowEmptyHitResult = false;
    bUsePersistentHitResults = true;
    bDrawDebug = false;
    CollisionHeightOffset = 0.0f;
}

void AGASExtGameplayAbilityTargetActor::K2_ConfirmTargeting()
{
    ConfirmTargeting();
}

void AGASExtGameplayAbilityTargetActor::StopTargeting()
{
    SetActorTickEnabled( false );
    ClearPersistentHitResults();

    TargetDataReadyDelegate.Clear();
    CanceledDelegate.Clear();

    if ( GenericDelegateBoundASC )
    {
        GenericDelegateBoundASC->GenericLocalConfirmCallbacks.RemoveDynamic( this, &AGameplayAbilityTargetActor::ConfirmTargeting );
        GenericDelegateBoundASC->GenericLocalCancelCallbacks.RemoveDynamic( this, &AGameplayAbilityTargetActor::CancelTargeting );
        GenericDelegateBoundASC = nullptr;
    }
}

void AGASExtGameplayAbilityTargetActor::BP_CancelTargeting()
{
    CancelTargeting();
}

void AGASExtGameplayAbilityTargetActor::BeginPlay()
{
    Super::BeginPlay();

    SetActorHiddenInGame( true );
}

void AGASExtGameplayAbilityTargetActor::StartTargeting( UGameplayAbility * ability )
{
    Super::StartTargeting( ability );

    PrimaryActorTick.SetTickFunctionEnable( true );
    SetActorHiddenInGame( false );

    ClearPersistentHitResults();

    SourceActor = GetSourceActor();

    checkf( SourceActor != nullptr, TEXT( "SourceActor must be set" ) );
}

void AGASExtGameplayAbilityTargetActor::Tick( const float delta_seconds )
{
    Super::Tick( delta_seconds );
    PerformTrace( delta_seconds );
}

void AGASExtGameplayAbilityTargetActor::CancelTargeting()
{
    const auto * actor_info = ( OwningAbility ? OwningAbility->GetCurrentActorInfo() : nullptr );
    auto * asc = ( actor_info ? actor_info->AbilitySystemComponent.Get() : nullptr );
    if ( asc )
    {
        asc->AbilityReplicatedEventDelegate( EAbilityGenericReplicatedEvent::GenericCancel, OwningAbility->GetCurrentAbilitySpecHandle(), OwningAbility->GetCurrentActivationInfo().GetActivationPredictionKey() ).Remove( GenericCancelHandle );
    }
    else
    {
        ABILITY_LOG( Warning, TEXT( "AGameplayAbilityTargetActor::CancelTargeting called with null ASC! Actor %s" ), *GetName() );
    }

    CanceledDelegate.Broadcast( FGameplayAbilityTargetDataHandle() );

    SetActorTickEnabled( false );
    SetActorHiddenInGame( true );

    ClearPersistentHitResults();
}

void AGASExtGameplayAbilityTargetActor::EndPlay( const EEndPlayReason::Type end_play_reason )
{
    ClearPersistentHitResults();
    Super::EndPlay( end_play_reason );
}

void AGASExtGameplayAbilityTargetActor::ConfirmTargetingAndContinue()
{
    check( ShouldProduceTargetData() );
    const auto handle = UGASExtTargetingHelperLibrary::MakeTargetDataFromHitResults( PersistentHitResults );
    TargetDataReadyDelegate.Broadcast( handle );
    SetActorHiddenInGame( true );
}

void AGASExtGameplayAbilityTargetActor::ComputeTrace( FVector & trace_start, FVector & trace_end )
{
    UGASExtTargetingHelperLibrary::AimWithPlayerController( trace_start, trace_end, FSWAimInfos( OwningAbility, StartLocation, TraceMaxRange ) );
}

void AGASExtGameplayAbilityTargetActor::FillActorsToIgnore( TArray< AActor * > actors_to_ignore ) const
{
    actors_to_ignore.Add( SourceActor );
    actors_to_ignore.Add( OwningAbility->GetCurrentActorInfo()->AvatarActor.Get() );
    actors_to_ignore.Add( const_cast< AGASExtGameplayAbilityTargetActor * >( this ) );
}

void AGASExtGameplayAbilityTargetActor::PerformTrace( const float delta_seconds )
{
    TArray< AActor * > actors_to_ignore;
    FillActorsToIgnore( actors_to_ignore );

    FCollisionQueryParams collision_query_params( SCENE_QUERY_STAT( AGASExtGameplayAbilityTargetActor_Trace ), bTraceComplex );
    collision_query_params.bReturnPhysicalMaterial = true;
    collision_query_params.AddIgnoredActors( actors_to_ignore );
    collision_query_params.bIgnoreBlocks = bIgnoreBlockingHits;

    FVector trace_start;
    FVector trace_end;

    ComputeTrace( trace_start, trace_end );

    for ( auto index = PersistentHitResults.Num() - 1; index >= 0; --index )
    {
        auto & hit_result = PersistentHitResults[ index ];

        if ( /*hit_result.bBlockingHit ||*/ !hit_result.HasValidHitObjectHandle() || FVector::DistSquared( trace_start, hit_result.GetActor()->GetActorLocation() ) > FMath::Square( TraceMaxRange ) )
        {
            RemoveTargetFromPersistentResult( index );
        }
    }

    TArray< FHitResult > hit_results;

    switch ( TraceType )
    {
        case EGASExtTargetTraceType::Line:
        {
            UGASExtTargetingHelperLibrary::LineTraceWithFilter( hit_results, GetWorld(), TargetDataFilterHandle, trace_start, trace_end, CollisionInfo, collision_query_params );
        }
        break;
        case EGASExtTargetTraceType::Sphere:
        {
            UGASExtTargetingHelperLibrary::SphereTraceWithFilter( hit_results, GetWorld(), TargetDataFilterHandle, trace_start, trace_end, TraceSphereRadius, CollisionInfo, collision_query_params );
        }
        break;
        default:
        {
            checkNoEntry();
        }
        break;
    }

#if ENABLE_DRAW_DEBUG
    if ( bDrawDebug )
    {
        auto has_hit_results = hit_results.Num() > 0;

        switch ( TraceType )
        {
            case EGASExtTargetTraceType::Line:
            {
                // UGBFTraceBlueprintLibrary::DrawDebugLineTraceMulti( GetWorld(), trace_start, trace_end, EDrawDebugTrace::ForOneFrame, has_hit_results, hit_results, FLinearColor::Red, FLinearColor::Green, 0.1f );
            }
            break;
            case EGASExtTargetTraceType::Sphere:
            {
                // UGBFTraceBlueprintLibrary::DrawDebugSphereTraceMulti( GetWorld(), trace_start, trace_end, TraceSphereRadius, EDrawDebugTrace::ForOneFrame, has_hit_results, hit_results, FLinearColor::Red, FLinearColor::Green, 0.1f );
            }
            break;
            default:
            {
                checkNoEntry();
            }
            break;
        }
    }
#endif

    if ( hit_results.Num() == 0 )
    {
        if ( bAllowEmptyHitResult )
        {
            hit_results.Emplace( FHitResult { trace_start, trace_end } );
            for ( const auto & hit_result : hit_results )
            {
                ClearPersistentHitResults();
                AddTargetToPersistentResult( hit_result );
            }
        }
    }
    else
    {
        if ( bUsePersistentHitResults )
        {
            // For persistent hits, loop backwards so that further objects from player are added first to the queue.
            // This results in closer actors taking precedence as the further actors will get bumped out of the TArray.
            for ( auto index = hit_results.Num() - 1; index >= 0; index-- )
            {
                const auto & hit_result = hit_results[ index ];
                if ( hit_result.HasValidHitObjectHandle() )
                {
                    if ( PersistentHitResults.FindByPredicate( [ &hit_result ]( const FHitResult & other_hit_result ) {
                             return other_hit_result.GetActor() == hit_result.GetActor();
                         } ) != nullptr )
                    {
                        continue;
                    }

                    if ( PersistentHitResults.Num() >= MaxHitResults )
                    {
                        RemoveTargetFromPersistentResult( 0 );
                    }

                    AddTargetToPersistentResult( hit_result );
                }
            }
        }
        else
        {
            // Loop forward and use the first valid hit
            for ( const auto & hit_result : hit_results )
            {
                if ( hit_result.HasValidHitObjectHandle() || bAllowEmptyHitResult )
                {
                    ClearPersistentHitResults();
                    AddTargetToPersistentResult( hit_result );
                    SetActorLocation( hit_result.Location );

                    break;
                }
            }
        }
    }
}

AActor * AGASExtGameplayAbilityTargetActor::GetSourceActor()
{
    return OwningAbility->GetCurrentActorInfo()->AvatarActor.Get();
}

void AGASExtGameplayAbilityTargetActor::ClearPersistentHitResults()
{
    PersistentHitResults.Empty();
    ReceiveDestroyAllReticles();
}

void AGASExtGameplayAbilityTargetActor::RemoveTargetFromPersistentResult( int target_index )
{
    ReceiveTargetRemoved( PersistentHitResults[ target_index ] );
    PersistentHitResults.RemoveAt( target_index );
}

void AGASExtGameplayAbilityTargetActor::AddTargetToPersistentResult( const FHitResult & hit_result )
{
    PersistentHitResults.Emplace( hit_result );
    ReceiveTargetAdded( hit_result );
}
