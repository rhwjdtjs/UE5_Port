// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"
void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget); //�θ� Ŭ������ Fire �Լ� ȣ��

	if (!HasAuthority()) return; //���������� ����
	APawn* InstigatorPawn = Cast<APawn>(GetOwner()); //�����ڸ� Pawn���� ĳ����
	const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName(FName("muzz")); //���� �޽ÿ��� "muzz" ������ ������
	if (MuzzleSocket) {
		FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh()); //������ ��ȯ ������ ������
		FVector ToTarget =HitTarget - SocketTransform.GetLocation(); //Ÿ�� ��ġ���� ���� ��ġ�� �� ���� ���
		FRotator TargetRotation = ToTarget.Rotation(); //Ÿ�� ������ ȸ�� ������ ���
		if (ProjectileClass && InstigatorPawn) {
			FActorSpawnParameters SpawnParams; 
			SpawnParams.Owner = GetOwner(); //�����ڸ� ����
			SpawnParams.Instigator = InstigatorPawn; //�ν�Ƽ�����͸� ����
			UWorld* World = GetWorld();
			if (World) {
				World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(),
					TargetRotation, SpawnParams); //�߻�ü�� ����
			}
		}
	}
}
