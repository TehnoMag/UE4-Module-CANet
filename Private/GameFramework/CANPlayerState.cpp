// Copyright (c) 2007-2019, Delta-Proxima Team All rights reserved.

#include "GameFramework/CANPlayerState.h"
#include "CANet.Definations.h"
#include "Engine/ClientChannel.h"

#include "GameFramework/PlayerController.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

ACANPlayerState::ACANPlayerState()
{
	bAlwaysRelevant = true;
	NetUpdateFrequency = 100.f;
}

AActor* ACANPlayerState::SpawnActorWithClientChannel(UObject* WorldContext, TSubclassOf<AActor> ActorClass,
	const FTransform& SpawnTransform, ESpawnActorCollisionHandlingMethod CollisionHandlingOverride,
	APlayerState* Player, UClientChannel* OwnerChannel, UClientChannel* InstigatorChannel)
{
	UWorld* World = WorldContext->GetWorld();
	check(World);

	const ENetMode NetMode = World->GetNetMode();

	if (NetMode == NM_Standalone)
	{
		UE_LOG(LogCANet, Error, TEXT("Can not spawn client channel in a standalone game"));
		return nullptr;
	}
	else if (NetMode == NM_Client)
	{
		if (Player == nullptr)
		{
			Player = World->GetFirstPlayerController()->PlayerState;
		}
	}
	else
	{
		if (Player == nullptr)
		{
#if WITH_EDITOR
			UFunction* ThisFunc = ACANPlayerState::StaticClass()->FindFunctionByName(TEXT("SpawnActorWithClientChannel"));
			LOG_PIE(ESeverity::Error, World, ThisFunc, TEXT("Player pin must be defined if calling on the server side"));
			return nullptr;
#else
			checkf(Player, TEXT("Player pin must be defined if calling on the server side"));
			FPlatformMisc::RequestExit(true);
#endif
		}
	}

	return nullptr;
}
