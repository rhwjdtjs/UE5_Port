// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "RocketMovementComponent.h"
AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RocketMesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 로켓 메쉬는 충돌을 하지 않도록 설정

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true; // 로켓이 이동 방향을 따라 회전하도록 설정
	RocketMovementComponent->SetIsReplicated(true); // 로켓 이동 컴포넌트가 네트워크에서 복제되도록 설정
}
void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {

	APawn* FiringPawn=GetInstigator();
	if (FiringPawn) {
		AController* FiringController = FiringPawn->GetController();
		if (FiringController) {
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, Damage, 10.f, GetActorLocation(), 200.f, 500.f, 1.f, UDamageType::StaticClass(), TArray<AActor*>(), this,FiringController); // 적	에게 피해를 적용
		}
	}

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
