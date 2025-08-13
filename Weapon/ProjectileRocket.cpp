// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "RocketMovementComponent.h"
AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RocketMesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // ���� �޽��� �浹�� ���� �ʵ��� ����

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true; // ������ �̵� ������ ���� ȸ���ϵ��� ����
	RocketMovementComponent->SetIsReplicated(true); // ���� �̵� ������Ʈ�� ��Ʈ��ũ���� �����ǵ��� ����
}
void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {

	APawn* FiringPawn=GetInstigator();
	if (FiringPawn) {
		AController* FiringController = FiringPawn->GetController();
		if (FiringController) {
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, Damage, 10.f, GetActorLocation(), 200.f, 500.f, 1.f, UDamageType::StaticClass(), TArray<AActor*>(), this,FiringController); // ��	���� ���ظ� ����
		}
	}

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
