#pragma once

#include <CoreMinimal.h>
#include <Engine/StreamableManager.h>

DECLARE_DELEGATE_OneParam( FGBFAssetManagerStartupJobSubstepProgress, float /*NewProgress*/ );

/** Handles reporting progress from streamable handles */
struct FGBFAssetManagerStartupJob
{
    /** Simple job that is all synchronous */
    FGBFAssetManagerStartupJob( const FString & job_name, const TFunction< void( const FGBFAssetManagerStartupJob &, TSharedPtr< FStreamableHandle > & ) > & function, float job_weight );

    /** Perform actual loading, will return a handle if it created one */
    TSharedPtr< FStreamableHandle > DoJob() const;

    void UpdateSubstepProgress( float new_progress ) const;
    void UpdateSubstepProgressFromStreamable( TSharedRef< FStreamableHandle > streamable_handle ) const;

    FGBFAssetManagerStartupJobSubstepProgress SubstepProgressDelegate;
    TFunction< void( const FGBFAssetManagerStartupJob &, TSharedPtr< FStreamableHandle > & ) > JobFunc;
    FString JobName;
    float JobWeight;
    mutable double LastUpdate = 0;
};
