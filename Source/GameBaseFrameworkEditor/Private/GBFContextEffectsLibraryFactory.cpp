#include "GBFContextEffectsLibraryFactory.h"

#include "Feedback/ContextEffects/GBFContextEffectsLibrary.h"

UGBFContextEffectsLibraryFactory::UGBFContextEffectsLibraryFactory()
{
    SupportedClass = UGBFContextEffectsLibrary::StaticClass();

    bCreateNew = true;
    bEditorImport = false;
    bEditAfterNew = true;
}

UObject * UGBFContextEffectsLibraryFactory::FactoryCreateNew( UClass * in_class,
    UObject * in_parent,
    const FName in_name,
    const EObjectFlags flags,
    UObject * context,
    FFeedbackContext * warn )
{
    auto * context_effects_library = NewObject< UGBFContextEffectsLibrary >( in_parent, in_name, flags );
    return context_effects_library;
}