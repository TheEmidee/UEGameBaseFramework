#include "UI/Widgets/GBFTabListWidgetBase.h"

#include "CommonAnimatedSwitcher.h"
#include "CommonButtonBase.h"

bool UGBFTabListWidgetBase::GetPreregisteredTabInfo( const FName tab_name_id, FGBFTabDescriptor & out_tab_info )
{
    const auto * const found_tab_info = PreregisteredTabInfoArray.FindByPredicate( [ & ]( auto & tab_info ) -> bool {
        return tab_info.TabId == tab_name_id;
    } );

    if ( !found_tab_info )
    {
        return false;
    }

    out_tab_info = *found_tab_info;
    return true;
}

void UGBFTabListWidgetBase::SetTabHiddenState( const FName tab_name_id, const bool hidden )
{
    for ( auto & tab_info : PreregisteredTabInfoArray )
    {
        if ( tab_info.TabId == tab_name_id )
        {
            tab_info.bHidden = hidden;
            break;
        }
    }
}

bool UGBFTabListWidgetBase::RegisterDynamicTab( const FGBFTabDescriptor & tab_descriptor )
{
    // If it's hidden we just ignore it.
    if ( tab_descriptor.bHidden )
    {
        return true;
    }

    PendingTabLabelInfoMap.Add( tab_descriptor.TabId, tab_descriptor );

    return RegisterTab( tab_descriptor.TabId, tab_descriptor.TabButtonType, tab_descriptor.CreatedTabContentWidget );
}

bool UGBFTabListWidgetBase::IsFirstTabActive() const
{
    if ( PreregisteredTabInfoArray.Num() > 0 )
    {
        return GetActiveTab() == PreregisteredTabInfoArray[ 0 ].TabId;
    }

    return false;
}

bool UGBFTabListWidgetBase::IsLastTabActive() const
{
    if ( PreregisteredTabInfoArray.Num() > 0 )
    {
        return GetActiveTab() == PreregisteredTabInfoArray.Last().TabId;
    }

    return false;
}

bool UGBFTabListWidgetBase::IsTabVisible( const FName tab_id )
{
    if ( const auto * button = GetTabButtonBaseByID( tab_id ) )
    {
        const auto tab_visibility = button->GetVisibility();
        return ( tab_visibility == ESlateVisibility::Visible || tab_visibility == ESlateVisibility::HitTestInvisible || tab_visibility == ESlateVisibility::SelfHitTestInvisible );
    }

    return false;
}

int32 UGBFTabListWidgetBase::GetVisibleTabCount()
{
    auto result = 0;
    const auto tab_count = GetTabCount();
    for ( auto index = 0; index < tab_count; index++ )
    {
        if ( IsTabVisible( GetTabIdAtIndex( index ) ) )
        {
            result++;
        }
    }

    return result;
}

void UGBFTabListWidgetBase::NativeConstruct()
{
    Super::NativeConstruct();
    SetupTabs();
}

void UGBFTabListWidgetBase::NativeDestruct()
{
    for ( auto & tab_info : PreregisteredTabInfoArray )
    {
        if ( tab_info.CreatedTabContentWidget != nullptr )
        {
            tab_info.CreatedTabContentWidget->RemoveFromParent();
            tab_info.CreatedTabContentWidget = nullptr;
        }
    }

    Super::NativeDestruct();
}

void UGBFTabListWidgetBase::HandlePreLinkedSwitcherChanged()
{
    for ( const auto & tab_info : PreregisteredTabInfoArray )
    {
        // Remove tab content widget from linked switcher, as it is being disassociated.
        if ( tab_info.CreatedTabContentWidget )
        {
            tab_info.CreatedTabContentWidget->RemoveFromParent();
        }
    }

    Super::HandlePreLinkedSwitcherChanged();
}

void UGBFTabListWidgetBase::HandlePostLinkedSwitcherChanged()
{
    if ( !IsDesignTime() && GetCachedWidget().IsValid() )
    {
        // Don't bother making tabs if we're in the designer or haven't been constructed yet
        SetupTabs();
    }

    Super::HandlePostLinkedSwitcherChanged();
}

void UGBFTabListWidgetBase::HandleTabCreation_Implementation( const FName tab_id, UCommonButtonBase * tab_button )
{
    const FGBFTabDescriptor * tab_info_ptr;

    FGBFTabDescriptor tab_info;
    if ( GetPreregisteredTabInfo( tab_id, tab_info ) )
    {
        tab_info_ptr = &tab_info;
    }
    else
    {
        tab_info_ptr = PendingTabLabelInfoMap.Find( tab_id );
    }

    if ( tab_button->GetClass()->ImplementsInterface( UGBFTabButtonInterface::StaticClass() ) )
    {
        if ( ensureMsgf( tab_info_ptr, TEXT( "A tab button was created with id %s but no label info was specified. RegisterDynamicTab should be used over RegisterTab to provide label info." ), *tab_id.ToString() ) )
        {
            IGBFTabButtonInterface::Execute_SetTabLabelInfo( tab_button, *tab_info_ptr );
        }
    }

    PendingTabLabelInfoMap.Remove( tab_id );
}

void UGBFTabListWidgetBase::SetupTabs()
{
    for ( auto & tab_info : PreregisteredTabInfoArray )
    {
        if ( tab_info.bHidden )
        {
            continue;
        }

        // If the tab content hasn't been created already, create it.
        if ( !tab_info.CreatedTabContentWidget && tab_info.TabContentType )
        {
            tab_info.CreatedTabContentWidget = CreateWidget< UCommonUserWidget >( GetOwningPlayer(), tab_info.TabContentType );
            OnTabContentCreatedNative.Broadcast( tab_info.TabId, Cast< UCommonUserWidget >( tab_info.CreatedTabContentWidget ) );
            OnTabContentCreated.Broadcast( tab_info.TabId, Cast< UCommonUserWidget >( tab_info.CreatedTabContentWidget ) );
        }

        if ( auto * current_linked_switcher = GetLinkedSwitcher() )
        {
            // Add the tab content to the newly linked switcher.
            if ( !current_linked_switcher->HasChild( tab_info.CreatedTabContentWidget ) )
            {
                current_linked_switcher->AddChild( tab_info.CreatedTabContentWidget );
            }
        }

        // If the tab is not already registered, register it.
        if ( GetTabButtonBaseByID( tab_info.TabId ) == nullptr )
        {
            RegisterTab( tab_info.TabId, tab_info.TabButtonType, tab_info.CreatedTabContentWidget );
        }
    }
}
