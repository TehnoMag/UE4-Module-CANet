// Copyright (c) 2007-2019, Delta-Proxima Team All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Engine/ClientChannel.h"
#include "CANPlayerState.generated.h"

UCLASS()
class CANET_API ACANPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ACANPlayerState();

//* Begin Blueprint Interface

public:
	UFUNCTION(BlueprintCallable, Category = "Client Authority Network",
			Meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "4")
		)
		static void SpawnActorWithClientChannel(UObject* WorldContextObject, UClass* ActorClass, const FTransform& SpawnTransform,
			ESpawnActorCollisionHandlingMethod CollisionMethodOverride, bool PossessOnSpawn = false,
			bool CreateReflectionObject = false, ACANPlayerState* Spawner = nullptr,
			UClientChannel* OwnerActor = nullptr, UClientChannel* InstigatorPawn = nullptr);

//* End Blueprint Interface

//* Begin Networking

private:
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_SpawnActorWithClientChannel(const FClientChannelInfo& ChannelInfo, const FClientChannelSpawnInfo& SpawnInfo);

//* End Networking

};
