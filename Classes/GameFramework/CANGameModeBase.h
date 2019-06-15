// Copyright (c) 2007-2019, Delta-Proxima Team All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CANGameModeBase.generated.h"

UCLASS()
class CANET_API ACANGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACANGameModeBase();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Classes")
		uint8 bUseClientChannelForPlayerPawn:1;

	UPROPERTY(EditDefaultsOnly, Category = "Classes", Meta = (EditCondition = "bUseClientChannelForPlayerPawn"))
		uint8 bCreateReflectionObjects:1;

//* Begin AGameModeBase Interface

public:
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;

//* End AGameModeBase Interface

//* Begin Blueprint Interface
	
public:
	UFUNCTION(BlueprintNativeEvent, Category = "Client Authority Network")
		void SpawnDefaultPawnAtTransformWithClientChannel(AController* NewPlayer, const FTransform& SpawnTransform);

//* End Blueperint Interface
};
