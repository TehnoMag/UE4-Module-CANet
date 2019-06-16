// Copyright (c) 2007-2019, Delta-Proxima Team All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CANPlayerState.generated.h"

class UClientChannel;

UCLASS()
class CANET_API ACANPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ACANPlayerState();

//* Begin Blueprint interface

public:

	/**
	 * Spawn actor with client authority channel
	 *
	 * @param	ActorClass				The object class you want to construct
	 * @param	SpawnTransform			The transform to spawn the actor with
	 * @param	CollisionMethodOverride	Specifies how to handle collisions at the spawn point. If undefined, uses actor class settings
	 * @param	Player					The player who will get authority control of this actor. May be null if running on client
	 * @param	OwnerChannel			The client channel who has owner rights to this actors channel. May be null.
	 * @param	InstigatorChannel		The channel that is controll APawn that is responsible for damage done by the spawned Actor. May be null.
	 *
	 * @return	Spawned actor or reflection object
	 */
	UFUNCTION(BlueprintCallable, Category = "ClientAuthorityNetwork",
		Meta = (
			WorldContext = "WorldContext",
			DeterminesOutputType = "ActorClass",
			AdvancedDisplay = "5"
			))
		static AActor* SpawnActorWithClientChannel(UObject* WorldContext, TSubclassOf<AActor> ActorClass,
			const FTransform& SpawnTransform, ESpawnActorCollisionHandlingMethod CollisionHandlingOverride,
			APlayerState* Player = nullptr, UClientChannel* OwnerChannel = nullptr, UClientChannel* InstigatorChannel = nullptr);

//* End Blueprint interface

};
