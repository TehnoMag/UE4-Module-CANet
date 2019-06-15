// Copyright (c) 2007-2019, Delta-Proxima Team All rights reserved.

#include "Engine/ClientChannel.h"
#include "CANet.Definations.h"
#include "UnrealNetwork.h"

void UClientChannel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UClientChannel::OnRep_ChannelInfo()
{
	
}

#if WITH_SERVER_CODE

void UClientChannel::InitializeChannel(const FClientChannelInfo& ChannelInfo, const FClientChannelSpawnInfo& SpawnInfo)
{
	_ChannelInfo = ChannelInfo;
	_SpawnInfo = SpawnInfo;
	Viewer = _ChannelInfo.Owner ? _ChannelInfo.Owner->Viewer : GetOwner();
	InitializeChannel_ServerSide();
}

void UClientChannel::InitializeChannel_ServerSide()
{
	UClass* Class = _ChannelInfo.Class;
	AActor* Template = nullptr;

	if (_ChannelInfo.bUseReflectObject)
	{
		Template = NewObject<AActor>(Viewer, Class, _ChannelInfo.ActorName, RF_Transient);
	}
	else
	{
		Template = Class->GetDefaultObject<AActor>();
	}

	View = FNetworkObjectInfo(Template);

	InitializeChannel();
}

#endif

void UClientChannel::InitializeChannel()
{
	UClass* Class = _ChannelInfo.Class;

	AActor* Template = View.Actor;

	Class->SetUpRuntimeReplicationData();
	PropertyTracker = FClientChannelPropertyTracker(Class->ClassReps.Num());

	TArray<FLifetimeProperty> LifetimeProps;
	Template->GetLifetimeReplicatedProps(LifetimeProps);

	for (FRepRecord& RepRecord : Class->ClassReps)
	{
		UProperty* Property = RepRecord.Property;

		PropertyTracker.RepProperty[Property->RepIndex].Property = Property;
	}

	for (FLifetimeProperty& Lifetime : LifetimeProps)
	{
		FClientChannelProperty& ChannelProperty = PropertyTracker.RepProperty[Lifetime.RepIndex];
		ChannelProperty.Condition = Lifetime.Condition;
		ChannelProperty.RepNotifyCondition = Lifetime.RepNotifyCondition;

		if ((ChannelProperty.Flags & ERepParentFlags::IsCustomDelta) != ERepParentFlags::None)
		{
			continue;
		}

		ChannelProperty.Flags |= ERepParentFlags::IsLifetime;

		if (Lifetime.Condition == COND_None)
		{
			ChannelProperty.Flags &= ~ERepParentFlags::IsConditional;
		}
	}

	for (FClientChannelProperty& ChannelProperty : PropertyTracker.RepProperty)
	{
		InitializeProperty(ChannelProperty);
	}
}

void UClientChannel::InitializeProperty(FClientChannelProperty& ChannelProperty)
{
	UProperty* Property = ChannelProperty.Property;
	PropertyTracker.Lifetime[Property->RepIndex].IsConditional = ((ChannelProperty.Flags & ERepParentFlags::IsConditional) != ERepParentFlags::None) ? 1 : 0;
	AActor* Container = View.Actor;

	ChannelProperty.RepNotify = Container->FindFunction(Property->RepNotifyFunc);

	uint8* _ptr_ctr_ptr = Property->ContainerPtrToValuePtr<uint8>(Container);
	uint32 _size = Property->GetSize();
	ChannelProperty.Value.AddZeroed(_size);
	uint8* _ptr_val_ptr = ChannelProperty.Value.GetData();
	Property->CopyCompleteValue(_ptr_val_ptr, _ptr_ctr_ptr);
	ChannelProperty.CheckSum = FCrc::MemCrc32(_ptr_val_ptr, _size);
}
