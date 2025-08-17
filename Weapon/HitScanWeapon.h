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
protected:
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);
private:
	UPROPERTY(EditAnywhere)
	float Damage = 20.f; // 데미지

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* ImpactNiagara; // 발사체의 트레이서 효과를 위한 나이아가라 시스템 템플릿;
	UPROPERTY(EditAnywhere)
	class UParticleSystem* BeamParticle; // 발사체의 트레이서 효과를 위한 파티클 시스템;

	//트레이스 엔드 위치를 산란 효과를 적용하여 계산하는 함수

	UPROPERTY(EditAnywhere, Category="ShotGun")
	float DistanceToSphere = 800.f; // 트레이스 끝 위치에서 산란 효과를 적용할 거리
	UPROPERTY(EditAnywhere)
	float SphereRadius = 75.f; // 트레이스 끝 위치에서 산란 효과를 적용할 거리
	UPROPERTY(EditAnywhere)
	bool bUseScatter = false; // 산란 효과 사용 여부
};
