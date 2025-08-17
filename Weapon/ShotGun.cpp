// Fill out your copyright notice in the Description page of Project Settings.


#include "ShotGun.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SkeletalMeshSocket.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "Particles/particleSystemComponent.h"
#include "Sound/SoundCue.h"
void AShotGun::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget); // 부모 클래스의 Fire 함수 호출
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("muzz"));
	if (MuzzleFlashSocket) {
		FTransform ScoketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = ScoketTransform.GetLocation();
		TMap<ATimeFractureCharacter*, uint32> HitMap; // 히트된 캐릭터와 히트 수를 저장할 맵
		for (uint32 i = 0; i < NumberOfPellets; i++) {
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit); // 트레이스 히트 결과를 얻음
			ATimeFractureCharacter* TFCharacter = Cast<ATimeFractureCharacter>(FireHit.GetActor());

			if (TFCharacter && InstigatorController && HasAuthority()) {
				if(HitMap.Contains(TFCharacter)) {
					HitMap[TFCharacter]++; // 이미 히트된 캐릭터라면 히트 수 증가
				} else {
					HitMap.Emplace(TFCharacter, 1); // 새로운 캐릭터라면 히트 수 1로 초기화
				}
			}
			if (ImpactNiagara) {
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					GetWorld(),
					ImpactNiagara,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation() // 표면 방향에 맞춰서 회전
				);
			}
			if (HitSound) {
				UGameplayStatics::PlaySoundAtLocation(
					GetWorld(),
					HitSound,
					FireHit.ImpactPoint, .5f, FMath::RandRange(-.5f, .5f) // 소리의 볼륨과 피치 랜덤화
				);
			}
		}
		for (auto HitPair : HitMap) {
			if (HitPair.Key && InstigatorController && HasAuthority()) {
				UGameplayStatics::ApplyDamage(
					HitPair.Key,
					Damage * HitPair.Value,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
		}
	}
}
