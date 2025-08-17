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
		FVector End = Start+ (HitTarget-Start) * 1.25f;

		FHitResult FireHit;
		UWorld* World = GetWorld();
		if (World) {
			World->LineTraceSingleByChannel(
				FireHit,
				Start,
				End,
				ECC_Visibility
			);
			FVector BeamEnd = End;
			if (FireHit.bBlockingHit) {
				BeamEnd = FireHit.ImpactPoint; // �浹 �������� Ʈ���̼� �� ��ġ ����
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
						World,
						ImpactNiagara,
						FireHit.ImpactPoint,
						FireHit.ImpactNormal.Rotation() // ǥ�� ���⿡ ���缭 ȸ��
					);
				}
			}
			if (BeamParticle) {
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					World,
					BeamParticle,
					ScoketTransform
				);
				if (Beam) {
					Beam->SetVectorParameter(FName("Target"), BeamEnd); // Ʈ���̼� �� ��ġ ����
				}
			}
		}
 	}
}

FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector ToTargetNormalize = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalize * DistanceToSphere; // Ʈ���̽� ���� ��ġ���� Ÿ�� �������� �Ÿ��� ���� ��ġ
	FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius); // ������ ���� ���� ����
	FVector EndLoc = SphereCenter + RandVec; // ���� ���͸� ���Ͽ� Ʈ���̽� �� ��ġ ���
	FVector ToEndLoc = EndLoc - TraceStart; // Ʈ���̽� ���� ��ġ���� �� ��ġ������ ����
	return FVector(TraceStart + ToEndLoc * 80000.f / ToEndLoc.Size()); // Ʈ���̽� �� ��ġ�� ��ȯ
}