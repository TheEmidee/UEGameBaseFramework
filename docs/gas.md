---
layout: default
title: GAS
nav_order: 5
has_children: true
permalink: /docs/gas
---

# Gameplay Ability System

How to integrate GAS in your game

## Config files

Add these entries in your `DefaultGame.ini`

```
[/Script/GameplayAbilities.AbilitySystemGlobals]
AbilitySystemGlobalsClassName="/Script/GameBaseFramework.GBFAbilitySystemGlobals"
GlobalGameplayCueManagerClass="/Script/GameBaseFramework.GBFGameplayCueManager"
```

You can of course create your own derived classes if you need.

## Attribute set initter

If you use the attribute set initter feature from GAS, you may find useful the class `FGBFAttributeSetInitterDiscreteLevels` which fixes some issues if you use curve tables that may not have values defined for each level. You can see this video : https://youtu.be/tc542u36JR0?si=Ng2QdFsuBT-UTHuR&t=1812

To use this, you need to create a subclass from `UGBFAbilitySystemGlobals` and override the function `AllocAttributeSetInitter` such as :

```C++
void UYGAbilitySystemGlobals::AllocAttributeSetInitter()
{
    GlobalAttributeSetInitter = MakeShared< FGBFAttributeSetInitterDiscreteLevels >( TEXT( "YG" ) );
}
```

The parameter you pass to the constructor through the `MakeShared` call is the prefix you use for your attribute set classes.

Indeed, the GameBaseFramework already contains some commonly used attribute sets, such as `UGBFHealthAttributeSet` but you may want to create a subclass of this attribute set, such as `UYGHealthAttributeSet`. Passing the `YG` token in the constructor allows the code to first look for attributes in a set that starts with `YG` before falling back to the parent attributes.

To get the attribute set initter to work, have a look at the comment above the class `FAttributeSetInitter` in the file `AttributeSet.h` of the engine.