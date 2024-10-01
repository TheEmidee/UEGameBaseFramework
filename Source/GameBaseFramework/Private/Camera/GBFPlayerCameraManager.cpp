#include "Camera/GBFPlayerCameraManager.h"

#include "Camera/GBFCameraComponent.h"
#include "Camera/Modifiers/GBFCameraModifier.h"
#include "Camera/Modifiers/GBFCameraModifierStack.h"

#include <Engine/Canvas.h>
#include <Engine/Engine.h>
#include <GameFramework/Pawn.h>
#include <GameFramework/PlayerController.h>

void AGBFPlayerCameraManager::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    ForEachCameraStackModifier( [ & ]( auto * modifier ) {
        modifier->AddedToCamera( this );
    },
        false );
}

void AGBFPlayerCameraManager::ProcessViewRotation( const float delta_time, FRotator & view_rotation, FRotator & delta_rot )
{
    ForEachCameraStackModifier( TFunctionRef< bool( UGBFCameraModifier * ) >( [ & ]( UGBFCameraModifier * modifier ) {
        return modifier->ProcessViewRotation( ViewTarget.Target, delta_time, view_rotation, delta_rot );
    } ),
        true );

    Super::ProcessViewRotation( delta_time, view_rotation, delta_rot );
}

void AGBFPlayerCameraManager::ApplyCameraModifiers( const float delta_time, FMinimalViewInfo & pov )
{
    ForEachCameraStackModifier( TFunctionRef< void( UGBFCameraModifier * ) >( [ & ]( UGBFCameraModifier * modifier ) {
        return modifier->CameraOwner = this;
    } ),
        false );

    Super::ApplyCameraModifiers( delta_time, pov );

    ForEachCameraStackModifier( TFunctionRef< bool( UGBFCameraModifier * ) >( [ & ]( UGBFCameraModifier * modifier ) {
        return modifier->ModifyCamera( delta_time, pov );
    } ),
        true );
}

void AGBFPlayerCameraManager::SetViewTarget( AActor * new_view_target, const FViewTargetTransitionParams transition_params )
{
    Super::SetViewTarget( new_view_target, transition_params );

    ForEachCameraStackModifier( [ & ]( auto * modifier ) -> void {
        modifier->OnViewTargetChanged( new_view_target );
    },
        false );
}

void AGBFPlayerCameraManager::SetModifierStack( UGBFCameraModifierStack * modifier_stack )
{
    if ( modifier_stack == ModifierStack )
    {
        return;
    }

    ForEachCameraStackModifier( [ & ]( auto * modifier ) -> void {
        modifier->RemovedFromCameraManager();
    },
        false );

    ModifierStack = modifier_stack;

    ForEachCameraStackModifier( [ & ]( auto * modifier ) {
        modifier->AddedToCamera( this );
        modifier->OnViewTargetChanged( GetViewTarget() );
    },
        false );
}

void AGBFPlayerCameraManager::DisplayDebug( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos )
{
    check( canvas != nullptr );

    auto & display_debug_manager = canvas->DisplayDebugManager;

    display_debug_manager.SetFont( GEngine->GetSmallFont() );
    display_debug_manager.SetDrawColor( FColor::Yellow );
    display_debug_manager.DrawString( FString::Printf( TEXT( "GBFPlayerCameraManager: %s" ), *GetNameSafe( this ) ) );

    Super::DisplayDebug( canvas, debug_display, yl, y_pos );

    const auto * pawn = PCOwner ? PCOwner->GetPawn() : nullptr;

    if ( const auto * camera_component = UGBFCameraComponent::FindCameraComponent( pawn ) )
    {
        camera_component->DrawDebug( canvas );
    }

    if ( ModifierStack != nullptr )
    {
        display_debug_manager.SetFont( GEngine->GetSmallFont() );
        display_debug_manager.SetDrawColor( FColor::Cyan );
        display_debug_manager.DrawString( FString( TEXT( "   --- Modifiers Stack (Begin) ---" ) ) );

        ForEachCameraStackModifier( [ & ]( auto * modifier ) {
            display_debug_manager.DrawString( modifier->GetName() );
            modifier->DisplayDebug( canvas, debug_display, yl, y_pos );
        },
            true );

        display_debug_manager.SetDrawColor( FColor::Cyan );
        display_debug_manager.DrawString( FString( TEXT( "   --- Modifiers Stack (End) ---" ) ) );
    }
}

void AGBFPlayerCameraManager::ForEachCameraStackModifier( const TFunctionRef< bool( UGBFCameraModifier * ) > & functor, bool dont_execute_if_disabled ) const
{
    if ( ModifierStack == nullptr )
    {
        return;
    }

    for ( auto modifier : ModifierStack->Modifiers )
    {
        if ( modifier != nullptr && ( !modifier->IsDisabled() || !dont_execute_if_disabled ) )
        {
            if ( functor( modifier ) )
            {
                break;
            }
        }
    }
}

void AGBFPlayerCameraManager::ForEachCameraStackModifier( const TFunctionRef< void( UGBFCameraModifier * ) > & functor, bool dont_execute_if_disabled ) const
{
    if ( ModifierStack == nullptr )
    {
        return;
    }

    for ( auto modifier : ModifierStack->Modifiers )
    {
        if ( modifier != nullptr && ( !modifier->IsDisabled() || !dont_execute_if_disabled ) )
        {
            functor( modifier );
        }
    }
}