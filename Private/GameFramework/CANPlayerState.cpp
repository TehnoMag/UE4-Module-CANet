// Copyright (c) 2007-2019, Delta-Proxima Team All rights reserved.

#include "GameFramework/CANPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

void ACANPlayerState::SpawnActorWithClientChannel(UObject* WorldContextObject, EClientChannelMode Authority, UClass* ActorClass, const FTransform& SpawnTransform,
	ESpawnActorCollisionHandlingMethod CollisionMethodOverride, APlayerState* Spawner, AActor* ActorOwner, APawn* Instigator)
{
	UWorld* World = WorldContextObject->GetWorld();
	check(World);

	if (World->GetNetMode() == NM_Client)
	{
		ACANPlayerState* PlayerState = nullptr;

		if (Spawner != nullptr && Spawner->IsA<ACANPlayerState>())
		{
			PlayerState = Cast<ACANPlayerState>(Spawner);
		}
		else
		{
			PlayerState = Cast<ACANPlayerState>(World->GetFirstPlayerController()->PlayerState);
		}

		if (PlayerState)
		{
			FClientChannelInfo ChannelInfo;
			ChannelInfo.AuthorityMode = Authority;
			ChannelInfo.Class = ActorClass;
			ChannelInfo.SpawnTransform = SpawnTransform;
			ChannelInfo.CollisionMethodOverride = CollisionMethodOverride;
			ChannelInfo.Owner = nullptr;
			ChannelInfo.Instigator = nullptr;
			PlayerState->Server_SpawnActorWithClientChannel(ChannelInfo);
		}
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
