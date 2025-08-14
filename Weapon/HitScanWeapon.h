// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_7A_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override; //발사 함수, 자식 클래스에서 구현할 수 있다.
private:
	UPROPERTY(EditAnywhere)
	float Damage = 20.f; // 데미지

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* ImpactNiagara; // 발사체의 트레이서 효과를 위한 나이아가라 시스템 템플릿;
};
