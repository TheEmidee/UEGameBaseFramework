#include "GameBaseFrameworkSettings.h"
#include "IGameBaseFrameworkModule.h"
#include "Input/GBFInputTypes.h"

#include <Engine/Texture2D.h>

IMPLEMENT_MODULE( IGameBaseFrameworkModule, GameBaseFramework )

void IGameBaseFrameworkModule::StartupModule()
{
    LoadAllPlatformInputTextures();
}

void IGameBaseFrameworkModule::ShutdownModule()
{
}

UTexture2D * IGameBaseFrameworkModule::GetPlatformInputTextureForKey( const FString & platform_name, const FKey & key )
{
    if ( auto * key_to_texture_map = PlatformInputTexturesMap.Find( platform_name ) )
    {
        if ( auto * texture = key_to_texture_map->Find( key ) )
        {
            return *texture;
        }
    }

    return nullptr;
}

#if WITH_EDITOR
void IGameBaseFrameworkModule::RefreshPlatformInputTextures()
{
    PlatformInputTexturesMap.Reset();
    LoadAllPlatformInputTextures();
}
#endif

// -- PRIVATE

void IGameBaseFrameworkModule::LoadAllPlatformInputTextures()
{
    if ( auto * settings = GetDefault< UGameBaseFrameworkSettings >() )
    {
        // ReSharper disable once CppExpressionWithoutSideEffects
        settings->PlatformInputTextures.LoadSynchronous();

#if PLATFORM_DESKTOP
        LoadPlatformInputTextures( "Keyboard" );
        LoadPlatformInputTextures( "Desktop" );
#else
        LoadPlatformInputTextures( UGameplayStatics::GetPlatformName() );
#endif
    }
}

void IGameBaseFrameworkModule::LoadPlatformInputTextures( const FString & platform_input_name )
{
    if ( auto * settings = GetDefault< UGameBaseFrameworkSettings >() )
    {
        if ( !settings->PlatformInputTextures.IsValid() )
        {
            return;
        }

        if ( auto * value = settings->PlatformInputTextures->PlatformInputToTextureMap.Find( platform_input_name ) )
        {
            if ( auto * data_table = value->LoadSynchronous() )
            {
                auto & key_to_texture_map = PlatformInputTexturesMap.Add( platform_input_name );

                TArray< FGBFPlatformInputTextureData * > platform_input_texture_data_table;
                static const FString context_string( TEXT( "GENERAL" ) );
                data_table->GetAllRows( context_string, platform_input_texture_data_table );

                for ( auto * platform_input_texture_data_row : platform_input_texture_data_table )
                {
                    key_to_texture_map.Add( platform_input_texture_data_row->Key, platform_input_texture_data_row->Texture.LoadSynchronous() );
                }
            }
        }
    }
}
