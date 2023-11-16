#include "GAS/Targeting/GASExtTargetDataFilter.h"

#include <AbilitySystemBlueprintLibrary.h>

FGameplayAbilityTargetDataHandle UGASExtTargetDataFilter::FilterTargetData( FGameplayAbilityTargetDataHandle target_data_handle ) const
{
    return UAbilitySystemBlueprintLibrary::FilterTargetData( target_data_handle, MakeFilterHandle() );
}
bool UGASExtTargetDataFilter::IsSupportedForNetworking() const
{
    return true;
}

FGameplayTargetDataFilterHandle UGASExtTargetDataFilter::MakeFilterHandle() const
{
    return FGameplayTargetDataFilterHandle();
}

UGASExtTargetDataFilter_IsActorOfClass::UGASExtTargetDataFilter_IsActorOfClass()
{
    Filter.SelfFilter = ETargetDataFilterSelf::TDFS_Any;
}

FGameplayTargetDataFilterHandle UGASExtTargetDataFilter_IsActorOfClass::MakeFilterHandle() const
{
    FGameplayTargetDataFilterHandle handle;

    const auto new_filter = MakeShared< FGameplayTargetDataFilter >( Filter );
    new_filter->RequiredActorClass = RequiredActorClass;
    new_filter->InitializeFilterContext( nullptr );
    handle.Filter = new_filter;

    return handle;
}