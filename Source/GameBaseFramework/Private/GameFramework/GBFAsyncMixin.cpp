#include "GameFramework/GBFAsyncMixin.h"

#include <Engine/AssetManager.h>
#include <Engine/StreamableManager.h>
#include <Stats/Stats.h>

DEFINE_LOG_CATEGORY_STATIC( LogGBFAsyncMixin, Log, All );

TMap< FGBFAsyncMixin *, TSharedRef< FGBFAsyncMixin::FGBFLoadingState > > FGBFAsyncMixin::Loading;

FGBFAsyncMixin::FGBFAsyncMixin()
{
}

FGBFAsyncMixin::~FGBFAsyncMixin()
{
    check( IsInGameThread() );

    // Removing the loading state will cancel any pending loadings it was
    // monitoring, and shouldn't receive any future callbacks for completion.
    Loading.Remove( this );
}

const FGBFAsyncMixin::FGBFLoadingState & FGBFAsyncMixin::GetLoadingStateConst() const
{
    check( IsInGameThread() );
    return Loading.FindChecked( this ).Get();
}

FGBFAsyncMixin::FGBFLoadingState & FGBFAsyncMixin::GetLoadingState()
{
    check( IsInGameThread() );

    if ( const auto loading_state = Loading.Find( this ) )
    {
        return loading_state->Get();
    }

    return Loading.Add( this, MakeShared< FGBFLoadingState >( *this ) ).Get();
}

bool FGBFAsyncMixin::HasLoadingState() const
{
    check( IsInGameThread() );

    return Loading.Contains( this );
}

void FGBFAsyncMixin::CancelAsyncLoading()
{
    // Don't create the loading state if we don't have anything pending.
    if ( HasLoadingState() )
    {
        GetLoadingState().CancelAndDestroy();
    }
}

bool FGBFAsyncMixin::IsAsyncLoadingInProgress() const
{
    // Don't create the loading state if we don't have anything pending.
    if ( HasLoadingState() )
    {
        return GetLoadingStateConst().IsLoadingInProgress();
    }

    return false;
}

bool FGBFAsyncMixin::IsLoadingInProgressOrPending() const
{
    if ( HasLoadingState() )
    {
        return GetLoadingStateConst().IsLoadingInProgressOrPending();
    }

    return false;
}

void FGBFAsyncMixin::AsyncLoad( const FSoftObjectPath & soft_object_path, const FSimpleDelegate & callback )
{
    GetLoadingState().AsyncLoad( soft_object_path, callback );
}

void FGBFAsyncMixin::AsyncLoad( const TArray< FSoftObjectPath > & soft_object_paths, const FSimpleDelegate & callback )
{
    GetLoadingState().AsyncLoad( soft_object_paths, callback );
}

void FGBFAsyncMixin::AsyncPreloadPrimaryAssetsAndBundles( const TArray< FPrimaryAssetId > & asset_ids, const TArray< FName > & load_bundles, const FSimpleDelegate & callback )
{
    GetLoadingState().AsyncPreloadPrimaryAssetsAndBundles( asset_ids, load_bundles, callback );
}

void FGBFAsyncMixin::AsyncCondition( const TSharedRef< FGBFAsyncCondition > & condition, const FSimpleDelegate & callback )
{
    GetLoadingState().AsyncCondition( condition, callback );
}

void FGBFAsyncMixin::AsyncEvent( const FSimpleDelegate & callback )
{
    GetLoadingState().AsyncEvent( callback );
}

void FGBFAsyncMixin::StartAsyncLoading()
{
    // If we don't actually have any loading state because they've not queued anything to load,
    // just immediately start and finish the operation by calling the callbacks, no point in allocating
    // the memory just to de-allocate it.
    if ( IsLoadingInProgressOrPending() )
    {
        GetLoadingState().Start();
    }
    else
    {
        OnStartedLoading();
        OnFinishedLoading();
    }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

FGBFAsyncMixin::FGBFLoadingState::FGBFLoadingState( FGBFAsyncMixin & owner ) :
    OwnerRef( owner )
{
}

FGBFAsyncMixin::FGBFLoadingState::~FGBFLoadingState()
{
    QUICK_SCOPE_CYCLE_COUNTER( STAT_FAsyncMixin_FLoadingState_DestroyThisMemoryDelegate );
    UE_LOG( LogGBFAsyncMixin, Verbose, TEXT( "[0x%p] Destroy LoadingState (Done)" ), this );

    // If we get destroyed, need to cancel whatever we're doing and cancel any
    // pending destruction - as we're already on the way out.
    CancelOnly( /*bDestroying*/ true );
    CancelDestroyThisMemory( /*bDestroying*/ true );
}

void FGBFAsyncMixin::FGBFLoadingState::CancelOnly( const bool destroying )
{
    if ( !destroying )
    {
        UE_LOG( LogGBFAsyncMixin, Verbose, TEXT( "[0x%p] Cancel" ), this );
    }

    CancelStartTimer();

    for ( const auto & step : AsyncSteps )
    {
        step->Cancel();
    }

    // Moving the memory to another array so we don't crash.
    // There was an issue where the Step would get corrupted because we were calling Reset() on the array.
    AsyncStepsPendingDestruction = MoveTemp( AsyncSteps );

    bPreloadedBundles = false;
    bHasStarted = false;
    CurrentAsyncStep = 0;
}

void FGBFAsyncMixin::FGBFLoadingState::CancelAndDestroy()
{
    CancelOnly( /*bDestroying*/ false );
    RequestDestroyThisMemory();
}

void FGBFAsyncMixin::FGBFLoadingState::CancelDestroyThisMemory( const bool destroying )
{
    // If we've schedule the memory to be deleted we need to abort that.
    if ( IsPendingDestroy() )
    {
        if ( !destroying )
        {
            UE_LOG( LogGBFAsyncMixin, Verbose, TEXT( "[0x%p] Destroy LoadingState (Canceled)" ), this );
        }

        FTSTicker::GetCoreTicker().RemoveTicker( DestroyMemoryDelegate );
        DestroyMemoryDelegate.Reset();
    }
}

void FGBFAsyncMixin::FGBFLoadingState::RequestDestroyThisMemory()
{
    // If we're already pending to destroy this memory, just ignore.
    if ( !IsPendingDestroy() )
    {
        UE_LOG( LogGBFAsyncMixin, Verbose, TEXT( "[0x%p] Destroy LoadingState (Requested)" ), this );

        DestroyMemoryDelegate = FTSTicker::GetCoreTicker().AddTicker( FTickerDelegate::CreateLambda( [ this ]( float delta_time ) {
            // Remove any memory we were using.
            FGBFAsyncMixin::Loading.Remove( &OwnerRef );
            return false;
        } ) );
    }
}

void FGBFAsyncMixin::FGBFLoadingState::CancelStartTimer()
{
    if ( StartTimerDelegate.IsValid() )
    {
        FTSTicker::GetCoreTicker().RemoveTicker( StartTimerDelegate );
        StartTimerDelegate.Reset();
    }
}

void FGBFAsyncMixin::FGBFLoadingState::Start()
{
    UE_LOG( LogGBFAsyncMixin, Verbose, TEXT( "[0x%p] Start (Current Progress %d/%d)" ), this, CurrentAsyncStep + 1, AsyncSteps.Num() );

    // Cancel any pending kickoff load requests.
    CancelStartTimer();

    if ( !bHasStarted )
    {
        bHasStarted = true;
        OwnerRef.OnStartedLoading();
    }

    TryCompleteAsyncLoading();
}

void FGBFAsyncMixin::FGBFLoadingState::AsyncLoad( const FSoftObjectPath & soft_object, const FSimpleDelegate & delegate_to_call )
{
    UE_LOG( LogGBFAsyncMixin, Verbose, TEXT( "[0x%p] AsyncLoad '%s'" ), this, *soft_object.ToString() );

    AsyncSteps.Add(
        MakeUnique< FAsyncStep >(
            delegate_to_call,
            UAssetManager::GetStreamableManager().RequestAsyncLoad( soft_object, FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority, false, false, TEXT( "AsyncMixin" ) ) ) );

    TryScheduleStart();
}

void FGBFAsyncMixin::FGBFLoadingState::AsyncLoad( const TArray< FSoftObjectPath > & soft_object_paths, const FSimpleDelegate & delegate_to_call )
{
    {
        const auto & paths = FString::JoinBy( soft_object_paths, TEXT( ", " ), []( const FSoftObjectPath & soft_object_path ) {
            return FString::Printf( TEXT( "'%s'" ), *soft_object_path.ToString() );
        } );
        UE_LOG( LogGBFAsyncMixin, Verbose, TEXT( "[0x%p] AsyncLoad [%s]" ), this, *paths );
    }

    AsyncSteps.Add(
        MakeUnique< FAsyncStep >(
            delegate_to_call,
            UAssetManager::GetStreamableManager().RequestAsyncLoad( soft_object_paths, FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority, false, false, TEXT( "AsyncMixin" ) ) ) );

    TryScheduleStart();
}

void FGBFAsyncMixin::FGBFLoadingState::AsyncPreloadPrimaryAssetsAndBundles( const TArray< FPrimaryAssetId > & primary_asset_ids, const TArray< FName > & load_bundles, const FSimpleDelegate & delegate_to_call )
{
    {
        const auto & assets = FString::JoinBy( primary_asset_ids, TEXT( ", " ), []( const FPrimaryAssetId & asset_id ) {
            return asset_id.ToString();
        } );
        const auto & bundles = FString::JoinBy( load_bundles, TEXT( ", " ), []( const FName & load_bundle ) {
            return load_bundle.ToString();
        } );
        UE_LOG( LogGBFAsyncMixin, Verbose, TEXT( "[0x%p]  AsyncPreload Assets [%s], Bundles[%s]" ), this, *assets, *bundles );
    }

    TSharedPtr< FStreamableHandle > streaming_handle;

    if ( primary_asset_ids.Num() > 0 )
    {
        bPreloadedBundles = true;

        constexpr bool load_recursive = true;
        streaming_handle = UAssetManager::Get().PreloadPrimaryAssets( primary_asset_ids, load_bundles, load_recursive );
    }

    AsyncSteps.Add( MakeUnique< FAsyncStep >( delegate_to_call, streaming_handle ) );

    TryScheduleStart();
}

void FGBFAsyncMixin::FGBFLoadingState::AsyncCondition( TSharedRef< FGBFAsyncCondition > condition, const FSimpleDelegate & callback )
{
    UE_LOG( LogGBFAsyncMixin, Verbose, TEXT( "[0x%p] AsyncCondition '0x%p'" ), this, &condition.Get() );

    AsyncSteps.Add( MakeUnique< FAsyncStep >( callback, condition ) );

    TryScheduleStart();
}

void FGBFAsyncMixin::FGBFLoadingState::AsyncEvent( const FSimpleDelegate & callback )
{
    UE_LOG( LogGBFAsyncMixin, Verbose, TEXT( "[0x%p] AsyncEvent" ), this );

    AsyncSteps.Add( MakeUnique< FAsyncStep >( callback ) );

    TryScheduleStart();
}

void FGBFAsyncMixin::FGBFLoadingState::TryScheduleStart()
{
    CancelDestroyThisMemory( /*bDestroying*/ false );

    // In the event the user forgets to start async loading, we'll begin doing it next frame.
    if ( !StartTimerDelegate.IsValid() )
    {
        StartTimerDelegate = FTSTicker::GetCoreTicker().AddTicker( FTickerDelegate::CreateLambda( [ this ]( float delta_time ) {
            QUICK_SCOPE_CYCLE_COUNTER( STAT_FAsyncMixin_FLoadingState_TryScheduleStartDelegate );
            Start();
            return false;
        } ) );
    }
}

bool FGBFAsyncMixin::FGBFLoadingState::IsLoadingInProgress() const
{
    if ( AsyncSteps.Num() > 0 )
    {
        if ( CurrentAsyncStep < AsyncSteps.Num() )
        {
            if ( CurrentAsyncStep == ( AsyncSteps.Num() - 1 ) )
            {
                return AsyncSteps[ CurrentAsyncStep ]->IsLoadingInProgress();
            }

            // If we know it's a valid index, but not the last one, then we know we're still loading,
            // if it's not a valid index, we know there's no loading, or we're beyond any loading.
            return true;
        }
    }

    return false;
}

bool FGBFAsyncMixin::FGBFLoadingState::IsLoadingInProgressOrPending() const
{
    return StartTimerDelegate.IsValid() || IsLoadingInProgress();
}

bool FGBFAsyncMixin::FGBFLoadingState::IsPendingDestroy() const
{
    return DestroyMemoryDelegate.IsValid();
}

void FGBFAsyncMixin::FGBFLoadingState::TryCompleteAsyncLoading()
{
    // If we haven't started when we get this callback it means we've already completed
    // and this is some other callback finishing on the same frame/stack that we need to avoid
    // doing anything with until the memory is finished being deleted.
    if ( !bHasStarted )
    {
        return;
    }

    UE_LOG( LogGBFAsyncMixin, Verbose, TEXT( "[0x%p] TryCompleteAsyncLoading - (Current Progress %d/%d)" ), this, CurrentAsyncStep + 1, AsyncSteps.Num() );

    while ( CurrentAsyncStep < AsyncSteps.Num() )
    {
        auto * step = AsyncSteps[ CurrentAsyncStep ].Get();
        if ( step->IsLoadingInProgress() )
        {
            if ( !step->IsCompleteDelegateBound() )
            {
                UE_LOG( LogGBFAsyncMixin, Verbose, TEXT( "[0x%p] Step %d - Still Loading (Listening)" ), this, CurrentAsyncStep + 1 );
                const auto bound = step->BindCompleteDelegate( FSimpleDelegate::CreateSP( this, &FGBFLoadingState::TryCompleteAsyncLoading ) );
                ensureMsgf( bound, TEXT( "This is not intended to return false.  We're checking if it's loaded above, this should definitely return true." ) );
            }
            else
            {
                UE_LOG( LogGBFAsyncMixin, Verbose, TEXT( "[0x%p] Step %d - Still Loading (Waiting)" ), this, CurrentAsyncStep + 1 );
            }

            break;
        }
        else
        {
            UE_LOG( LogGBFAsyncMixin, Verbose, TEXT( "[0x%p] Step %d - Completed (Calling User)" ), this, CurrentAsyncStep + 1 );

            // Always advance the CurrentAsyncStep, before calling the user callback, it's possible they might
            // add new work, and try and start again, so we need to be ready for the next bit.
            CurrentAsyncStep++;

            step->ExecuteUserCallback();
        }
    }

    // If we're done loading, and bHasStarted is still true (meaning this is the first time we're encountering a request to complete)
    // try and complete.  It's entirely possible that a user callback might append new work, which they immediately start, which
    // immediately tries to complete, which might create a case where we're now inside of TryCompleteAsyncLoading, which then
    // calls Start, which then calls TryCompleteAsyncLoading, so when we come back out of the stack, we need to avoid trying to
    // complete the async loading N+ times.
    if ( IsLoadingComplete() && bHasStarted )
    {
        CompleteAsyncLoading();
    }
}

void FGBFAsyncMixin::FGBFLoadingState::CompleteAsyncLoading()
{
    UE_LOG( LogGBFAsyncMixin, Verbose, TEXT( "[0x%p] CompleteAsyncLoading" ), this );

    // Mark that we've completed loading.
    if ( bHasStarted )
    {
        bHasStarted = false;
        OwnerRef.OnFinishedLoading();
    }

    // It's unlikely but possible they started loading more stuff in the OnFinishedLoading callback,
    // so double check that we're still actually done.
    //
    // NOTE: We don't delete ourselves from memory in use.  Doing things like
    // pre-loading a bundle requires keeping the streaming handle alive.  So we're keeping
    // things alive.
    //
    // We won't destroy the memory but we need to cleanup anything that may be hanging on to
    // captured scope, like completion handlers.
    if ( IsLoadingComplete() )
    {
        if ( !bPreloadedBundles && !IsLoadingInProgressOrPending() )
        {
            // If we're all done loading or pending loading, we should clean up the memory we're using.
            // go ahead and remove this loading state the owner mix-in allocated.
            RequestDestroyThisMemory();
            return;
        }
    }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

FGBFAsyncMixin::FGBFLoadingState::FAsyncStep::FAsyncStep( const FSimpleDelegate & user_callback ) :
    UserCallback( user_callback )
{
}

FGBFAsyncMixin::FGBFLoadingState::FAsyncStep::FAsyncStep( const FSimpleDelegate & user_callback, const TSharedPtr< FStreamableHandle > & streaming_handle ) :
    UserCallback( user_callback ),
    StreamingHandle( streaming_handle )
{
}

FGBFAsyncMixin::FGBFLoadingState::FAsyncStep::FAsyncStep( const FSimpleDelegate & user_callback, const TSharedPtr< FGBFAsyncCondition > & condition ) :
    UserCallback( user_callback ),
    Condition( condition )
{
}

FGBFAsyncMixin::FGBFLoadingState::FAsyncStep::~FAsyncStep()
{
}

void FGBFAsyncMixin::FGBFLoadingState::FAsyncStep::ExecuteUserCallback()
{
    UserCallback.ExecuteIfBound();
    UserCallback.Unbind();
}

bool FGBFAsyncMixin::FGBFLoadingState::FAsyncStep::IsComplete() const
{
    if ( StreamingHandle.IsValid() )
    {
        return StreamingHandle->HasLoadCompleted();
    }

    if ( Condition.IsValid() )
    {
        return Condition->IsComplete();
    }

    return true;
}

void FGBFAsyncMixin::FGBFLoadingState::FAsyncStep::Cancel()
{
    if ( StreamingHandle.IsValid() )
    {
        StreamingHandle->BindCompleteDelegate( FSimpleDelegate() );
        StreamingHandle.Reset();
    }
    else if ( Condition.IsValid() )
    {
        Condition.Reset();
    }

    bIsCompletionDelegateBound = false;
}

bool FGBFAsyncMixin::FGBFLoadingState::FAsyncStep::BindCompleteDelegate( const FSimpleDelegate & new_delegate )
{
    if ( IsComplete() )
    {
        // Too Late!
        return false;
    }

    if ( StreamingHandle.IsValid() )
    {
        StreamingHandle->BindCompleteDelegate( new_delegate );
    }
    else if ( Condition )
    {
        Condition->BindCompleteDelegate( new_delegate );
    }

    bIsCompletionDelegateBound = true;

    return true;
}

bool FGBFAsyncMixin::FGBFLoadingState::FAsyncStep::IsCompleteDelegateBound() const
{
    return bIsCompletionDelegateBound;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

FGBFAsyncCondition::FGBFAsyncCondition( const FGBFAsyncConditionDelegate & condition ) :
    UserCondition( condition )
{
}

FGBFAsyncCondition::FGBFAsyncCondition( TFunction< EGBFAsyncConditionResult() > && condition ) :
    UserCondition( FGBFAsyncConditionDelegate::CreateLambda( [ UserFunction = MoveTemp( condition ) ]() mutable {
        return UserFunction();
    } ) )
{
}

FGBFAsyncCondition::~FGBFAsyncCondition()
{
    FTSTicker::GetCoreTicker().RemoveTicker( RepeatHandle );
}

bool FGBFAsyncCondition::IsComplete() const
{
    if ( UserCondition.IsBound() )
    {
        const auto result = UserCondition.Execute();
        return result == EGBFAsyncConditionResult::Complete;
    }

    return true;
}

bool FGBFAsyncCondition::BindCompleteDelegate( const FSimpleDelegate & new_delegate )
{
    if ( IsComplete() )
    {
        // Already Complete
        return false;
    }

    CompletionDelegate = new_delegate;

    if ( !RepeatHandle.IsValid() )
    {
        RepeatHandle = FTSTicker::GetCoreTicker().AddTicker( FTickerDelegate::CreateSP( this, &FGBFAsyncCondition::TryToContinue ), 0.16 );
    }

    return true;
}

bool FGBFAsyncCondition::TryToContinue( float )
{
    QUICK_SCOPE_CYCLE_COUNTER( STAT_FAsyncCondition_TryToContinue );

    UE_LOG( LogGBFAsyncMixin, Verbose, TEXT( "[0x%p] AsyncCondition::TryToContinue" ), this );

    if ( UserCondition.IsBound() )
    {
        const auto result = UserCondition.Execute();

        switch ( result )
        {
            case EGBFAsyncConditionResult::TryAgain:
                return true;
            case EGBFAsyncConditionResult::Complete:
                RepeatHandle.Reset();
                UserCondition.Unbind();

                CompletionDelegate.ExecuteIfBound();
                CompletionDelegate.Unbind();
                break;
        }
    }

    return false;
}
