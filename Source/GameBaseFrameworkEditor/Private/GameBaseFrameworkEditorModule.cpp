#include "GBFGameEditorStyle.h"
#include "GameBaseFrameworkSettings.h"
#include "IGameBaseFrameworkEditorModule.h"

#include <AbilitySystemGlobals.h>
#include <GameplayAbilitiesEditorModule.h>
#include <GameplayAbilitiesModule.h>
#include <GameplayCueInterface.h>
#include <GameplayCueNotify_Burst.h>
#include <GameplayCueNotify_BurstLatent.h>
#include <GameplayCueNotify_Looping.h>
#include <Modules/ModuleManager.h>
#include <PropertyEditorModule.h>

#define LOCTEXT_NAMESPACE "GBFEditor"

namespace
{
    // This function tells the GameplayCue editor what classes to expose when creating new notifies.
    void GetGameplayCueDefaultClasses( TArray< UClass * > & classes )
    {
        classes.Empty();
        classes.Add( UGameplayCueNotify_Burst::StaticClass() );
        classes.Add( AGameplayCueNotify_BurstLatent::StaticClass() );
        classes.Add( AGameplayCueNotify_Looping::StaticClass() );
    }

    // This function tells the GameplayCue editor what classes to search for GameplayCue events.
    void GetGameplayCueInterfaceClasses( TArray< UClass * > & classes )
    {
        classes.Empty();

        for ( auto * object_class : TObjectRange< UClass >() )
        {
            if ( object_class->IsChildOf< AActor >() && object_class->ImplementsInterface( UGameplayCueInterface::StaticClass() ) )
            {
                classes.Add( object_class );
            }
        }
    }

    // This function tells the GameplayCue editor where to create the GameplayCue notifies based on their tag.
    FString GetGameplayCuePath( FString gameplay_cue_tag )
    {
        auto path = FString( TEXT( "/Game" ) );

        //@TODO: Try plugins (e.g., GameplayCue.ShooterGame.Foo should be in ShooterCore or something)

        // Default path to the first entry in the UAbilitySystemGlobals::GameplayCueNotifyPaths.
        if ( IGameplayAbilitiesModule::IsAvailable() )
        {
            if ( auto & gameplay_abilities_module = IGameplayAbilitiesModule::Get();
                 gameplay_abilities_module.IsAbilitySystemGlobalsAvailable() )
            {
                auto * ability_system_globals = gameplay_abilities_module.GetAbilitySystemGlobals();
                check( ability_system_globals != nullptr );

                auto gameplay_cue_notify_paths = ability_system_globals->GetGameplayCueNotifyPaths();

                if ( gameplay_cue_notify_paths.Num() > 0 )
                {
                    path = gameplay_cue_notify_paths[ 0 ];
                }
            }
        }

        gameplay_cue_tag.RemoveFromStart( TEXT( "GameplayCue." ) );

        auto new_default_path_name = FString::Printf( TEXT( "%s/GCN_%s" ), *path, *gameplay_cue_tag );

        return new_default_path_name;
    }

    bool HasPlayWorld()
    {
        return GEditor->PlayWorld != nullptr;
    }

    bool HasNoPlayWorld()
    {
        return !HasPlayWorld();
    }

    bool CanShowCommonMaps()
    {
        return HasNoPlayWorld() && !GetDefault< UGameBaseFrameworkSettings >()->CommonEditorMaps.IsEmpty();
    }

    void OpenCommonMap_Clicked( const FString map_path )
    {
        if ( ensure( map_path.Len() ) )
        {
            GEditor->GetEditorSubsystem< UAssetEditorSubsystem >()->OpenEditorForAsset( map_path );
        }
    }

    TSharedRef< SWidget > GetCommonMapsDropdown()
    {
        FMenuBuilder menu_builder( true, nullptr );

        for ( const auto & path : GetDefault< UGameBaseFrameworkSettings >()->CommonEditorMaps )
        {
            if ( !path.IsValid() )
            {
                continue;
            }

            const FText display_name = FText::FromString( path.GetAssetName() );
            menu_builder.AddMenuEntry(
                display_name,
                LOCTEXT( "CommonPathDescription", "Opens this map in the editor" ),
                FSlateIcon(),
                FUIAction(
                    FExecuteAction::CreateStatic( &OpenCommonMap_Clicked, path.ToString() ),
                    FCanExecuteAction::CreateStatic( &HasNoPlayWorld ),
                    FIsActionChecked(),
                    FIsActionButtonVisible::CreateStatic( &HasNoPlayWorld ) ) );
        }

        return menu_builder.MakeWidget();
    }

    void RegisterGameEditorMenus()
    {
        auto * menu = UToolMenus::Get()->ExtendMenu( "LevelEditor.LevelEditorToolBar.PlayToolBar" );
        auto & section = menu->FindOrAddSection( "PlayGameExtensions" );
        section.InsertPosition.Name = "Play";
        section.InsertPosition.Position = EToolMenuInsertType::After;

        FToolMenuEntry common_map_entry = FToolMenuEntry::InitComboButton(
            "CommonMapOptions",
            FUIAction(
                FExecuteAction(),
                FCanExecuteAction::CreateStatic( &HasNoPlayWorld ),
                FIsActionChecked(),
                FIsActionButtonVisible::CreateStatic( &CanShowCommonMaps ) ),
            FOnGetContent::CreateStatic( &GetCommonMapsDropdown ),
            LOCTEXT( "CommonMaps_Label", "Common Maps" ),
            LOCTEXT( "CommonMaps_ToolTip", "Some commonly desired maps while using the editor" ),
            FSlateIcon( FAppStyle::GetAppStyleSetName(), "Icons.Level" ) );

        common_map_entry.StyleNameOverride = "CalloutToolbar";
        section.AddEntry( common_map_entry );
    }
}

class FGameBaseFrameworkEditorModule : public FDefaultGameModuleImpl
{
    typedef FGameBaseFrameworkEditorModule ThisClass;

    void StartupModule() override
    {
        FGBFGameEditorStyle::Initialize();

        if ( !IsRunningGame() )
        {
            FModuleManager::Get().OnModulesChanged().AddRaw( this, &ThisClass::ModulesChangedCallback );

            BindGameplayAbilitiesEditorDelegates();

            if ( FSlateApplication::IsInitialized() )
            {
                UToolMenus::RegisterStartupCallback( FSimpleMulticastDelegate::FDelegate::CreateStatic( &RegisterGameEditorMenus ) );
            }

            FEditorDelegates::BeginPIE.AddRaw( this, &ThisClass::OnBeginPIE );
            FEditorDelegates::EndPIE.AddRaw( this, &ThisClass::OnEndPIE );
        }
    }

    void OnBeginPIE( bool bIsSimulating )
    {
        // :NOTE: Is this used?
        /*auto * ExperienceManager = GEngine->GetEngineSubsystem< UGBFExperienceManager >();
        check( ExperienceManager );
        ExperienceManager->OnPlayInEditorBegun();*/
    }

    void OnEndPIE( bool bIsSimulating )
    {
    }

    void ShutdownModule() override
    {
        FModuleManager::Get().OnModulesChanged().RemoveAll( this );
    }

protected:
    static void BindGameplayAbilitiesEditorDelegates()
    {
        auto & gameplay_abilities_editor_module = IGameplayAbilitiesEditorModule::Get();

        gameplay_abilities_editor_module.GetGameplayCueNotifyClassesDelegate().BindStatic( &GetGameplayCueDefaultClasses );
        gameplay_abilities_editor_module.GetGameplayCueInterfaceClassesDelegate().BindStatic( &GetGameplayCueInterfaceClasses );
        gameplay_abilities_editor_module.GetGameplayCueNotifyPathDelegate().BindStatic( &GetGameplayCuePath );
    }

    void ModulesChangedCallback( const FName module_that_changed, const EModuleChangeReason reason_for_change )
    {
        if ( reason_for_change == EModuleChangeReason::ModuleLoaded && module_that_changed.ToString() == TEXT( "GameplayAbilitiesEditor" ) )
        {
            BindGameplayAbilitiesEditorDelegates();
        }
    }
};

IMPLEMENT_MODULE( FGameBaseFrameworkEditorModule, GBFEditorEngine );

#undef LOCTEXT_NAMESPACE