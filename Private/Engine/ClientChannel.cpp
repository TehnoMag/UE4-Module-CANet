// Copyright (c) 2007-2019, Delta-Proxima Team All rights reserved.

#include "Engine/ClientChannel.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "UnrealNetwork.h"
#include "EngineGlobals.h"

DEFINE_LOG_CATEGORY(LogClientChannel);

void UClientChannel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UClientChannel, _ChannelInfo);
	DOREPLIFETIME(UClientChannel, Viewer);
	DOREPLIFETIME(UClientChannel, ReplicationData);
}

void UClientChannel::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TArray<FClientChannelRepData> RepData;
	RepData.Empty();
	GatherUpdates(RepData);

	if (RepData.Num())
	{
		switch (GetWorld()->GetNetMode())
		{
			case NM_Client:
			{
				Server_ReceiveUpdate(RepData);
				break;
			}
			case NM_DedicatedServer:
			{
				ReplicationData = RepData;
				break;
			}
		}
	}

}

UClientChannel::UClientChannel()
{
	bReplicates = true;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UClientChannel::InitializeChannel(const FClientChannelInfo& ChannelInfo)
{
	_ChannelInfo = ChannelInfo;
	Viewer = GetOwner();
	InitializeChannel();
}

void UClientChannel::InitializeChannel()
{
	UClass* Class = _ChannelInfo.Class;
	AActor* Template = View ? View : Class->GetDefaultObject<AActor>();

	Class->SetUpRuntimeReplicationData();

	PropertyTracker = FClientChannelPropertyTracker(Class->ClassReps.Num());
	Template->GetLifetimeReplicatedProps(PropertyTracker.LifetimeProperty);

	for (FRepRecord RepRecord : Class->ClassReps)
	{
		UProperty* Property = RepRecord.Property;
		InitializeProperty(Property, Template);
	}

#if WITH_EDITOR
	if (GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogClientChannel, Log, TEXT("%s initialized on client %i."), *GetName(), GPlayInEditorID - 1);
	}
	else
	{
		UE_LOG(LogClientChannel, Log, TEXT("%s initialized on server"), *GetName());
	}
#endif
}

void UClientChannel::InitializeProperty(UProperty* Property, AActor* Container)
{
	FClientChannelProperty& ChannelProperty = PropertyTracker.RepProperty[Property->RepIndex];
	ChannelProperty.Property = Property;
	//ChannelProperty.LifetimeProperty = &PropertyTracker.LifetimeProperty[Property->RepIndex];

	uint32 _size = Property->GetSize();

	ChannelProperty.Raw.AddZeroed(_size);
	uint8* _ptr_cnl_ptr = ChannelProperty.Raw.GetData();
	uint8* _ptr_ctr_ptr = Property->ContainerPtrToValuePtr<uint8>(Container);

	Property->CopyCompleteValue(_ptr_cnl_ptr, _ptr_ctr_ptr);

	ChannelProperty.CheckSum = FCrc::MemCrc32(_ptr_cnl_ptr, _size);
}

void UClientChannel::OnRep_ChannelInfo()
{
	if (View == nullptr && Viewer == GetOwner() && Viewer->IsA<APlayerState>())
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.ObjectFlags |= RF_Transient;
		SpawnInfo.SpawnCollisionHandlingOverride = _ChannelInfo.CollisionMethodOverride;
		SpawnInfo.Instigator = _ChannelInfo.Instigator ? Cast<APawn>(_ChannelInfo.Instigator->View) : nullptr;
		SpawnInfo.Owner = _ChannelInfo.Owner ? _ChannelInfo.Owner->View : nullptr;

		
		View = GetWorld()->SpawnActor<AActor>(_ChannelInfo.Class, _ChannelInfo.SpawnTransform, SpawnInfo);

		if (View != nullptr)
		{
			InitializeChannel();

			if (_ChannelInfo.AuthorityMode == EClientChannelMode::CCM_OWNER && Viewer->GetOwner() != nullptr)
			{
#if WITH_EDITOR
				UE_LOG(LogClientChannel, Log, TEXT("%s: Client %i set as authority"), *GetName(), GPlayInEditorID - 1);
#endif
				SetComponentTickEnabled(true);

				if (_ChannelInfo.bPossesOnSpawn)
				{
					GetWorld()->GetFirstPlayerController()->Possess(Cast<APawn>(View));

					if (View->IsA<ACharacter>())
					{
						Cast<ACharacter>(View)->GetMesh()->bOnlyAllowAutonomousTickPose = false;
					}
				}
			}
			else
			{
				View->SwapRoles();
			}
		}
	}
}

void UClientChannel::GatherUpdates(TArray<FClientChannelRepData>& RepData)
{
	View->PreReplication(PropertyTracker);

	for (FClientChannelProperty& RepProperty : PropertyTracker.RepProperty)
	{
		UProperty* Property = RepProperty.Property;
		uint32 _size = Property->GetSize();

		uint8* _ptr_ctr_ptr = Property->ContainerPtrToValuePtr<uint8>(View);
		uint32 _crc = FCrc::MemCrc32(_ptr_ctr_ptr, _size);

		if (RepProperty.CheckSum != _crc)
		{
			RepProperty.Raw.Empty();
			RepProperty.Raw.AddZeroed(_size);
			uint8* _ptr_rpr_ptr = RepProperty.Raw.GetData();
			Property->CopyCompleteValue(_ptr_rpr_ptr, _ptr_ctr_ptr);
			RepProperty.CheckSum = _crc;

			FClientChannelRepData RepInfo;
			RepInfo.RepIndex = Property->RepIndex;
			RepInfo.CheckSum = _crc;
			RepInfo.Raw = RepProperty.Raw;
			RepData.Emplace(RepInfo);
		}
	}
}

bool UClientChannel::Server_ReceiveUpdate_Validate(const TArray<FClientChannelRepData>& RepData)
{
	return true;
}

void UClientChannel::Server_ReceiveUpdate_Implementation(const TArray<FClientChannelRepData>& RepData)
{
	ReceiveUpdate(RepData);
}

void UClientChannel::Client_ReceiveUpdate()
{
	if (View != nullptr)
	{
		View->PreNetReceive();
		ReceiveUpdate(ReplicationData);
		View->PostNetReceive();
	}
}

void UClientChannel::ReceiveUpdate(const TArray<FClientChannelRepData>& RepData)
{
	if (RepData.Num())
	{
		TArray<FClientChannelRepData> srvRepInfo;
		srvRepInfo.Empty();

		for (FClientChannelRepData RepInfo : RepData)
		{
			FClientChannelProperty& RepProperty = PropertyTracker.RepProperty[RepInfo.RepIndex];

			if (RepProperty.CheckSum != RepInfo.CheckSum)
			{
				RepProperty.Raw = RepInfo.Raw;
				RepProperty.CheckSum = RepInfo.CheckSum;
				srvRepInfo.Emplace(RepInfo);

				if (View != nullptr)
				{
					uint8* _ptr_ctr_ptr = RepProperty.Property->ContainerPtrToValuePtr<uint8>(View);
					uint8* _ptr_rpr_ptr = RepProperty.Raw.GetData();
					RepProperty.Property->CopyCompleteValue(_ptr_ctr_ptr, _ptr_rpr_ptr);

					//Call RepNotify
					if (RepProperty.Property->RepNotifyFunc != NAME_None)
					{
						UFunction* RepNotify = View->FindFunction(RepProperty.Property->RepNotifyFunc);

						if (RepNotify)
						{
							FString NotifyName = RepNotify->GetName();
							UE_LOG(LogClientChannel, Log, TEXT("Client %i %s: notify %s"), GPlayInEditorID - 1, *GetName(), *NotifyName);
							View->ProcessEvent(RepNotify, nullptr);
						}
					}
				}
			}

#if WITH_EDITOR
			FString PropName = RepProperty.Property->GetName();

			if (GetWorld()->GetNetMode() == NM_Client)
			{
				UE_LOG(LogClientChannel, Log, TEXT("Client %i %s: receive property %s"), GPlayInEditorID - 1, *GetName(), *PropName);
			}
			else
			{
				UE_LOG(LogClientChannel, Log, TEXT("Server %s: receive property %s"), *GetName(), *PropName);
			}
#endif
		}

		if (srvRepInfo.Num())
			ReplicationData = srvRepInfo;
	}
}
