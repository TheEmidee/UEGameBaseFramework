#pragma once

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "GBFPlayerDisplayAsset.generated.h"

class UTexture;
class UMeshComponent;
class UMaterialInstanceDynamic;
class UNiagaraComponent;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFPlayerDisplayAsset : public UDataAsset
{
    GENERATED_BODY()
public:
    UFUNCTION( BlueprintCallable, Category = "Player" )
    void ApplyToMaterial( UMaterialInstanceDynamic * material ) const;

    UFUNCTION( BlueprintCallable, Category = "Player" )
    void ApplyToMeshComponent( UMeshComponent * mesh_component ) const;

    UFUNCTION( BlueprintCallable, Category = "Player" )
    void ApplyToNiagaraComponent( UNiagaraComponent * niagara_component ) const;

    UFUNCTION( BlueprintCallable, Category = "Player", meta = ( DefaultToSelf = "TargetActor" ) )
    void ApplyToActor( AActor * target_actor, bool include_child_actors = true, bool include_attached_actors = true, bool include_parent_actors = true ) const;

protected:
    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    FText DisplayName;

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TMap< FName, float > ScalarParameters;

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TMap< FName, FLinearColor > ColorParameters;

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TMap< FName, TObjectPtr< UTexture > > TextureParameters;
};
