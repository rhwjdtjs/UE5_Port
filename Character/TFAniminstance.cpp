// Fill out your copyright notice in the Description page of Project Settings.


#include "TFAniminstance.h"
#include "TimeFractureCharacter.h" //캐릭터의 헤더파일을 포함시킨다
#include "GameFramework/CharacterMovementComponent.h" //캐릭터의 이동 컴포넌트를 포함시킨다
#include "Kismet/KismetMathLibrary.h" //수학 라이브러리를 포함시킨다
#include "UnrealProject_7A/TFComponents/CombatStates.h" //전투 상태를 포함시킨다
#include "UnrealProject_7A/Weapon/Weapon.h" //무기의 헤더파일을 포함시킨다
#include "UnrealProject_7A/TFComponents/WireComponent.h" //와이어 컴포넌트를 포함시킨다
void UTFAniminstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation(); //부모클래스의 초기화함수 호출
	if (!TFCharacter)
	{
		TFCharacter = Cast<ATimeFractureCharacter>(TryGetPawnOwner());
	}
}

void UTFAniminstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	// 캐릭터 참조 확인
	if (TFCharacter == nullptr)
	{
		TFCharacter = Cast<ATimeFractureCharacter>(TryGetPawnOwner());
	}
	if (TFCharacter == nullptr) return;


	// 와이어 연결 상태 갱신
	if (TFCharacter && TFCharacter->WireComponent)
	{
		bIsWireAttached = TFCharacter->WireComponent->IsAttached();
	}


	// 이동 관련 정보 계산
	FVector Velocity = TFCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = TFCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = TFCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;
	bIsCrouching = TFCharacter->bIsCrouched;


	// 전투 / 조준 상태 갱신
	bWeaponEquipped = TFCharacter->IsWeaponEquipped();
	bIsAiming = TFCharacter->IsAiming();
	bEliminated = TFCharacter->IsElimmed();


	// 조준 회전값 계산
	FRotator AimRotation = TFCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(TFCharacter->GetVelocity());

	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 15.f);
	YawOffset = DeltaRotation.Yaw;

	AO_Yaw = TFCharacter->GETAO_YAW();
	AO_Pitch = TFCharacter->GETAO_PITCH();


	// 무기 및 IK 보정
	EquippedWeapon = TFCharacter->GetEquippedWeapon();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && TFCharacter->GetMesh())
	{
		// 왼손 위치 보정 (IK)
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);

		FVector OutPos;
		FRotator OutRot;
		TFCharacter->GetMesh()->TransformToBoneSpace(FName("RightHand"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPos, OutRot);
		LeftHandTransform.SetLocation(OutPos);
		LeftHandTransform.SetRotation(FQuat(OutRot));

		// 오른손 회전 보정 (로컬 플레이어 전용)
		if (TFCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;

			FTransform MuzzleTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("muzz"), ERelativeTransformSpace::RTS_World);
			FVector MuzzleLocation = MuzzleTransform.GetLocation();
			FVector AimTarget = TFCharacter->GetHitTarget();

			FVector LookAtDirection = (AimTarget - MuzzleLocation).GetSafeNormal();
			FRotator BaseRotation = UKismetMathLibrary::MakeRotFromYZ(-LookAtDirection, FVector::UpVector);

			FQuat CorrectionQuat(FVector::YAxisVector, FMath::DegreesToRadians(-90.f));
			FQuat TargetWorldQuat = BaseRotation.Quaternion() * CorrectionQuat;

			const FTransform& ComponentToWorld = TFCharacter->GetMesh()->GetComponentToWorld();
			FQuat TargetComponentQuat = ComponentToWorld.InverseTransformRotation(TargetWorldQuat);

			RightHandRotation = FMath::RInterpTo(RightHandRotation, TargetComponentQuat.Rotator(), DeltaTime, 3000.f);
		}
	}


	// FABRIK 사용 여부 결정
	ECombatState CurrentCombatState = TFCharacter->GetCombatState();
	bUseFABRIK = (CurrentCombatState == ECombatState::ECS_Unoccupied) && bWeaponEquipped && !bEliminated;
}
