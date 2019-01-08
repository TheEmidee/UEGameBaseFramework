#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GBFSoftObjectPtrFunctionLibrary.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFSoftObjectPtrFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    UFUNCTION( BlueprintPure, Category = "Utilities", meta = ( BlueprintThreadSafe ) )
    static UObject * SyncLoadSoftObjectPtr( const TSoftObjectPtr<UObject> & soft_object_reference );
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FGBFSoftObjectPtrLoaded, UObject *, object );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFASyncLoadSoftObjectPtr : public UObject
{
    GENERATED_BODY()

public:
    
    UFUNCTION( BlueprintCallable, Category = "Utilities", meta = ( DisplayName = "ASyncLoadSoftObjectPtr", BlueprintInternalUseOnly = "TRUE" ) )
    static UGBFASyncLoadSoftObjectPtr * ASyncLoadSoftObjectPtr( const TSoftObjectPtr<UObject> & soft_object_reference );

    UPROPERTY( BlueprintAssignable )
    FGBFSoftObjectPtrLoaded ObjectLoaded;

    UFUNCTION( BlueprintCallable )
    void RequestAsyncLoad();

private:

    TSoftObjectPtr< UObject > ObjectToLoad;
};
