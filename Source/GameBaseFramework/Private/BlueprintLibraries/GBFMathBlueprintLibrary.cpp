#include "BlueprintLibraries/GBFMathBlueprintLibrary.h"

int UGBFMathBlueprintLibrary::GetWrappedIndex( const int index, const int size )
{
    return ( ( index % size ) + size ) % size;
}
