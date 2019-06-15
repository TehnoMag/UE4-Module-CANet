// Copyright (c) 2007-2019, Delta-Proxima Team All rights reserved.

#include "GameFramework/CANGameModeBase.h"
#include "GameFramework/CANPlayerController.h"
#include "GameFramework/CANPlayerState.h"
#include "Engine/ClientChannel.h"
#include "Engine/World.h"

ACANGameModeBase::ACANGameModeBase()
{
	bUseClientChannelForPlayerPawn = true;
	bCreateReflectionObjects = true;
	PlayerControllerClass = ACANPlayerController::StaticClass();
	PlayerStateClass = ACANPlayerState::StaticClass();
}

APawn* ACANGameModeBase::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	APawn* SpawnedPawn = nullptr;

	// Don't allow pawn to be spawned with any pitch or roll
	FRotator StartRotation(ForceInit);
	StartRotation.Yaw = StartSpot->GetActorRotation().Yaw;
	FVector StartLocation = StartSpot->GetActorLocation();

	FTransform Transform = FTransform(StartRotation, StartLocation);

	if (bUseClientChannelForPlayerPawn &&
		((GetNetMode() == NM_ListenServer && NewPlayer != GetWorld()->GetFirstPlayerController()) ||
			GetNetMode() == NM_DedicatedServer) 
		)
	{
		InitStartSpot(StartSpot, NewPlayer);
		SpawnDefaultPawnAtTransformWithClientChannel(NewPlayer, Transform);
	}
	else
	{
		SpawnedPawn = SpawnDefaultPawnAtTransform(NewPlayer, Transform);
	}

	return SpawnedPawn;
}

void ACANGameModeBase::SpawnDefaultPawnAtTransformWithClientChannel_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	ACANPlayerState* PlayerState = NewPlayer->GetPlayerState<ACANPlayerState>();
	UClass* PlayerClass = GetDefaultPawnClassForController(NewPlayer);
	UWorld* World = GetWorld();

	if (PlayerState)
	{
		ACANPlayerState::SpawnActorWithClientChannel(World, PlayerClass, SpawnTransform,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, true,
			bCreateReflectionObjects, PlayerState);
	}
}
