#include "Engine/GBFAssetManagerStartupJob.h"

#include "GBFLog.h"

FGBFAssetManagerStartupJob::FGBFAssetManagerStartupJob( const FString & job_name, const TFunction< void( const FGBFAssetManagerStartupJob &, TSharedPtr< FStreamableHandle > & ) > & function, float job_weight ) :
    JobFunc( function ),
    JobName( job_name ),
    JobWeight( job_weight )
{}

TSharedPtr< FStreamableHandle > FGBFAssetManagerStartupJob::DoJob() const
{
    const double job_start_time = FPlatformTime::Seconds();

    TSharedPtr< FStreamableHandle > handle;
    UE_LOG( LogGBF, Display, TEXT( "Startup job \"%s\" starting" ), *JobName );
    JobFunc( *this, handle );

    if ( handle.IsValid() )
    {
        handle->BindUpdateDelegate( FStreamableUpdateDelegate::CreateRaw( this, &FGBFAssetManagerStartupJob::UpdateSubstepProgressFromStreamable ) );
        handle->WaitUntilComplete( 0.0f, false );
        handle->BindUpdateDelegate( FStreamableUpdateDelegate() );
    }

    UE_LOG( LogGBF, Display, TEXT( "Startup job \"%s\" took %.2f seconds to complete" ), *JobName, FPlatformTime::Seconds() - job_start_time );

    return handle;
}

void FGBFAssetManagerStartupJob::UpdateSubstepProgress( float new_progress ) const
{
    SubstepProgressDelegate.ExecuteIfBound( new_progress );
}

void FGBFAssetManagerStartupJob::UpdateSubstepProgressFromStreamable( TSharedRef< FStreamableHandle > streamable_handle ) const
{
    if ( SubstepProgressDelegate.IsBound() )
    {
        // streamable_handle::GetProgress traverses() a large graph and is quite expensive
        const auto now = FPlatformTime::Seconds();

        if ( LastUpdate - now > 1.0 / 60 )
        {
            SubstepProgressDelegate.Execute( streamable_handle->GetProgress() );
            LastUpdate = now;
        }
    }
}
