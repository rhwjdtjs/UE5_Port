// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "ShieldPickup.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_7A_API AShieldPickup : public APickup
{
	GENERATED_BODY()
public:
	AShieldPickup();
protected:
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
private:
	UPROPERTY(EditAnywhere)
	float ShieldAmount = 30.f;

	UPROPERTY(EditAnywhere)
	float ShieldTime = 5.f;
};
