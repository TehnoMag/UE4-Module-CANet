// Copyright (c) 2007-2019, Delta-Proxima Team All rights reserved.

#include "Engine/ClientChannel.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
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
	FClientChannelProperty ChannelProperty;
	ChannelProperty.Property = Property;

	uint32 _size = Property->GetSize();

	ChannelProperty.Raw.Empty();
	ChannelProperty.Raw.AddZeroed(_size);
	uint8* _ptr_cnl_ptr = ChannelProperty.Raw.GetData();
	uint8* _ptr_ctr_ptr = Property->ContainerPtrToValuePtr<uint8>(Container);

	Property->CopyCompleteValue(_ptr_cnl_ptr, _ptr_ctr_ptr);

	ChannelProperty.CheckSum = FCrc::MemCrc32(_ptr_cnl_ptr, _size);

	Properties.Emplace(ChannelProperty);
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
			
#if WITH_EDITOR
			if (_ChannelInfo.AuthorityMode == EClientChannelMode::CCM_OWNER && Viewer->GetOwner() != nullptr)
			{
				UE_LOG(LogClientChannel, Log, TEXT("%s: Client %i set as authority"), *GetName(), GPlayInEditorID - 1);
				SetComponentTickEnabled(true);
			}
#endif

		}
	}
}

void UClientChannel::GatherUpdates(TArray<FClientChannelRepData>& RepData)
{
	for (int32 i = 0; i < Properties.Num(); ++i)
	{
		FClientChannelProperty* ChannelProperty = &Properties[i];
		UProperty* Property = ChannelProperty->Property;
		uint32 _size = Property->GetSize();
		
		uint8* _ptr_ctr_ptr = Property->ContainerPtrToValuePtr<uint8>(View);
		uint32 _crc = FCrc::MemCrc32(_ptr_ctr_ptr, _size);

		if (ChannelProperty->CheckSum != _crc)
		{
			ChannelProperty->Raw.Empty();
			ChannelProperty->Raw.AddZeroed(_size);
			uint8* _ptr_cnl_ptr = ChannelProperty->Raw.GetData();
			Property->CopyCompleteValue(_ptr_cnl_ptr, _ptr_ctr_ptr);
			ChannelProperty->CheckSum = _crc;

			FClientChannelRepData RepInfo;
			RepInfo.Index = i;
			RepInfo.CheckSum = _crc;
			RepInfo.Raw = ChannelProperty->Raw;
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
	ReceiveUpdate(ReplicationData);
}

void UClientChannel::ReceiveUpdate(const TArray<FClientChannelRepData>& RepData)
{
	if (RepData.Num())
	{
		TArray<FClientChannelRepData> srvRepInfo;
		srvRepInfo.Empty();

		for (FClientChannelRepData RepInfo : RepData)
		{
			FClientChannelProperty* ChannelProperty = &Properties[RepInfo.Index];

			if (ChannelProperty->CheckSum != RepInfo.CheckSum)
			{
				ChannelProperty->Raw = RepInfo.Raw;
				ChannelProperty->CheckSum = RepInfo.CheckSum;
				srvRepInfo.Emplace(RepInfo);

				if (View != nullptr)
				{
					uint8* _ptr_ctr_ptr = ChannelProperty->Property->ContainerPtrToValuePtr<uint8>(View);
					uint8* _ptr_cnl_ptr = ChannelProperty->Raw.GetData();
					ChannelProperty->Property->CopyCompleteValue(_ptr_ctr_ptr, _ptr_cnl_ptr);

					//Call RepNotify
					if (ChannelProperty->Property->RepNotifyFunc != NAME_None)
					{
						UFunction* RepNotify = View->FindFunction(ChannelProperty->Property->RepNotifyFunc);

						if (RepNotify)
						{
							View->ProcessEvent(RepNotify, nullptr);
						}
					}
				}

#if WITH_EDITOR
				FString PropName = ChannelProperty->Property->GetName();

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
		}

		if (srvRepInfo.Num())
			ReplicationData = srvRepInfo;
	}
}
