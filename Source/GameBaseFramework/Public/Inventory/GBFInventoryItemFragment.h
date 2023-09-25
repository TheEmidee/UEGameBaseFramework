#pragma once

#include <CoreMinimal.h>

#include "GBFInventoryItemFragment.generated.h"

class UGBFInventoryItemInstance;

UCLASS( DefaultToInstanced, EditInlineNew, Abstract )
class GAMEBASEFRAMEWORK_API UGBFInventoryItemFragment : public UObject
{
    GENERATED_BODY()

public:
    virtual void OnInstanceCreated( UGBFInventoryItemInstance * instance ) const;
};
