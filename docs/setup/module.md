---
layout: default
title: Module
parent: Setup
nav_order: 1
---

# Game Module

{: .no_toc }

You need to edit the `Build.cs` module file of your game project, and add the following entries to the `PublicDependencyModuleNames` property:

```
"EnhancedInput",
"CommonGame",
"CommonUser",
"GameBaseFramework",
"ModularGameplayActors",
"GameplayAbilities"
```