// Copyright (c) 2007-2019, Delta-Proxima Team All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/NetworkObjectList.h"
#include "Net/RepLayout.h"
#include "ClientChannel.generated.h"

class UClientChannel;

USTRUCT()
struct FClientChannelInfo
{
	GENERATED_BODY()

public:
	UPROPERTY()
		uint8 bUseReflectObject : 1;

	UPROPERTY()
		UClass* Class;

	UPROPERTY()
		FName ActorName;

	UPROPERTY()
		UClientChannel* Owner;

	UPROPERTY()
		UClientChannel* Instigator;
};

USTRUCT()
struct FClientChannelSpawnInfo
{
	GENERATED_BODY()

public:
	UPROPERTY()
		FTransform SpawnTransform;

	UPROPERTY()
		ESpawnActorCollisionHandlingMethod CollisionMethodOverride;

	UPROPERTY()
		uint8 bPossessOnSpawn : 1;
};

struct FClientChannelProperty
{
	UProperty* Property;
	UClass* Class;
	ELifetimeCondition	Condition;
	ELifetimeRepNotifyCondition RepNotifyCondition;
	ERepParentFlags Flags;
	UFunction* RepNotify;
};

struct FClientChannelPropertyData
{
	TArray<uint8> Value;
	uint32 CheckSum;
};

class FClientChannelPropertyMap
{
public:
	FClientChannelPropertyMap() {};
	FClientChannelPropertyMap(uint16 Reps)
	{
		Property.SetNum(Reps);
	};

	virtual ~FClientChannelPropertyMap() {};

	TArray<FClientChannelProperty> Property;
};

class FClientChannelPropertyTracker : public IRepChangedPropertyTracker
{
public:
	FClientChannelPropertyTracker(){};

	FClientChannelPropertyTracker(FClientChannelPropertyMap* PropSet, uint16 Reps)
	{
		PropertySet = PropSet;
		RepProperty = &PropSet->Property;
		Lifetime.SetNum(Reps);
		Data.SetNum(Reps);
	};

	virtual ~FClientChannelPropertyTracker() {};

	virtual void SetCustomIsActiveOverride(const uint16 RepIndex, const bool bIsActive) override {};

	virtual void SetExternalData(const uint8* Src, const int32 NumBits) override {};

	virtual bool IsReplay() const override { return false; };

	FClientChannelPropertyMap* PropertySet;
	TArray<FRepChangedParent> Lifetime;
	TArray<FClientChannelProperty>* RepProperty;
	TArray<FClientChannelPropertyData> Data;
};

UCLASS()
class CANET_API UClientChannel : public UActorComponent
{
	GENERATED_BODY()

#if WITH_SERVER_CODE

public:
	virtual void InitializeChannel(const FClientChannelInfo& ChannelInfo, const FClientChannelSpawnInfo& SpawnInfo);

protected:
	virtual void InitializeChannel_ServerSide();

#endif 

protected:
	virtual void InitializeChannel();
	virtual void InitializeProperty(FClientChannelProperty& ChannelProperty);

private:
	FNetworkObjectInfo View;
	FClientChannelPropertyTracker PropertyTracker;
	static TMap<UClass*, FClientChannelPropertyMap*> PropertyMap;

//* Begin Networking

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UFUNCTION()
		void OnRep_ChannelInfo();

private:
	UPROPERTY(ReplicatedUsing = "OnRep_ChannelInfo")
		FClientChannelInfo _ChannelInfo;

	UPROPERTY(Replicated)
		FClientChannelSpawnInfo _SpawnInfo;

	UPROPERTY(Replicated)
		AActor* Viewer;

//* End Networking

};
