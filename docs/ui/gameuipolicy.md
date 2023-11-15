---
layout: default
title: Game UI Policy
parent: Common UI
nav_order: 2
---

# Game UI Policy

Create a blueprint from the class `GameUIPolicy` and tell `GBFGameUIManagerSubsystem` to use it with `DefaultGame.ini` : 

```ini
[/Script/GameBaseFramework.GBFGameUIManagerSubsystem]
DefaultUIPolicyClass=/Game/YOURGAME/UI/BP_GameUIPolicy.BP_GameUIPolicy_C
```

Then create a widget blueprint that inherits from `PrimaryGameLayout` layout class. You can mimic the one in Lyra, that has 4 `CommonActivatableWidgetStack` children to handle the game layer, the game menu, the menu, and the modal elements.

You will have to reference that game layout in your game policy blueprint.

You must also register an input action for the tag `UI.Action.Escape` to make widgets such as the HUD work correctly.

Go in the `Project Settings` and select the category `Common UI Input Settings` then add an entry in the `Input Actions` array. Define the `Action Tag` with `UI.Action.Escape`, set the `Default Display Name` to `Back` and add the key mappings you want the system to use.

You can for example use `Escape` and `Gamepad Special Right` as in Lyra.