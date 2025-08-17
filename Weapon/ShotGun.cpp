// Fill out your copyright notice in the Description page of Project Settings.


#include "ShotGun.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SkeletalMeshSocket.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "Particles/particleSystemComponent.h"
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
		for (uint32 i = 0; i < NumberOfPellets; i++) {
			FVector End = TraceEndWithScatter(Start, HitTarget);
		}
	}
}
