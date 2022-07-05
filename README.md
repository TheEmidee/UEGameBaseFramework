# UE4GameBaseFramework

## Conditional Event System

The conditional event system is a GAS based system that waits for certain triggers and then applies outcomes.

### GBF Conditional Event Subsystem

The system uses a World Subsystem that exposes functions to `Activate` or `Deactivate` `Events` or `Event Groups`.

### GBF Conditional Event Group Data

This data asset holds an array of Events to be activated.

### GBF Conditional Event Ability

This Gameplay Ability that allows to specify `Triggers`. When all of these triggers have been triggered, the event `ExecuteOutcomes` will be called.
This function can be added to the blueprint editor to specify the outcomes.

### GBF Conditional Trigger

This trigger has an `Activate` and `Deactivate` function, which are automatically called by the `Conditional Event Ability`.
It also has a `OnTriggeredDelegate` which is used to determine if a trigger has been triggered by the system.

This class should be overwritten for as many cases as needed. There are already two child classes:
- `GBF Time Passed Trigger`, which triggers its delegate after a certain amount of time.
- `GBF Trigger Box Trigger`, which waits until a trigger box has been triggered to trigger its delegate.