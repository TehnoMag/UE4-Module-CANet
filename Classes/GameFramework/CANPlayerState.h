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
	TSet<UClientChannel*> Channels;

//* Begin Blueprint interface

public:

	/**
	 * Spawn actor with client authority channel
	 *
	 * @param	ActorClass				The object class you want to construct
	 * @param	SpawnTransform			The transform to spawn the actor with
	 * @param	CollisionMethodOverride	Specifies how to handle collisions at the spawn point. If undefined, uses actor class settings
	 * @param	Player					The player who will get authority control of this actor. May be null if running on client
	 * @param	AllowChangeAuthority	If enabled, then allow server select new authority if authority client are missing.
	 * @param	OwnerChannel			The client channel who has owner rights to this actors channel. May be null.
	 * @param	InstigatorChannel		The channel that is controll APawn that is responsible for damage done by the spawned Actor. May be null.
	 * @param	PossesOnSpawn			If enabled, then local controller start possesing with new actor.
	 *
	 * @return	Return true if success or false if not
	 */
	UFUNCTION(BlueprintCallable, Category = "ClientAuthorityNetwork",
		Meta = (
			WorldContext = "WorldContext",
			AdvancedDisplay = "5"
			))
		static bool SpawnActorWithClientChannel(UObject* WorldContext, TSubclassOf<AActor> ActorClass,
			const FTransform& SpawnTransform, ESpawnActorCollisionHandlingMethod CollisionHandlingOverride,
			ACANPlayerState* Player = nullptr, bool AllowChangeAuthority = false, UClientChannel* OwnerChannel = nullptr,
			UClientChannel* InstigatorChannel = nullptr, bool PossesOnSpawn = false);

	UFUNCTION(BlueprintCallable, Category = "ClientAuthorityNetwork")
		static TSet<UClientChannel*> GetChannelsForPlayer(ACANPlayerState* Player) { return Player->Channels; };

//* End Blueprint interface

//* Begin Networking

protected:
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_CreateClientChannel(FClientChannelInfo ChannelInfo, FClientChannelSpawnInfo SpawnInfo);

//* End Networking

};
