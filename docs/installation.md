---
layout: default
title: Installation
nav_order: 2
---

# Installation
{: .no_toc }

The easiest way to add this plugin to an Unreal Engine project is to add it as a submodule:

`git submodule add https://github.com/TheEmidee/UEGameBaseFramework.git Plugins/GameBaseFramework`

You can also download the repository as a zip file and extract it in the Plugins folder.

# Dependencies

This plugin has dependencies with other plugins. You will need to install those also either as submodules of your project, or by unzipping them in the Plugins folder:

* ModularGameplayActors : https://github.com/TheEmidee/UEModularGameplayActors
* GameplayMessageRouter : https://github.com/TheEmidee/UEGameplayMessageRouter
* CommonGame : https://github.com/TheEmidee/UECommonGame
* CommonLoadingScreen : https://github.com/TheEmidee/UECommonLoadingScreen

{: .fs-6 .fw-300 }