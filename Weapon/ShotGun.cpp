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
	AWeapon::Fire(HitTarget); // �θ� Ŭ������ Fire �Լ� ȣ��
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("muzz"));
	if (MuzzleFlashSocket) {
		FTransform ScoketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = ScoketTransform.GetLocation();
		TMap<ATimeFractureCharacter*, uint32> HitMap; // ��Ʈ�� ĳ���Ϳ� ��Ʈ ���� ������ ��
		for (uint32 i = 0; i < NumberOfPellets; i++) {
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit); // Ʈ���̽� ��Ʈ ����� ����
			ATimeFractureCharacter* TFCharacter = Cast<ATimeFractureCharacter>(FireHit.GetActor());

			if (TFCharacter && InstigatorController && HasAuthority()) {
				if(HitMap.Contains(TFCharacter)) {
					HitMap[TFCharacter]++; // �̹� ��Ʈ�� ĳ���Ͷ�� ��Ʈ �� ����
				} else {
					HitMap.Emplace(TFCharacter, 1); // ���ο� ĳ���Ͷ�� ��Ʈ �� 1�� �ʱ�ȭ
				}
			}
			if (ImpactNiagara) {
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					GetWorld(),
					ImpactNiagara,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation() // ǥ�� ���⿡ ���缭 ȸ��
				);
			}
			if (HitSound) {
				UGameplayStatics::PlaySoundAtLocation(
					GetWorld(),
					HitSound,
					FireHit.ImpactPoint, .5f, FMath::RandRange(-.5f, .5f) // �Ҹ��� ������ ��ġ ����ȭ
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
