---
layout: default
title: Game Settings
nav_order: 9
has_children: true
permalink: /docs/settings
---

# Setup

Create a class that inherits from `GBFGameUserSettings` and register it in `DefaultEngine.ini`:

```ini
[/Script/Engine.Engine]
GameUserSettingsClassName=/Script/YOURGAME.GameUserSettings
```

# Game Settings Screen

Use the class `GBFGameSettingScreen` as the parent for your own C++ widget, then create a widget our this class

Your game must implement the function `UGameSettingRegistry * CreateRegistry() override` in the game settings screen.

Tell the engine to use this class as the default game user settings class by updating the `DefaultEngine.ini` file :

```ini
[/Script/Engine.Engine]
GameUserSettingsClassName=/Script/YOURGAME.YOURCLASSNAME
```

# Game Settings Registry

Your game should create it’s derived class from `UGBFGameSettingRegistry` which will automatically save the local and shared settings. It also creates the basic options, to be saved in the aforementioned settings

The registry has many virtual functions to add your own settings

You must create a widget of type `GameSettingListView` and add that widget inside your game settings panel widget.

You must create an asset of type `GameSettingVisualData` in which you define for each type of `GameSetting` which widget to use.

You must reference this asset in the game settings list view widget.

https://docs.unrealengine.com/5.0/en-US/lyra-sample-game-settings-in-unreal-engine/

You will need to implement widgets as in Lyra