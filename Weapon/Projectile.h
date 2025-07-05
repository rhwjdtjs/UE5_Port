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
	class UNiagaraSystem* TracerNiagara; // 발사체의 트레이서 효과를 위한 나이아가라 시스템 템플릿;
	class UNiagaraComponent* TracerNiagaraComponent; // 발사체의 트레이서 효과를 위한 나이아가라 컴포넌트;
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* ImpactNiagara; // 발사체의 트레이서 효과를 위한 나이아가라 시스템 템플릿;
	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound; // 발사체가 충돌할 때 재생할 사운드 큐
	UFUNCTION(NetMulticast, Reliable)
	void MulticastImpactEffect(const FVector& Location, const FRotator& Rotation);
};
