// Copyright (c) 2007-2019, Delta-Proxima Team All rights reserved.

#include "GameFramework/CANGameModeBase.h"
#include "GameFramework/CANPlayerState.h"
#include "Engine/ClientChannel.h"

ACANGameModeBase::ACANGameModeBase()
{
	bUseClientChannelForDefaultPawnClass = true;
	ClientChannelClass = UClientChannel::StaticClass();
}

APawn* ACANGameModeBase::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	APawn* SpawnedPawn = nullptr;

	// Don't allow pawn to be spawned with any pitch or roll
	FRotator StartRotation(ForceInit);
	StartRotation.Yaw = StartSpot->GetActorRotation().Yaw;
	FVector StartLocation = StartSpot->GetActorLocation();

	FTransform Transform = FTransform(StartRotation, StartLocation);

	//If we running on dedicated server and enable using client channel for player pawn then go different way
	if (GetNetMode() == NM_DedicatedServer && bUseClientChannelForDefaultPawnClass == true)
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
	ACANPlayerState* PlayerState = Cast<ACANPlayerState>(NewPlayer->PlayerState);

	if (PlayerState)
	{

	}

}
