// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class UNREALPROJECT_7A_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();
	virtual void Destroyed() override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
private:
	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;
	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TracerNiagara; // �߻�ü�� Ʈ���̼� ȿ���� ���� ���̾ư��� �ý��� ���ø�;
	class UNiagaraComponent* TracerNiagaraComponent; // �߻�ü�� Ʈ���̼� ȿ���� ���� ���̾ư��� ������Ʈ;
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* ImpactNiagara; // �߻�ü�� Ʈ���̼� ȿ���� ���� ���̾ư��� �ý��� ���ø�;
	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound; // �߻�ü�� �浹�� �� ����� ���� ť
	UFUNCTION(NetMulticast, Reliable)
	void MulticastImpactEffect(const FVector& Location, const FRotator& Rotation);
};
