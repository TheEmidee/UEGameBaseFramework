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
* GBFHUD
* GBFGameSession

You can update your game mode class constructor to use your classes by default:

```
GameSessionClass = AGBFGameSession::StaticClass();
GameStateClass = AGBFGameState::StaticClass();
GameSessionClass = AGBFGameSession::StaticClass();
PlayerControllerClass = AGBFPlayerController::StaticClass();
PlayerStateClass = AGBFPlayerState::StaticClass();
DefaultPawnClass = AGBFCharacter::StaticClass();
HUDClass = AGBFHUD::StaticClass();
```