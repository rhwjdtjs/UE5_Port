// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_7A_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	AProjectileRocket();
protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	UPROPERTY(VisibleAnywhere)
	class URocketMovementComponent* RocketMovementComponent; // ������ �������� �����ϴ� ������Ʈ
private:
	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* RocketMesh; // ���� �޽� ������Ʈ
};
