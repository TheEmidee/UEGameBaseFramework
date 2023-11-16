#include "GAS/Targeting/GBFTargetDataFilter.h"

#include <AbilitySystemBlueprintLibrary.h>

FGameplayAbilityTargetDataHandle UGBFTargetDataFilter::FilterTargetData( FGameplayAbilityTargetDataHandle target_data_handle ) const
{
    return UAbilitySystemBlueprintLibrary::FilterTargetData( target_data_handle, MakeFilterHandle() );
}
bool UGBFTargetDataFilter::IsSupportedForNetworking() const
{
    return true;
}

FGameplayTargetDataFilterHandle UGBFTargetDataFilter::MakeFilterHandle() const
{
    return FGameplayTargetDataFilterHandle();
}

UGBFTargetDataFilter_IsActorOfClass::UGBFTargetDataFilter_IsActorOfClass()
{
    Filter.SelfFilter = ETargetDataFilterSelf::TDFS_Any;
}

FGameplayTargetDataFilterHandle UGBFTargetDataFilter_IsActorOfClass::MakeFilterHandle() const
{
    FGameplayTargetDataFilterHandle handle;

    const auto new_filter = MakeShared< FGameplayTargetDataFilter >( Filter );
    new_filter->RequiredActorClass = RequiredActorClass;
    new_filter->InitializeFilterContext( nullptr );
    handle.Filter = new_filter;

    return handle;
}