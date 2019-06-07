// Copyright (c) 2007-2019, Delta-Proxima Team All rights reserved.

#include "GameFramework/CANGameModeBase.h"
#include "GameFramework/CANPlayerController.h"
#include "GameFramework/CANPlayerState.h"

ACANGameModeBase::ACANGameModeBase()
{
	PlayerControllerClass = ACANPlayerController::StaticClass();
	PlayerStateClass = ACANPlayerState::StaticClass();
}
