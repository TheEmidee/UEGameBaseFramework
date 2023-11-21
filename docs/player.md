---
layout: default
title: Player Setup
nav_order: 5
has_children: true
permalink: /docs/player
---

# Character actor

* Create a character blueprint inheriting from your own Character class (which must inherit from `AGBFCharacter`)
  * Create a C++ class inheriting from `UGBFHeroComponent`. You only have to override `BindNativeActions` to register native functions with input tags
  * Give the hero component to the player actor

# Pawn Data Asset

* Create a pawn data asset
  * Link the character blueprint
  * Create and link a AbilityTagRelationshipMapping asset to automatically block or cancel abilities based on some tags
  * Create and link an InputConfig asset to bind input tags with an Enhanced Input action
  * Optional :
    * Link a defaut camera mode
    * Add default camera modifiers

You will have to create an experience definition to link the pawn data asset, and have that experience be loaded by the game mode.

# Input

You must update the `Engine - Input` settings to set the `Default Input Component Class` to `GBFInputComponent`

{: .fs-6 .fw-300 }