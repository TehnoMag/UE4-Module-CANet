// Copyright (c) 2007-2019, Delta-Proxima Team All rights reserved.

#include "Engine/ClientChannel.h"
#include "CANet.Definations.h"
#include "UnrealNetwork.h"

TMap<UClass*, FClientChannelPropertyMap*> UClientChannel::PropertyMap;

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

	FClientChannelPropertyMap* PropertySet = PropertyMap.Num() ? *PropertyMap.Find(Class) : nullptr;

	if (PropertySet == nullptr)
	{
		Class->SetUpRuntimeReplicationData();

		PropertySet = PropertyMap.Emplace(Class, new FClientChannelPropertyMap(Class->ClassReps.Num()));
		
		PropertyTracker = FClientChannelPropertyTracker(PropertySet, Class->ClassReps.Num());
		TArray<FClientChannelProperty>& RepProperty = *PropertyTracker.RepProperty;

		TArray<FLifetimeProperty> LifetimeProps;
		Template->GetLifetimeReplicatedProps(LifetimeProps);

		for (FRepRecord& RepRecord : Class->ClassReps)
		{
			UProperty* Property = RepRecord.Property;

			RepProperty[Property->RepIndex].Property = Property;
			RepProperty[Property->RepIndex].RepNotify = Class->FindFunctionByName(Property->RepNotifyFunc);
		}

		for (FLifetimeProperty& Lifetime : LifetimeProps)
		{
			FClientChannelProperty& ChannelProperty = RepProperty[Lifetime.RepIndex];
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
	}
	else
	{
		PropertyTracker = FClientChannelPropertyTracker(PropertySet, Class->ClassReps.Num());
	}

	for (FClientChannelProperty& ChannelProperty : PropertySet->Property)
	{
		InitializeProperty(ChannelProperty);
	}
}

void UClientChannel::InitializeProperty(FClientChannelProperty& ChannelProperty)
{
	UProperty* Property = ChannelProperty.Property;
	PropertyTracker.Lifetime[Property->RepIndex].IsConditional = ((ChannelProperty.Flags & ERepParentFlags::IsConditional) != ERepParentFlags::None) ? 1 : 0;
	AActor* Container = View.Actor;

	FClientChannelPropertyData& PropertyData = PropertyTracker.Data[Property->RepIndex];

	uint8* _ptr_ctr_ptr = Property->ContainerPtrToValuePtr<uint8>(Container);
	uint32 _size = Property->GetSize();
	PropertyData.Value.AddZeroed(_size);
	uint8* _ptr_val_ptr = PropertyData.Value.GetData();
	Property->CopyCompleteValue(_ptr_val_ptr, _ptr_ctr_ptr);
	PropertyData.CheckSum = FCrc::MemCrc32(_ptr_val_ptr, _size);
}
