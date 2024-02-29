#pragma once

#include <Animation/AnimNotifies/AnimNotify.h>
#include <CoreMinimal.h>

#include "GBFAnimNotify_SendGameplayEvent.generated.h"

UCLASS( meta = ( DisplayName = "Send Gameplay Event" ) )
class GAMEBASEFRAMEWORK_API UGBFAnimNotify_SendGameplayEvent : public UAnimNotify
{
    GENERATED_BODY()

public:
    void Notify( USkeletalMeshComponent * mesh_comp, UAnimSequenceBase * animation, const FAnimNotifyEventReference & event_reference ) override;

protected:
    UFUNCTION( BlueprintNativeEvent )
    AActor * GetTargetActorFromOwner( AActor * owner ) const;

private:
    UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    FGameplayTag EventTag;
};
