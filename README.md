# Unreal Engine4 - Client Authority Network module

## About
This module will allow you to create games with a client-server network model, where the player is the full owner of objects and fully controls the rules of their replication to other clients. The server in this case acts only as a router and validator of the transmitted data.

## Installation
- Clone latest sources into **Source/CANet** directory of you project
- Edit **{ProjectName}Editor.Target.cs** file in **Source** directory and add next code into *{ProjectName}EditorTarget* function
```c#
ExtraModuleNames.AddRange(new string[] { "CANet" });
```
- Into **{ProjectName}.uproject** add next code into Modules section
```json
{
  "Name": "CANet",
  "Type": "Runtime",
  "LoadingPhase": "Default"
}
```
- Recompile you project

## Usage
### Dedicated Server
*Todo*

### Listen Server
**Limited supported**

### Standalone game
**This game mode are not supported**
