---
layout: default
title: Settings
parent: Common UI
nav_order: 1
---

# Settings

## GameViewPort

You must update your `DefaultEngine.ini` to register the game viewport client class:

```ini
[/Script/Engine.Engine]
GameViewportClientClassName=/Script/GameBaseFramework.GBFGameViewportClient
```

## Platform traits

You must update the file `DefaultGame.ini` with these entries:

```ini
[/Script/CommonUI.CommonUISettings]
bAutoLoadData=True
DefaultImageResourceObject=None
+PlatformTraits=(TagName="Platform.Trait.SupportsWindowedMode")
+PlatformTraits=(TagName="Platform.Trait.CanExitApplication")
+PlatformTraits=(TagName="Platform.Trait.SupportsChangingAudioOutputDevice")
+PlatformTraits=(TagName="Platform.Trait.SupportsBackgroundAudio")
+PlatformTraits=(TagName="Platform.Trait.Input.SupportsMouseAndKeyboard")
+PlatformTraits=(TagName="Platform.Trait.Input.HardwareCursor")
+PlatformTraits=(TagName="Platform.Trait.NeedsBrightnessAdjustment")
```

You will also have to add those same tags in for example `DefaultGameplayTags.ini`

## Common Input Settings

Create a blueprint from the class `CommonUIInputData`, then a data table from the struct `CommonUIInputDataBase`, which you can fill as in Lyra. Then you need to link that data table in the common ui data blueprint you created before.

Then you will need to update the `Platform Input` table. You can follow Lyra here again, which has some predefined assets you can copy, that do the binding between the various controllers and their visual representation in the UI.

## Common UI Input Settings

You must also register an input action for the tag `UI.Action.Escape` to make widgets such as the HUD work correctly.

Go in the `Project Settings` and select the category `Common UI Input Settings` then add an entry in the `Input Actions` array. Define the `Action Tag` with `UI.Action.Escape`, set the `Default Display Name` to `Back` and add the key mappings you want the system to use.

You can for example use `Escape` and `Gamepad Special Right` as in Lyra.

# Classes

You must implement your own LocalPlayer class, that inherits from `CommonLocalPlayer` and register it in `DefaultEngine.ini`

```ini
[/Script/Engine.Engine]
LocalPlayerClassName=/Script/YOURGAME.LocalPlayer
```