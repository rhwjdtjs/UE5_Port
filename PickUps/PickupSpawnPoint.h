// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

UCLASS()
class UNREALPROJECT_7A_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APickupSpawnPoint();
	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere, Category = "Pickup")
	TArray<TSubclassOf<class APickup >> PickupClasses;
	UPROPERTY()
	APickup* SpawnedPickup;
	void SpawnPickup();
	void SpawnPickupTimerFinished();
	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);
private:
	FTimerHandle SpawnPickupTimer;
	UPROPERTY(EditAnywhere, Category = "Pickup")
	float SpawnPickupTimeMin;
	UPROPERTY(EditAnywhere, Category = "Pickup")
	float SpawnPickupTimeMax;
public:	


};
