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
// ============================================================
// [발사 처리] Fire()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 다중 탄환(펠릿)을 동시에 발사하는 산탄총 전용 발사 로직.
//   - 각 펠릿이 개별 트레이스되어 피격 판정, 이펙트, 사운드 처리.
//   - 서버에서는 동일 캐릭터에 다수 피격 시 누적 데미지 적용.
//
// 알고리즘 설명 : 
//   1. 부모 클래스 AWeapon::Fire() 호출하여 공통 발사 처리.
//   2. 총구 소켓 위치("muzz")에서 여러 방향으로 라인트레이스 반복.
//   3. 각 탄환 트레이스 결과(FHitResult)에 대해:
//      - Niagara Impact 이펙트 생성
//      - Hit 사운드 재생 (볼륨/피치 랜덤)
//      - 피격된 캐릭터를 TMap<ATimeFractureCharacter*, uint32> 에 누적 저장
//   4. 모든 트레이스 후 HitMap을 순회하며
//      - 같은 캐릭터가 여러 번 맞았다면 히트 수 * 데미지만큼 누적 피해 적용.
//
// 사용 예시 : 
//   - Damage = 10, NumberOfPellets = 8 → 캐릭터가 3펠릿 맞으면 30 데미지
//   - 서버 권한(HasAuthority())일 때만 실제 피해 계산 수행
//
// 참고 : 
//   - ImpactNiagara : 피격 시 시각 효과
//   - HitSound : 표면 충돌 시 사운드
//   - WeaponTraceHit() : 총알 궤적 계산 유틸리티 함수
// ============================================================
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
