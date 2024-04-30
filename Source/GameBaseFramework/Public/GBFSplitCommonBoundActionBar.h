#pragma once

#include "UI/Widgets/GBFBoundActionButton.h"

#include <Components/DynamicEntryBoxBase.h>
#include <Components/HorizontalBox.h>
#include <CoreMinimal.h>
#include <Tickable.h>

#include "GBFSplitCommonBoundActionBar.generated.h"

class ICommonBoundActionButtonInterface;
struct FUIActionBindingHandle;

/**
 * split action bar with one part snap to the left fort the back handler action and display the other actions snap to the right
 */
UCLASS()
class GAMEBASEFRAMEWORK_API UGBFSplitCommonBoundActionBar : public UUserWidget, public FTickableGameObject
{
    GENERATED_BODY()

public:
    explicit UGBFSplitCommonBoundActionBar( const FObjectInitializer & object_initializer );
    // FTickableGameObject Begin
    void Tick( float delta_time ) override;
    ETickableTickType GetTickableTickType() const override;
    TStatId GetStatId() const override;
    bool IsTickableWhenPaused() const override;
    // FTickableGameObject End

protected:
    bool IsEntryClassValid( TSubclassOf< UUserWidget > in_entry_class ) const;
    void OnWidgetRebuilt() override;
    void SynchronizeProperties() override;
    void ReleaseSlateResources( bool release_children ) override;
    UUserWidget * CreateEntryInternal( TSubclassOf< UUserWidget > in_entry_class, bool is_back_action );

    virtual void NativeOnActionButtonCreated( ICommonBoundActionButtonInterface * ActionButton, const FUIActionBindingHandle & RepresentedAction )
    {}

    UFUNCTION( BlueprintImplementableEvent )
    void OnActionBarUpdated();

#if WITH_EDITOR
    void ValidateCompiledDefaults( IWidgetCompilerLog & compile_log ) const override;
#endif

private:
    void HandleBoundActionsUpdated( bool from_owning_player );
    void HandleDeferredDisplayUpdate();
    void HandlePlayerAdded( int32 player_id );
    void MonitorPlayerActions( const ULocalPlayer * new_player );

    UPROPERTY( EditAnywhere, Category = EntryLayout, meta = ( MustImplement = "/Script/CommonUI.CommonBoundActionButtonInterface" ) )
    TSubclassOf< UGBFBoundActionButton > ActionButtonClass;

    UPROPERTY( EditAnywhere, AdvancedDisplay, Category = Display )
    uint8 bIgnoreDuplicateActions : 1;

    UPROPERTY( EditAnywhere, Category = Display )
    uint8 bDisplayOwningPlayerActionsOnly : 1;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess, BindWidget ) )
    TObjectPtr< UHorizontalBox > LeftHorizontalBox;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess, BindWidget ) )
    TObjectPtr< UHorizontalBox > RightHorizontalBox;

    UPROPERTY( Transient )
    FUserWidgetPool WidgetPool;

    uint8 bIsRefreshQueued : 1;
};
