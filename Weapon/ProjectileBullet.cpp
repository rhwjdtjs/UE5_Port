// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true; // �߻�ü�� �̵� ������ ���� ȸ���ϵ��� ����
	ProjectileMovementComponent->SetIsReplicated(true); // �߻�ü �̵� ������Ʈ�� ��Ʈ��ũ���� �����ǵ��� ����
}
void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()); // �߻�ü�� �����ڰ� ACharacter���� Ȯ��
	if (OwnerCharacter) { // �����ڰ� ACharacter�� ��쿡�� ���ظ� ����
		AController* OwnerController = OwnerCharacter->Controller; // �������� ��Ʈ�ѷ��� ������
		if (OwnerController) { // ��Ʈ�ѷ��� ��ȿ�� ��쿡�� ���ظ� ����
			UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass()); // ���ظ� ����
			}
		}
	
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
