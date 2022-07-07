# UE4GameBaseFramework

## Conditional Event System

The conditional event system is a GAS based system that waits for certain triggers and then applies outcomes.

### GBF Conditional Event Subsystem

The system uses a World Subsystem that exposes functions to `Activate` or `Deactivate` `Events` or `Event Groups`.
These functions should be used by the user to start tracking events and wait for their outcomes.

### GBF Conditional Event Group Data

This data asset holds an array of Events to be activated.
A blueprint data asset should be created to pass to the functions of the `Conditional Event Subsystem`.

### GBF Conditional Event Ability

This Gameplay Ability that allows to specify `Triggers`. When all of these triggers have been triggered, the event `ExecuteOutcomes` will be called.
This function can be added to the blueprint editor to specify the outcomes.
Blueprint GA's should be created to either store in the `Conditional Event Group Data` or to be directly passed to the `Conditional Event Subsystem`.

### GBF Conditional Trigger

This trigger has an `Activate` and `Deactivate` function, which are automatically called by the `Conditional Event Ability`.
It also has a `OnTriggeredDelegate` which is used to determine if a trigger has been triggered by the system.

This class should be inherited from for as many cases as needed, but can only be done in C++.
To inherit correctly, one should implement both `Activate` and `Deactivate`.

There are already two child classes:
- `GBF Time Passed Trigger`, which triggers its delegate after a certain amount of time.
- `GBF Trigger Box Trigger`, which waits until a trigger box has been triggered to trigger its delegate.