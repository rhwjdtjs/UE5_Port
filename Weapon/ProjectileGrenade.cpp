// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGrenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Sound/soundcue.h"
#include "Kismet/GameplayStatics.h"
AProjectileGrenade::AProjectileGrenade()
{
	ProjectileMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GrenadeMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 로켓 메쉬는 충돌을 하지 않도록 설정

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true; // 발사체가 이동 방향을 따라 회전하도록 설정
	ProjectileMovementComponent->SetIsReplicated(true); // 발사체 이동 컴포넌트가 네트워크에서 복제되도록 설정
	ProjectileMovementComponent->bShouldBounce = true; // 발사체가 충돌 시 튕기도록 설정
}

void AProjectileGrenade::Destroyed()
{
	ExplodeDamage(); // 발사체가 파괴될 때 폭발 피해 적용
	Super::Destroyed();
}

void AProjectileGrenade::BeginPlay()
{
	AActor::BeginPlay();

	StartDestroyTimer();
	SpawnTrailSystem(); // 발사체의 트레일 시스템을 생성

	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce); // 발사체가 튕길 때 호출될 함수 바인딩
}

void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if (BounceSound) {
		UGameplayStatics::PlaySoundAtLocation(this, BounceSound, GetActorLocation()); // 튕길 때 사운드 재생
	}
}
