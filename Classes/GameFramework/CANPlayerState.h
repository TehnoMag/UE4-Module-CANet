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

private:
	TArray<UClientChannel*> Channels;
	
//~ Begin Network

private:
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_SpawnActorWithClientChannel(const FClientChannelInfo& ChannelInfo);

//~ End Network

//~ Begin Blueprint Interface

public:
	UFUNCTION(BlueprintCallable, Category = "Client Authority Network",
		Meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "5")
	)
	static void SpawnActorWithClientChannel(UObject* WorldContextObject, EClientChannelMode Authority, UClass* ActorClass, const FTransform& SpawnTransform,
		ESpawnActorCollisionHandlingMethod CollisionMethodOverride,
		ACANPlayerState* Spawner = nullptr, UClientChannel* OwnerActor = nullptr, UClientChannel* InstigatorPawn = nullptr);

//~ End Blueprint Interface

};
