// Copyright (c) 2007-2019, Delta-Proxima Team All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CANGameModeBase.generated.h"

class UClientChannel;

UCLASS()
class CANET_API ACANGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACANGameModeBase();

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "ClientAuthorityNetwork")
		void SpawnDefaultPawnAtTransformWithClientChannel(AController* NewPlayer, const FTransform& SpawnTransform);

private:
	//Enable using ClientChannel for player pawn
	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
		uint8 bUseClientChannelForDefaultPawnClass : 1;

	//ClientChannel class
	UPROPERTY(EditDefaultsOnly, Category = "Classes")
		TSubclassOf<UClientChannel> ClientChannelClass;

//* Begin AGameModeBase Interface

public:
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;

//*

};
