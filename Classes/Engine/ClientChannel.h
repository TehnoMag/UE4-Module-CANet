// Copyright (c) 2007-2019, Delta-Proxima Team All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ClientChannel.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogClientChannel, Log, All);

class UClientChannel;

UENUM()
enum class EClientChannelMode : uint8
{
	CCM_DEFAULT		UMETA(DisplayName = "Server Authority"),
	CCM_OWNER		UMETA(DisplayName = "Own Client Authority"),
	CCM_MAX			UMETA(DisplayName = "Symmetric authority")
};

USTRUCT()
struct FClientChannelInfo
{
	GENERATED_BODY()

public:
	UPROPERTY()
		UClass* Class;

	UPROPERTY()
		FTransform SpawnTransform;

	UPROPERTY()
		ESpawnActorCollisionHandlingMethod CollisionMethodOverride;

	UPROPERTY()
		EClientChannelMode AuthorityMode;

	UPROPERTY()
		UClientChannel* Owner;

	UPROPERTY()
		UClientChannel* Instigator;

	UPROPERTY()
		uint8 bPossesOnSpawn : 1;
};

USTRUCT()
struct FClientChannelProperty
{
	GENERATED_BODY()

public:
	UPROPERTY()
		UProperty* Property;

	UPROPERTY()
		uint32 CheckSum;

	UPROPERTY()
		TArray<uint8> Raw;
};

USTRUCT()
struct FClientChannelRepData
{
	GENERATED_BODY()

public:
	UPROPERTY()
		uint32 Index;

	UPROPERTY()
		uint32 CheckSum;

	UPROPERTY()
		TArray<uint8> Raw;
};

class FRepPropertyTrackerNull : public IRepChangedPropertyTracker
{
public:
	FRepPropertyTrackerNull() {};

	virtual void SetCustomIsActiveOverride(const uint16 RepIndex, const bool bIsActive) override {};

	virtual void SetExternalData(const uint8* Src, const int32 NumBits) override {};

	virtual bool IsReplay() const override { return false; }

	/**
	* Used when tracking memory to gather the total size of a given instance.
	* This should include the dynamically allocated data, as well as the classes size.
	*/
	virtual void CountBytes(FArchive& Ar) const {};
};

UCLASS()
class CANET_API UClientChannel : public UActorComponent
{
	GENERATED_BODY()

public:
	UClientChannel();
	
	void InitializeChannel(const FClientChannelInfo& ChannelInfo);

private:
	void InitializeChannel();
	void InitializeProperty(UProperty* Property, AActor* Container);
	void GatherUpdates(TArray<FClientChannelRepData>& RepData);
	void ReceiveUpdate(const TArray<FClientChannelRepData>& RepData);

private:
	AActor* View;
	TArray<FClientChannelProperty> Properties;
	FRepPropertyTrackerNull TrackerNull;

//~ Begin Network

private:
	UFUNCTION(Server, UnReliable, WithValidation)
		void Server_ReceiveUpdate(const TArray<FClientChannelRepData>& RepData);

	UFUNCTION()
		void Client_ReceiveUpdate();

	UFUNCTION()
		void OnRep_ChannelInfo();

private:
	UPROPERTY(ReplicatedUsing = "OnRep_ChannelInfo")
		FClientChannelInfo _ChannelInfo;
	
	UPROPERTY(Replicated)
		AActor* Viewer;

	UPROPERTY(ReplicatedUsing = "Client_ReceiveUpdate")
		TArray<FClientChannelRepData> ReplicationData;

//~ End Network

//~ Begin UActorComponent Interface

public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

//~ End UActorComponent Interface

//~ Begin UObject Interface

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

//~ End UObject Interface

//~ Begin Blueprint Interface

public:
	UFUNCTION(BlueprintCallable, Category = "Client Authority Network")
		AActor* GetViewActor() const { return View; };

//~ End Blueprint Interface

};
