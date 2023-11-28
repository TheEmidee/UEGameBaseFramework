#pragma once

#include <Containers/Ticker.h>
#include <UObject/SoftObjectPtr.h>

class FGBFAsyncCondition;
class FName;
class UPrimaryDataAsset;
struct FPrimaryAssetId;
struct FStreamableHandle;
template < class TClass >
class TSubclassOf;

DECLARE_DELEGATE_OneParam( FGBFStreamableHandleDelegate, TSharedPtr< FStreamableHandle > );

// TODO I think we need to introduce a retention policy, preloads automatically stay in memory until canceled
//      but what if you want to preload individual items just using the AsyncLoad functions?  I don't want to
//      introduce individual policies per call, or introduce a whole set of preload vs asyncloads, so would
//      would rather have a retention policy.  Should it be a member and actually create real memory when
//      you inherit from AsyncMixin, or should it be a template argument?
// enum class EAsyncMixinRetentionPolicy : uint8
//{
//	Default,
//	KeepResidentUntilComplete,
//	KeepResidentUntilCancel
// };

/**
 * The FGBFAsyncMixin allows easier management of async loading requests, to ensure linear request handling, to make
 * writing code much easier.  The usage pattern is as follows,
 *
 * First - inherit from FGBFAsyncMixin, even if you're a UObject, you can also inherit from FGBFAsyncMixin.
 *
 * Then - you can make your async loads as follows.
 *
 * CancelAsyncLoading();			// Some objects get reused like in lists, so it's important to cancel anything you had pending doesn't complete.
 * AsyncLoad(ItemOne, CallbackOne);
 * AsyncLoad(ItemTwo, CallbackTwo);
 * StartAsyncLoading();
 *
 * You can also include the 'this' scope safely, one of the benefits of the mix-in, is that none of the callbacks
 * are ever out of scope of the host AsyncMixin derived object.
 * e.g.
 * AsyncLoad(SomeSoftObjectPtr, [this, ...]() {
 *
 * });
 *
 *
 * What will happen is first we cancel any existing one(s), e.g. perhaps we are a widget that just got told to represent
 * some new thing.  What will happen is we'll Load ItemOne and ItemTwo, *THEN* we'll call the callbacks in the order you
 * requested the async loads - even if ItemOne or ItemTwo was already loaded when you request it.
 *
 * When all the async loading requests complete, OnFinishedLoading will be called.
 *
 * If you forget to call StartAsyncLoading(), we'll call it next frame, but you should remember to call it
 * when you're done with your setup, as maybe everything is already loaded, and it will avoid a single frame
 * of a loading indicator flash, which is annoying.
 *
 * NOTE: The FGBFAsyncMixin also makes it safe to pass [this] as a captured input into your lambda, because it handles
 * unhooking everything if either your owner class is destroyed, or you cancel everything.
 *
 * NOTE: FGBFAsyncMixin doesn't add any additional memory to your class.  Several classes currently handling async loading
 * internally allocate TSharedPtr<FStreamableHandle> members and tend to hold onto SoftObjectPaths temporary state.  The
 * FGBFAsyncMixin does all of this internally with a static TMap so that all of the async request memory is stored temporarily
 * and sparsely.
 *
 * NOTE: For debugging and understanding what's going on, you should add -LogCmds="LogAsyncMixin Verbose" to the command line.
 */

class GAMEBASEFRAMEWORK_API FGBFAsyncMixin : public FNoncopyable
{
protected:
    FGBFAsyncMixin();

public:
    virtual ~FGBFAsyncMixin();

protected:
    /** Called when loading starts. */
    virtual void OnStartedLoading()
    {}
    /** Called when all loading has finished. */
    virtual void OnFinishedLoading()
    {}

protected:
    /** Async load a TSoftClassPtr<T>, call the Callback when complete. */
    template < typename T = UObject >
    void AsyncLoad( TSoftClassPtr< T > soft_class, TFunction< void() > && callback )
    {
        AsyncLoad( soft_class.ToSoftObjectPath(), FSimpleDelegate::CreateLambda( MoveTemp( callback ) ) );
    }

    /** Async load a TSoftClassPtr<T>, call the Callback when complete. */
    template < typename T = UObject >
    void AsyncLoad( TSoftClassPtr< T > soft_class, TFunction< void( TSubclassOf< T > ) > && callback )
    {
        AsyncLoad( soft_class.ToSoftObjectPath(),
            FSimpleDelegate::CreateLambda( [ soft_class, UserCallback = MoveTemp( callback ) ]() mutable {
                UserCallback( soft_class.Get() );
            } ) );
    }

    /** Async load a TSoftClassPtr<T>, call the Callback when complete. */
    template < typename T = UObject >
    void AsyncLoad( TSoftClassPtr< T > soft_class, const FSimpleDelegate & callback = FSimpleDelegate() )
    {
        AsyncLoad( soft_class.ToSoftObjectPath(), callback );
    }

    /** Async load a TSoftObjectPtr<T>, call the Callback when complete. */
    template < typename T = UObject >
    void AsyncLoad( TSoftObjectPtr< T > soft_object, TFunction< void() > && callback )
    {
        AsyncLoad( soft_object.ToSoftObjectPath(), FSimpleDelegate::CreateLambda( MoveTemp( callback ) ) );
    }

    /** Async load a TSoftObjectPtr<T>, call the Callback when complete. */
    template < typename T = UObject >
    void AsyncLoad( TSoftObjectPtr< T > soft_object, TFunction< void( T * ) > && callback )
    {
        AsyncLoad( soft_object.ToSoftObjectPath(),
            FSimpleDelegate::CreateLambda( [ soft_object, UserCallback = MoveTemp( callback ) ]() mutable {
                UserCallback( soft_object.Get() );
            } ) );
    }

    /** Async load a TSoftObjectPtr<T>, call the Callback when complete. */
    template < typename T = UObject >
    void AsyncLoad( TSoftObjectPtr< T > soft_object, const FSimpleDelegate & callback = FSimpleDelegate() )
    {
        AsyncLoad( soft_object.ToSoftObjectPath(), callback );
    }

    /** Async load a FSoftObjectPath, call the Callback when complete. */
    void AsyncLoad( const FSoftObjectPath & soft_object_path, const FSimpleDelegate & callback = FSimpleDelegate() );

    /** Async load an array of FSoftObjectPath, call the Callback when complete. */
    void AsyncLoad( const TArray< FSoftObjectPath > & soft_object_paths, TFunction< void() > && callback )
    {
        AsyncLoad( soft_object_paths, FSimpleDelegate::CreateLambda( MoveTemp( callback ) ) );
    }

    /** Async load an array of FSoftObjectPath, call the Callback when complete. */
    void AsyncLoad( const TArray< FSoftObjectPath > & soft_object_paths, const FSimpleDelegate & callback = FSimpleDelegate() );

    /** Given an array of primary assets, it loads all of the bundles referenced by properties of these assets specified in the LoadBundles array. */
    template < typename T = UPrimaryDataAsset >
    void AsyncPreloadPrimaryAssetsAndBundles( const TArray< T * > & assets, const TArray< FName > & load_bundles, const FSimpleDelegate & callback = FSimpleDelegate() )
    {
        TArray< FPrimaryAssetId > primary_asset_ids;
        for ( const auto * item : assets )
        {
            primary_asset_ids.Add( item );
        }

        AsyncPreloadPrimaryAssetsAndBundles( primary_asset_ids, load_bundles, callback );
    }

    /** Given an array of primary asset ids, it loads all of the bundles referenced by properties of these assets specified in the LoadBundles array. */
    void AsyncPreloadPrimaryAssetsAndBundles( const TArray< FPrimaryAssetId > & asset_ids, const TArray< FName > & load_bundles, TFunction< void() > && callback )
    {
        AsyncPreloadPrimaryAssetsAndBundles( asset_ids, load_bundles, FSimpleDelegate::CreateLambda( MoveTemp( callback ) ) );
    }

    /** Given an array of primary asset ids, it loads all of the bundles referenced by properties of these assets specified in the LoadBundles array. */
    void AsyncPreloadPrimaryAssetsAndBundles( const TArray< FPrimaryAssetId > & asset_ids, const TArray< FName > & load_bundles, const FSimpleDelegate & callback = FSimpleDelegate() );

    /** Add a future condition that must be true before we move forward. */
    void AsyncCondition( const TSharedRef< FGBFAsyncCondition > & condition, const FSimpleDelegate & callback = FSimpleDelegate() );

    /**
     * Rather than load anything, this callback is just inserted into the callback sequence so that when async loading
     * completes this event will be called at the same point in the sequence.  Super useful if you don't want a step to be
     * tied to a particular asset in case some of the assets are optional.
     */
    void AsyncEvent( TFunction< void() > && callback )
    {
        AsyncEvent( FSimpleDelegate::CreateLambda( MoveTemp( callback ) ) );
    }

    /**
     * Rather than load anything, this callback is just inserted into the callback sequence so that when async loading
     * completes this event will be called at the same point in the sequence.  Super useful if you don't want a step to be
     * tied to a particular asset in case some of the assets are optional.
     */
    void AsyncEvent( const FSimpleDelegate & callback );

    /** Flushes any async loading requests. */
    void StartAsyncLoading();

    /** Cancels any pending async loads. */
    void CancelAsyncLoading();

    /** Is async loading current in progress? */
    bool IsAsyncLoadingInProgress() const;

private:
    /**
     * The FGBFLoadingState is what actually is allocated for the FGBFAsyncMixin in a big map so that the FGBFAsyncMixin itself holds no
     * no memory, and we dynamically create the FGBFLoadingState only if needed, and destroy it when it's unneeded.
     */
    class FGBFLoadingState : public TSharedFromThis< FGBFLoadingState >
    {
    public:
        FGBFLoadingState( FGBFAsyncMixin & owner );
        virtual ~FGBFLoadingState();

        /** Starts the async sequence. */
        void Start();

        /** Cancels the async sequence. */
        void CancelAndDestroy();

        void AsyncLoad( const FSoftObjectPath & soft_object, const FSimpleDelegate & delegate_to_call );
        void AsyncLoad( const TArray< FSoftObjectPath > & soft_object_paths, const FSimpleDelegate & delegate_to_call );
        void AsyncPreloadPrimaryAssetsAndBundles( const TArray< FPrimaryAssetId > & primary_asset_ids, const TArray< FName > & load_bundles, const FSimpleDelegate & delegate_to_call );
        void AsyncCondition( TSharedRef< FGBFAsyncCondition > condition, const FSimpleDelegate & callback );
        void AsyncEvent( const FSimpleDelegate & callback );

        bool IsLoadingComplete() const
        {
            return !IsLoadingInProgress();
        }
        bool IsLoadingInProgress() const;
        bool IsLoadingInProgressOrPending() const;
        bool IsPendingDestroy() const;

    private:
        void CancelOnly( bool destroying );
        void CancelStartTimer();
        void TryScheduleStart();
        void TryCompleteAsyncLoading();
        void CompleteAsyncLoading();

    private:
        void RequestDestroyThisMemory();
        void CancelDestroyThisMemory( bool destroying );

        /** Who owns the loading state?  We need this to call back into the owning mix-in object. */
        FGBFAsyncMixin & OwnerRef;

        /**
         * Did we need to pre-load bundles?  If we didn't pre-load bundles (which require you keep the streaming handle
         * around or they will be destroyed), then we can safely destroy the FGBFLoadingState when everything is done loading.
         */
        bool bPreloadedBundles = false;

        class FAsyncStep
        {
        public:
            FAsyncStep( const FSimpleDelegate & user_callback );
            FAsyncStep( const FSimpleDelegate & user_callback, const TSharedPtr< FStreamableHandle > & streaming_handle );
            FAsyncStep( const FSimpleDelegate & user_callback, const TSharedPtr< FGBFAsyncCondition > & condition );

            ~FAsyncStep();

            void ExecuteUserCallback();

            bool IsLoadingInProgress() const
            {
                return !IsComplete();
            }

            bool IsComplete() const;
            void Cancel();

            bool BindCompleteDelegate( const FSimpleDelegate & new_delegate );
            bool IsCompleteDelegateBound() const;

        private:
            FSimpleDelegate UserCallback;
            bool bIsCompletionDelegateBound = false;

            // Possible Async 'thing'
            TSharedPtr< FStreamableHandle > StreamingHandle;
            TSharedPtr< FGBFAsyncCondition > Condition;
        };

        bool bHasStarted = false;

        int32 CurrentAsyncStep = 0;
        TArray< TUniquePtr< FAsyncStep > > AsyncSteps;
        TArray< TUniquePtr< FAsyncStep > > AsyncStepsPendingDestruction;

        FTSTicker::FDelegateHandle StartTimerDelegate;
        FTSTicker::FDelegateHandle DestroyMemoryDelegate;
    };

    const FGBFLoadingState & GetLoadingStateConst() const;

    FGBFLoadingState & GetLoadingState();

    bool HasLoadingState() const;

    bool IsLoadingInProgressOrPending() const;

private:
    static TMap< FGBFAsyncMixin *, TSharedRef< FGBFLoadingState > > Loading;
};

/**
 * Sometimes a mix-in just doesn't make sense.  Perhaps the object has to manage many different jobs
 * that each have their own async dependency chain/scope.  For those situations you can use the FGBFAsyncScope.
 *
 * This class is a standalone Async dependency handler so that you can fire off several load jobs and always handle them
 * in the proper order, just like with combining FGBFAsyncMixin with your class.
 */
class GAMEBASEFRAMEWORK_API FGBFAsyncScope : public FGBFAsyncMixin
{
public:
    using FGBFAsyncMixin::AsyncLoad;

    using FGBFAsyncMixin::AsyncPreloadPrimaryAssetsAndBundles;

    using FGBFAsyncMixin::AsyncCondition;

    using FGBFAsyncMixin::AsyncEvent;

    using FGBFAsyncMixin::CancelAsyncLoading;

    using FGBFAsyncMixin::StartAsyncLoading;

    using FGBFAsyncMixin::IsAsyncLoadingInProgress;
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

enum class EGBFAsyncConditionResult : uint8
{
    TryAgain,
    Complete
};

DECLARE_DELEGATE_RetVal( EGBFAsyncConditionResult, FGBFAsyncConditionDelegate );

/**
 * The async condition allows you to have custom reasons to hault the async loading until some condition is met.
 */
class FGBFAsyncCondition : public TSharedFromThis< FGBFAsyncCondition >
{
public:
    FGBFAsyncCondition( const FGBFAsyncConditionDelegate & condition );
    FGBFAsyncCondition( TFunction< EGBFAsyncConditionResult() > && condition );
    virtual ~FGBFAsyncCondition();

protected:
    bool IsComplete() const;
    bool BindCompleteDelegate( const FSimpleDelegate & new_delegate );

private:
    bool TryToContinue( float delta_time );

    FTSTicker::FDelegateHandle RepeatHandle;
    FGBFAsyncConditionDelegate UserCondition;
    FSimpleDelegate CompletionDelegate;

    friend FGBFAsyncMixin;
};
