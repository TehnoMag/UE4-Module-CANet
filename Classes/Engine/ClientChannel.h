// Copyright (c) 2007-2019, Delta-Proxima Team All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ClientChannel.generated.h"

class UClientChannel;

USTRUCT()
struct FClientChannelInfo
{
	GENERATED_BODY()

public:
	UPROPERTY()
		UClass* Class;

	UPROPERTY()
		FName Name;

	UPROPERTY()
		uint8 bCanChangeAuthorityWhilePlay : 1;
};

USTRUCT()
struct FClientChannelSpawnInfo
{
	GENERATED_BODY()

public:
	UPROPERTY()
		FTransform SpawnTransform;

	UPROPERTY()
		FName Name;

	UPROPERTY()
		ESpawnActorCollisionHandlingMethod CollisionHandlingOverride;

	UPROPERTY()
		UClientChannel* Owner;

	UPROPERTY()
		UClientChannel* Instigator;

	UPROPERTY()
		uint8 bPossesOnSpawn : 1;
};

UCLASS()
class CANET_API UClientChannel : public UActorComponent
{
	GENERATED_BODY()

public:
	UClientChannel();
	void InitializeChannel(const FClientChannelInfo& ChannelInfo, const FClientChannelSpawnInfo& SpawnInfo);

};
