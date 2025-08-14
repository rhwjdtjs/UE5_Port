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
	virtual void Fire(const FVector& HitTarget) override; //�߻� �Լ�, �ڽ� Ŭ�������� ������ �� �ִ�.
private:
	UPROPERTY(EditAnywhere)
	float Damage = 20.f; // ������

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* ImpactNiagara; // �߻�ü�� Ʈ���̼� ȿ���� ���� ���̾ư��� �ý��� ���ø�;
};
