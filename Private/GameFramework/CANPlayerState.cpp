// Copyright (c) 2007-2019, Delta-Proxima Team All rights reserved.

#include "GameFramework/CANPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "CANet.Definations.h"

void ACANPlayerState::SpawnActorWithClientChannel(UObject* WorldContextObject, EClientChannelMode Authority, UClass* ActorClass, const FTransform& SpawnTransform,
	ESpawnActorCollisionHandlingMethod CollisionMethodOverride,
	ACANPlayerState* Spawner, UClientChannel* ActorOwner, UClientChannel* Instigator)
{
	UWorld* World = WorldContextObject->GetWorld();
	check(World);

	ACANPlayerState* PlayerState = nullptr;

	switch (World->GetNetMode())
	{
		case NM_Client:
		{
			
			// In this mode we can use this function only for Local Player, so we must ignore Spawner parameter
			PlayerState = Cast<ACANPlayerState>(World->GetFirstPlayerController()->PlayerState);
			break;
		}
		case NM_DedicatedServer:
		{
			if (Spawner == nullptr)
			{
				UE_LOG(LogCANet, Error, TEXT("Spawner parameter must be defined in SpawnActorWithClientChannel function if calling on a server"));
				return;
			}

			PlayerState = Spawner;

			break;
		}
		default:
		{
			UE_LOG(LogCANet, Error, TEXT("Can not use SpawnActorWithClientChannel function in current network mode"));
			return;
		}
	}

	// If Owner and/or Instigator are defined we must check it relations with Local Player

	if (ActorOwner != nullptr)
	{
		if (ActorOwner->GetOwner() != PlayerState)
		{
			UE_LOG(LogCANet, Warning, TEXT("Can not assign Channel %s as owner, because channel owner is invalid"), *ActorOwner->GetName());
			ActorOwner = nullptr;
		}
	}

	if (Instigator != nullptr)
	{
		if (Instigator->GetOwner() != PlayerState)
		{
			UE_LOG(LogCANet, Warning, TEXT("Can not assign Channel %s as Instigator, because channel owner is invalid"), *Instigator->GetName());
			Instigator = nullptr;
		}
		else if (!Instigator->GetViewActor()->IsA<APawn>())
		{
			UE_LOG(LogCANet, Warning, TEXT("Instigator %s for a channel %s has invalid class"), 
				*Instigator->GetViewActor()->GetName(), *ActorOwner->GetName());
			Instigator = nullptr;
		}
	}

	//Check if spawning actor are available for replication
	AActor* TemplateActor = ActorClass->GetDefaultObject<AActor>();

	if (PlayerState && TemplateActor->GetIsReplicated())
	{
		FClientChannelInfo ChannelInfo;
		ChannelInfo.AuthorityMode = Authority;
		ChannelInfo.Class = ActorClass;
		ChannelInfo.SpawnTransform = SpawnTransform;
		ChannelInfo.CollisionMethodOverride = CollisionMethodOverride;
		ChannelInfo.Owner = ActorOwner;
		ChannelInfo.Instigator = Instigator;
		ChannelInfo.bPossesOnSpawn = (PlayerState->GetPawn()) ? false : true;
		PlayerState->Server_SpawnActorWithClientChannel(ChannelInfo);
	}
	else
	{
		UE_LOG(LogCANet, Error, TEXT("Actor`s Class %s is not available for replication"), *ActorClass->GetName());
	}
}

ACANPlayerState::ACANPlayerState()
{
	bAlwaysRelevant = true;
	NetUpdateFrequency = 100.f;
}

bool ACANPlayerState::Server_SpawnActorWithClientChannel_Validate(const FClientChannelInfo& ChannelInfo)
{
	return true;
}

void ACANPlayerState::Server_SpawnActorWithClientChannel_Implementation(const FClientChannelInfo& ChannelInfo)
{
	if (GetWorld()->GetNetMode() != NM_DedicatedServer)
		return;

	FName ChannelName = MakeUniqueObjectName(GetOuter(), UClientChannel::StaticClass());
	UClientChannel* newChannel = NewObject<UClientChannel>(this, UClientChannel::StaticClass(), ChannelName, RF_Transient);
	newChannel->RegisterComponent();
	newChannel->InitializeChannel(ChannelInfo);
	AddOwnedComponent(newChannel);
}
