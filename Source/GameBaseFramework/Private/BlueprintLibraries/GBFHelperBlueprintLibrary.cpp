#include "GBFHelperBlueprintLibrary.h"

void UGBFHelperBlueprintLibrary::CreateObject( TSubclassOf< UObject > class_of, UObject *& object )
{
    object = NewObject<UObject>( GetTransientPackage(), class_of );
}