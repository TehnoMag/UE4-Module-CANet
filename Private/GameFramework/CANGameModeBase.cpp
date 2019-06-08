// Copyright (c) 2007-2019, Delta-Proxima Team All rights reserved.

#include "GameFramework/CANGameModeBase.h"
#include "GameFramework/CANPlayerController.h"
#include "GameFramework/CANPlayerState.h"
#include "Engine/ClientChannel.h"
#include "Engine/World.h"

ACANGameModeBase::ACANGameModeBase()
{
	bUseClientChannelForPlayerPawn = true;
	PlayerControllerClass = ACANPlayerController::StaticClass();
	PlayerStateClass = ACANPlayerState::StaticClass();
}

APawn* ACANGameModeBase::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	// Don't allow pawn to be spawned with any pitch or roll
	FRotator StartRotation(ForceInit);
	StartRotation.Yaw = StartSpot->GetActorRotation().Yaw;
	FVector StartLocation = StartSpot->GetActorLocation();

	FTransform Transform = FTransform(StartRotation, StartLocation);
	
	if (bUseClientChannelForPlayerPawn)
	{
		SpawnDefaultPawnAtTransformWithClientChannel(NewPlayer, Transform);
		InitStartSpot(StartSpot, NewPlayer);
		return nullptr;
	}
	else
	{
		return SpawnDefaultPawnAtTransform(NewPlayer, Transform);
	}
}

void ACANGameModeBase::SpawnDefaultPawnAtTransformWithClientChannel_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	ACANPlayerState* Spawner = Cast<ACANPlayerState>(NewPlayer->PlayerState);

	if (Spawner)
	{
		ACANPlayerState::SpawnActorWithClientChannel(
			GetWorld(), EClientChannelMode::CCM_OWNER, GetDefaultPawnClassForController(NewPlayer), SpawnTransform,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, Spawner);
	}
}
