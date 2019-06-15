// Copyright (c) 2007-2019, Delta-Proxima Team All rights reserved.

#include "GameFramework/CANPlayerState.h"
#include "CANet.Definations.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

ACANPlayerState::ACANPlayerState()
{
	bAlwaysRelevant = true;
	NetUpdateFrequency = 100.f;
}

void ACANPlayerState::SpawnActorWithClientChannel(UObject* WorldContextObject, UClass* ActorClass, const FTransform& SpawnTransform,
	ESpawnActorCollisionHandlingMethod CollisionMethodOverride, bool PossessOnSpawn,
	bool CreateReflectionObject, ACANPlayerState* Spawner,
	UClientChannel* OwnerActor, UClientChannel* InstigatorPawn)
{
	UWorld* World = WorldContextObject->GetWorld();
	check(World);

	ACANPlayerState* PlayerState = nullptr;

	if (World->GetNetMode() != NM_Standalone)
	{
		if (World->GetNetMode() == NM_Client)
		{
			PlayerState = Cast<ACANPlayerState>(World->GetFirstPlayerController()->PlayerState);
		}
		else
		{
			PlayerState = Spawner;
		}
	}

	if (OwnerActor != nullptr)
	{
		if (OwnerActor->GetOwner() != PlayerState)
		{
			UE_LOG(LogCANet, Warning, TEXT("Can not assign Channel %s as owner, because channel owner is invalid"), *OwnerActor->GetName());
			OwnerActor = nullptr;
		}
	}

	if (InstigatorPawn != nullptr)
	{
		if (InstigatorPawn->GetOwner() != PlayerState)
		{
			UE_LOG(LogCANet, Warning, TEXT("Can not assign Channel %s as Instigator, because channel owner is invalid"), *InstigatorPawn->GetName());
			InstigatorPawn = nullptr;
		}
		/*else if (!InstigatorPawn->GetViewActor()->IsA<APawn>())
		{
			UE_LOG(LogCANet, Warning, TEXT("Instigator %s for a channel %s has invalid class"),
				*InstigatorPawn->GetViewActor()->GetName(), *ActorOwner->GetName());
			InstigatorPawn = nullptr;
		}*/
	}

	AActor* TemplateActor = ActorClass->GetDefaultObject<AActor>();

	if (PlayerState && TemplateActor->GetIsReplicated())
	{
		FClientChannelInfo ChannelInfo;
		ChannelInfo.bUseReflectObject = CreateReflectionObject;
		ChannelInfo.Class = ActorClass;
		ChannelInfo.Owner = OwnerActor;
		ChannelInfo.Instigator = InstigatorPawn;
		ChannelInfo.ActorName = MakeUniqueObjectName(PlayerState->GetOuter(), ActorClass);

		FClientChannelSpawnInfo SpawnInfo;
		SpawnInfo.SpawnTransform = SpawnTransform;
		SpawnInfo.CollisionMethodOverride = CollisionMethodOverride;
		SpawnInfo.bPossessOnSpawn = PossessOnSpawn;

		PlayerState->Server_SpawnActorWithClientChannel(ChannelInfo, SpawnInfo);
	}
	else
	{
		UE_LOG(LogCANet, Error, TEXT("Actor`s Class %s is not available for replication"), *ActorClass->GetName());
	}
}

bool ACANPlayerState::Server_SpawnActorWithClientChannel_Validate(const FClientChannelInfo& ChannelInfo, const FClientChannelSpawnInfo& SpawnInfo)
{
	return true;
}

void ACANPlayerState::Server_SpawnActorWithClientChannel_Implementation(const FClientChannelInfo& ChannelInfo, const FClientChannelSpawnInfo& SpawnInfo)
{
	if (GetWorld()->GetNetMode() == NM_Standalone)
		return;

	// Make sure that's all names on server and clients are sync
	FName ChannelName = MakeUniqueObjectName(GetOuter(), UClientChannel::StaticClass(), ChannelInfo.ActorName);

	UClientChannel* Channel = NewObject<UClientChannel>(this, UClientChannel::StaticClass(), ChannelName, RF_Transient);
	Channel->RegisterComponent();
	Channel->InitializeChannel(ChannelInfo, SpawnInfo);
	AddOwnedComponent(Channel);
}
