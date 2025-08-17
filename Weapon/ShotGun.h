// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "ShotGun.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_7A_API AShotGun : public AHitScanWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;
private:
	UPROPERTY(EditAnyWhere)
	uint32 NumberOfPellets = 10; // ¼¦°ÇÀÇ Æç¸´ ¼ö
};
