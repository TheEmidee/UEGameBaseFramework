---
layout: default
title: Classes to implement
parent: Setup
nav_order: 2
---

# Classes

{: .no_toc }

It is recommend you create child classes from the following classes of the plugin:

* GBFPlayerState
* GBFGameState
* GBFLocalPlayer
* GBFGameInstance
* GBFCharacter
* GBFPlayerController
* GBFGameMode
* GBFGameUserSettings
* GBFHUD

You can update your game mode class constructor to use your classes by default:

```c++
GameSessionClass = AYourGameGameSession::StaticClass();
GameStateClass = AYourGameGameState::StaticClass();
GameSessionClass = AYourGameGameSession::StaticClass();
PlayerControllerClass = AYourGamePlayerController::StaticClass();
PlayerStateClass = AYourGamePlayerState::StaticClass();
DefaultPawnClass = AYourGameCharacter::StaticClass();
HUDClass = AYourGameHUD::StaticClass();
```

And update your `DefaultEngine.ini` to register the other:

```ini
[/Script/Engine.Engine]
LocalPlayerClassName=/Script/YOURGAME.LocalPlayer
GameUserSettingsClassName=/Script/YOURGAME.SettingsLocal
```

You will also have to register the world settings and asset manager classes with those from the GameBaseFramework plugin, unless you want to subclass those in your project:

```ini
[/Script/Engine.Engine]
WorldSettingsClassName=/Script/GameBaseFramework.GBFWorldSettings
AssetManagerClassName=/Script/GameBaseFramework.GBFAssetManager
```