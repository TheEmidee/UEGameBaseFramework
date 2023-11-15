---
layout: default
title: HUD
parent: Common UI
nav_order: 3
---

# HUD

Create a widget inheriting from `GBFHUDLayout`

Add the HUD to your UI using the game feature action `Add Widgets`

Using this parent class allows you to automatically register an escape input binding, and display a game menu when this input is pressed.

You can define the game menu to show up using the property `Escape Menu Class`

As in Lyra, you can make your HUD fully modular by defining extension points in your widget hierarchy. Each extension point is defined by a gameplay tag. You can use the `Add Widgets` game feature action to fill each extension point by a specific widget.