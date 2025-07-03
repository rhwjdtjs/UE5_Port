// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"
void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget); //부모 클래스의 Fire 함수 호출

	if (!HasAuthority()) return; //서버에서만 실행
	APawn* InstigatorPawn = Cast<APawn>(GetOwner()); //소유자를 Pawn으로 캐스팅
	const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName(FName("muzz")); //무기 메시에서 "muzz" 소켓을 가져옴
	if (MuzzleSocket) {
		FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh()); //소켓의 변환 정보를 가져옴
		FVector ToTarget =HitTarget - SocketTransform.GetLocation(); //타겟 위치에서 소켓 위치를 뺀 벡터 계산
		FRotator TargetRotation = ToTarget.Rotation(); //타겟 벡터의 회전 정보를 계산
		if (ProjectileClass && InstigatorPawn) {
			FActorSpawnParameters SpawnParams; 
			SpawnParams.Owner = GetOwner(); //소유자를 설정
			SpawnParams.Instigator = InstigatorPawn; //인스티게이터를 설정
			UWorld* World = GetWorld();
			if (World) {
				World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(),
					TargetRotation, SpawnParams); //발사체를 생성
			}
		}
	}
}
