#pragma once

#include "CommonTabListWidgetBase.h"

#include <CoreMinimal.h>

#include "GBFTabListWidgetBase.generated.h"

USTRUCT( BlueprintType )
struct FGBFTabDescriptor
{
    GENERATED_BODY()

public:
    FGBFTabDescriptor() :
        bHidden( false ),
        CreatedTabContentWidget( nullptr )
    {}

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    FName TabId;

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    FText TabText;

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    FSlateBrush IconBrush;

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    bool bHidden;

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TSubclassOf< UCommonButtonBase > TabButtonType;

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TSubclassOf< UCommonUserWidget > TabContentType;

    UPROPERTY( Transient )
    TObjectPtr< UWidget > CreatedTabContentWidget;
};

UINTERFACE( BlueprintType )
class UGBFTabButtonInterface : public UInterface
{
    GENERATED_BODY()
};

class IGBFTabButtonInterface
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintNativeEvent, Category = "Tab Button" )
    void SetTabLabelInfo( const FGBFTabDescriptor & TabDescriptor );
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFTabListWidgetBase : public UCommonTabListWidgetBase
{
    GENERATED_BODY()

public:
    /** Helper method to get at all the preregistered tab infos */
    const TArray< FGBFTabDescriptor > & GetAllPreregisteredTabInfos();

    UFUNCTION( BlueprintCallable, BlueprintPure, Category = "Tab List" )
    bool GetPreregisteredTabInfo( const FName tab_name_id, FGBFTabDescriptor & out_tab_info );

    // Toggles whether or not a specified tab is hidden, can only be called before the switcher is associated
    UFUNCTION( BlueprintCallable, Category = "Tab List" )
    void SetTabHiddenState( FName tab_name_id, bool hidden );

    UFUNCTION( BlueprintCallable, Category = "Tab List" )
    bool RegisterDynamicTab( const FGBFTabDescriptor & tab_descriptor );

    UFUNCTION( BlueprintCallable, Category = "Tab List" )
    bool IsFirstTabActive() const;

    UFUNCTION( BlueprintCallable, Category = "Tab List" )
    bool IsLastTabActive() const;

    UFUNCTION( BlueprintCallable, BlueprintPure, Category = "Tab List" )
    bool IsTabVisible( FName tab_id );

    UFUNCTION( BlueprintCallable, BlueprintPure, Category = "Tab List" )
    int32 GetVisibleTabCount();

    /** Delegate broadcast when a new tab is created. Allows hook ups after creation. */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnTabContentCreated, FName, TabId, UCommonUserWidget *, TabWidget );
    DECLARE_EVENT_TwoParams( UGBFTabListWidgetBase, FOnTabContentCreatedNative, FName /* TabId */, UCommonUserWidget * /* TabWidget */ );

    /** Broadcasts when a new tab is created. */
    UPROPERTY( BlueprintAssignable, Category = "Tab List" )
    FOnTabContentCreated OnTabContentCreated;
    FOnTabContentCreatedNative OnTabContentCreatedNative;

protected:
    void NativeConstruct() override;
    void NativeDestruct() override;
    void HandlePreLinkedSwitcherChanged() override;
    void HandlePostLinkedSwitcherChanged() override;

    void HandleTabCreation_Implementation( FName tab_id, UCommonButtonBase * tab_button ) override;

private:
    void SetupTabs();

    UPROPERTY( EditAnywhere, meta = ( TitleProperty = "TabId" ) )
    TArray< FGBFTabDescriptor > PreregisteredTabInfoArray;

    /**
     * Stores label info for tabs that have been registered at runtime but not yet created.
     * Elements are removed once they are created.
     */
    UPROPERTY()
    TMap< FName, FGBFTabDescriptor > PendingTabLabelInfoMap;
};

FORCEINLINE const TArray< FGBFTabDescriptor > & UGBFTabListWidgetBase::GetAllPreregisteredTabInfos()
{
    return PreregisteredTabInfoArray;
}
