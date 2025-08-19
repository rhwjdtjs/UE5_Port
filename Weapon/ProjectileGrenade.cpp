// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGrenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Sound/soundcue.h"
#include "Kismet/GameplayStatics.h"
AProjectileGrenade::AProjectileGrenade()
{
	ProjectileMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GrenadeMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // ���� �޽��� �浹�� ���� �ʵ��� ����

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true; // �߻�ü�� �̵� ������ ���� ȸ���ϵ��� ����
	ProjectileMovementComponent->SetIsReplicated(true); // �߻�ü �̵� ������Ʈ�� ��Ʈ��ũ���� �����ǵ��� ����
	ProjectileMovementComponent->bShouldBounce = true; // �߻�ü�� �浹 �� ƨ�⵵�� ����
}

void AProjectileGrenade::Destroyed()
{
	ExplodeDamage(); // �߻�ü�� �ı��� �� ���� ���� ����
	Super::Destroyed();
}

void AProjectileGrenade::BeginPlay()
{
	AActor::BeginPlay();

	StartDestroyTimer();
	SpawnTrailSystem(); // �߻�ü�� Ʈ���� �ý����� ����

	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce); // �߻�ü�� ƨ�� �� ȣ��� �Լ� ���ε�
}

void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if (BounceSound) {
		UGameplayStatics::PlaySoundAtLocation(this, BounceSound, GetActorLocation()); // ƨ�� �� ���� ���
	}
}
