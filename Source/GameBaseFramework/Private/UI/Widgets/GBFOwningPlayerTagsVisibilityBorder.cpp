#include "UI/Widgets/GBFOwningPlayerTagsVisibilityBorder.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Pawn.h"

#include <GameplayTagAssetInterface.h>

UGBFOwningPlayerTagsVisibilityBorder::UGBFOwningPlayerTagsVisibilityBorder( const FObjectInitializer & object_initializer ) :
    Super( object_initializer ),
    VisibleType( ESlateVisibility::SelfHitTestInvisible ),
    HiddenType( ESlateVisibility::Collapsed )
{
    SetPadding( FMargin( 0.f, 0.f, 0.f, 0.f ) );
}

void UGBFOwningPlayerTagsVisibilityBorder::OnWidgetRebuilt()
{
    Super::OnWidgetRebuilt();
    UpdateVisibility();
    ListenToTagChanged();
}

void UGBFOwningPlayerTagsVisibilityBorder::UpdateVisibility()
{
    if ( IsDesignTime() )
    {
        return;
    }

    if ( auto * lp = GetOwningLocalPlayer() )
    {
        if ( auto * pc = lp->GetPlayerController( GetWorld() ) )
        {
            if ( auto * pawn = pc->GetPawn() )
            {
                if ( auto * interface = Cast< IGameplayTagAssetInterface >( pawn ) )
                {
                    FGameplayTagContainer tags;
                    interface->GetOwnedGameplayTags( tags );

                    const auto is_visible = VisibilityQuery.Matches( tags );
                    SetVisibility( is_visible ? VisibleType : HiddenType );
                }
            }
        }
    }
}

void UGBFOwningPlayerTagsVisibilityBorder::ListenToTagChanged()
{
    if ( auto * lp = GetOwningLocalPlayer() )
    {
        if ( auto * pc = lp->GetPlayerController( GetWorld() ) )
        {
            if ( auto * pawn = pc->GetPawn() )
            {
                if ( auto * asc = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent( pawn ) )
                {
                    for ( const auto & [ tag, handle ] : GameplayTagListenerHandles )
                    {
                        asc->UnregisterGameplayTagEvent( handle, tag );
                    }

                    GameplayTagListenerHandles.Reset();

                    TArray< FGameplayTag > query_tags;
                    VisibilityQuery.GetGameplayTagArray( query_tags );

                    for ( auto tag : query_tags )
                    {
                        GameplayTagListenerHandles.Add( tag, asc->RegisterGameplayTagEvent( tag ).AddUObject( this, &ThisClass::OnTagsUpdated ) );
                    }
                }
            }
        }
    }
}

void UGBFOwningPlayerTagsVisibilityBorder::OnTagsUpdated( FGameplayTag /*gameplay_tag*/, int /*count*/ )
{
    UpdateVisibility();
}