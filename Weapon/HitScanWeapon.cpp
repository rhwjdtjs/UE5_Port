// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "Particles/particleSystemComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"
void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("muzz"));
	if (MuzzleFlashSocket) {
		FTransform ScoketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = ScoketTransform.GetLocation();
		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit); // 트레이스 히트 결과를 얻음
		ATimeFractureCharacter* TFCharacter = Cast<ATimeFractureCharacter>(FireHit.GetActor());
		if (TFCharacter && InstigatorController && HasAuthority()) {
			UGameplayStatics::ApplyDamage(
				TFCharacter,
				Damage,
				InstigatorController,
				this,
				UDamageType::StaticClass()
			);
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
				FireHit.ImpactPoint
			);
		}
 	}
}

FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector ToTargetNormalize = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalize * DistanceToSphere; // 트레이스 시작 위치에서 타겟 방향으로 거리를 더한 위치
	FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius); // 랜덤한 방향 벡터 생성
	FVector EndLoc = SphereCenter + RandVec; // 랜덤 벡터를 더하여 트레이스 끝 위치 계산
	FVector ToEndLoc = EndLoc - TraceStart; // 트레이스 시작 위치에서 끝 위치까지의 벡터
	return FVector(TraceStart + ToEndLoc * 80000.f / ToEndLoc.Size()); // 트레이스 끝 위치를 반환
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World) {
		FVector End = bUseScatter ? TraceEndWithScatter(TraceStart,HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f; // 타겟 방향으로 트레이스 끝 위치 설정
		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECC_Visibility
		);
		FVector BeamEnd = End;
		if (OutHit.bBlockingHit) {
			BeamEnd = OutHit.ImpactPoint; // 충돌 지점으로 트레이서 끝 위치 설정
			if (BeamParticle) {
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					World,
					BeamParticle,
					TraceStart, FRotator::ZeroRotator,true // 트레이스 시작 위치에서 파티클 생성
				);
				if (Beam) {
					Beam->SetVectorParameter(FName("Target"), BeamEnd); // 트레이서 끝 위치 설정
				}
			}
		}
	}
}
