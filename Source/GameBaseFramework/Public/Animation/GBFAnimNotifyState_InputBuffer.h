#pragma once

#include <Animation/AnimNotifies/AnimNotifyState.h>
#include <CoreMinimal.h>

#include "GBFAnimNotifyState_InputBuffer.generated.h"

class UGBFAbilityInputBufferComponent;

UCLASS( DisplayName = "Input Buffer Window" )
class GAMEBASEFRAMEWORK_API UGBFAnimNotifyState_InputBuffer : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    void NotifyBegin( USkeletalMeshComponent * mesh_component, UAnimSequenceBase * animation, float total_duration, const FAnimNotifyEventReference & event_reference ) override;
    void NotifyEnd( USkeletalMeshComponent * mesh_component, UAnimSequenceBase * animation, const FAnimNotifyEventReference & event_reference ) override;

protected:
    UFUNCTION( BlueprintNativeEvent )
    UGBFAbilityInputBufferComponent * GetAbilityInputBufferComponent( const USkeletalMeshComponent * mesh_component ) const;

private:
    UPROPERTY( EditAnywhere, Meta = ( Categories = "Input" ) )
    FGameplayTagContainer InputTagContainer;
};
