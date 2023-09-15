#pragma once

#include <CoreMinimal.h>
#include <UObject/Object.h>

#include "GBFContextEffectsLibrary.generated.h"

class UNiagaraSystem;

UENUM()
enum class EGBFContextEffectsLibraryLoadState : uint8
{
    Unloaded = 0,
    Loading = 1,
    Loaded = 2
};

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGBFContextEffects
{
    GENERATED_BODY()

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    FGameplayTag EffectTag;

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    FGameplayTagContainer Context;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( AllowedClasses = "/Script/Engine.SoundBase, /Script/Niagara.NiagaraSystem" ) )
    TArray< FSoftObjectPath > Effects;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFActiveContextEffects : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY( VisibleAnywhere )
    FGameplayTag EffectTag;

    UPROPERTY( VisibleAnywhere )
    FGameplayTagContainer Context;

    UPROPERTY( VisibleAnywhere )
    TArray< TObjectPtr< USoundBase > > Sounds;

    UPROPERTY( VisibleAnywhere )
    TArray< TObjectPtr< UNiagaraSystem > > NiagaraSystems;
};

DECLARE_DYNAMIC_DELEGATE_OneParam( FGBFContextEffectLibraryLoadingComplete, TArray< UGBFActiveContextEffects * >, active_context_effects );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFContextEffectsLibrary : public UObject
{
    GENERATED_BODY()

public:
    EGBFContextEffectsLibraryLoadState GetContextEffectsLibraryLoadState() const;

    UFUNCTION( BlueprintCallable )
    void GetEffects( const FGameplayTag effect, const FGameplayTagContainer context, TArray< USoundBase * > & sounds, TArray< UNiagaraSystem * > & niagara_systems );

    UFUNCTION( BlueprintCallable )
    void LoadEffects();

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TArray< FGBFContextEffects > ContextEffects;

private:
    void LoadEffectsInternal();

    void ContextEffectLibraryLoadingComplete( const TArray< UGBFActiveContextEffects * > & active_context_effects );

    UPROPERTY( Transient )
    TArray< TObjectPtr< UGBFActiveContextEffects > > ActiveContextEffects;

    UPROPERTY( Transient )
    EGBFContextEffectsLibraryLoadState EffectsLoadState = EGBFContextEffectsLibraryLoadState::Unloaded;
};

FORCEINLINE EGBFContextEffectsLibraryLoadState UGBFContextEffectsLibrary::GetContextEffectsLibraryLoadState() const
{
    return EffectsLoadState;
}
