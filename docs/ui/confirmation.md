---
layout: default
title: Confirmation Dialogs
parent: Common UI
nav_order: 4
---

# Confirmation Dialog

The widget parent class must be `GBFConfirmationDialog`, and it must have the following widgets to be bound to:

```c++
UPROPERTY( Meta = ( BindWidget ) )
TObjectPtr< UCommonTextBlock > Text_Title;

UPROPERTY( Meta = ( BindWidget ) )
TObjectPtr< UCommonRichTextBlock > RichText_Description;

UPROPERTY( Meta = ( BindWidget ) )
TObjectPtr< UDynamicEntryBox > EntryBox_Buttons;

UPROPERTY( Meta = ( BindWidget ) )
TObjectPtr< UCommonBorder > Border_TapToCloseZone;
```
    
There must be a match in the CommonUI data tables for the CancelAction. (See the Input section)

```C++
UPROPERTY( EditDefaultsOnly, meta = ( RowType = "/Script/CommonUI.CommonInputActionDataBase" ) )
FDataTableRowHandle CancelAction
```

You need to register a confirmation dialog class, and an error dialog class, in the `Project Settings > GameBaseFramework - Settings` section