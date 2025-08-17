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
protected:
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);
private:
	UPROPERTY(EditAnywhere)
	float Damage = 20.f; // ������

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* ImpactNiagara; // �߻�ü�� Ʈ���̼� ȿ���� ���� ���̾ư��� �ý��� ���ø�;
	UPROPERTY(EditAnywhere)
	class UParticleSystem* BeamParticle; // �߻�ü�� Ʈ���̼� ȿ���� ���� ��ƼŬ �ý���;

	//Ʈ���̽� ���� ��ġ�� ��� ȿ���� �����Ͽ� ����ϴ� �Լ�

	UPROPERTY(EditAnywhere, Category="ShotGun")
	float DistanceToSphere = 800.f; // Ʈ���̽� �� ��ġ���� ��� ȿ���� ������ �Ÿ�
	UPROPERTY(EditAnywhere)
	float SphereRadius = 75.f; // Ʈ���̽� �� ��ġ���� ��� ȿ���� ������ �Ÿ�
	UPROPERTY(EditAnywhere)
	bool bUseScatter = false; // ��� ȿ�� ��� ����
};
