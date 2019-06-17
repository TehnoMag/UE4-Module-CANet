// Copyright (c) 2007-2019, Delta-Proxima Team All rights reserved.

#include "GameFramework/CANPlayerState.h"
#include "CANet.Definations.h"

#include "GameFramework/PlayerController.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

ACANPlayerState::ACANPlayerState()
{
	bAlwaysRelevant = true;
	NetUpdateFrequency = 100.f;
}

bool ACANPlayerState::SpawnActorWithClientChannel(UObject* WorldContext, TSubclassOf<AActor> ActorClass,
	const FTransform& SpawnTransform, ESpawnActorCollisionHandlingMethod CollisionHandlingOverride,
	ACANPlayerState* Player, bool AllowChangeAuthority, UClientChannel* OwnerChannel,
	UClientChannel* InstigatorChannel, bool PossesOnSpawn)
{
	UWorld* World = WorldContext->GetWorld();
	check(World);

	const ENetMode NetMode = World->GetNetMode();

	switch (NetMode)
	{
		case NM_Standalone:
		{
			UE_LOG(LogCANet, Error, TEXT("Can not spawn client channel in a standalone game"));
			return nullptr;
			break;
		}
		case NM_Client:
		{
			if (Player == nullptr) Player = Cast<ACANPlayerState>(World->GetFirstPlayerController()->PlayerState);
			break;
		}
		default:
		{
			if (Player == nullptr)
			{
#if WITH_EDITOR
				UFunction* ThisFunc = ACANPlayerState::StaticClass()->FindFunctionByName(TEXT("SpawnActorWithClientChannel"));
				LOG_PIE(ESeverity::Error, World, ThisFunc, TEXT("Player pin must be defined if calling on the server side"));
				return false;
#else
				checkf(Player, TEXT("Player pin must be defined if calling on the server side"));
				FPlatformMisc::RequestExit(true);
#endif
			}
			break;
		}
	}

	if (Player == nullptr)
	{
#if WITH_EDITOR
		UFunction* ThisFunc = ACANPlayerState::StaticClass()->FindFunctionByName(TEXT("SpawnActorWithClientChannel"));
		LOG_PIE(ESeverity::Error, World, ThisFunc, TEXT("Can not get valid PlayerState"));
		return nullptr;
#else
		checkf(Player, TEXT("Can not get valid PlayerState"));
		FPlatformMisc::RequestExit(true);
#endif
	}

	if (OwnerChannel != nullptr)
	{
		
	}

	if (InstigatorChannel != nullptr)
	{

	}

	FClientChannelInfo ChannelInfo;
	ChannelInfo.Class = ActorClass;
	ChannelInfo.bCanChangeAuthorityWhilePlay = AllowChangeAuthority;

	FClientChannelSpawnInfo SpawnInfo;
	SpawnInfo.bPossesOnSpawn = PossesOnSpawn;
	SpawnInfo.CollisionHandlingOverride = CollisionHandlingOverride;
	SpawnInfo.Instigator = InstigatorChannel;
	SpawnInfo.Owner = OwnerChannel;
	SpawnInfo.SpawnTransform = SpawnTransform;

	Player->Server_CreateClientChannel(ChannelInfo, SpawnInfo);

	return true;
}

bool ACANPlayerState::Server_CreateClientChannel_Validate(FClientChannelInfo ChannelInfo, FClientChannelSpawnInfo SpawnInfo)
{
	return true;
}


void ACANPlayerState::Server_CreateClientChannel_Implementation(FClientChannelInfo ChannelInfo, FClientChannelSpawnInfo SpawnInfo)
{
	FName ActorName = MakeUniqueObjectName(GetWorld(), ChannelInfo.Class);
	FName ChannelName = FName(*FString::Printf(TEXT("%s_Channel"), *ActorName.ToString()));

	UClientChannel* Channel = NewObject<UClientChannel>(this, UClientChannel::StaticClass(), ChannelName, RF_Transient);

	if (Channel)
	{
		ChannelInfo.Name = ChannelName;
		SpawnInfo.Name = ActorName;

		Channel->RegisterComponent();
		Channel->InitializeChannel(ChannelInfo, SpawnInfo);
		AddOwnedComponent(Channel);
		Channels.Add(Channel);
	}

}
