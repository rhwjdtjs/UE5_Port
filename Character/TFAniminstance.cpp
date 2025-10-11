// Fill out your copyright notice in the Description page of Project Settings.


#include "TFAniminstance.h"
#include "TimeFractureCharacter.h" //ĳ������ ��������� ���Խ�Ų��
#include "GameFramework/CharacterMovementComponent.h" //ĳ������ �̵� ������Ʈ�� ���Խ�Ų��
#include "Kismet/KismetMathLibrary.h" //���� ���̺귯���� ���Խ�Ų��
#include "UnrealProject_7A/TFComponents/CombatStates.h" //���� ���¸� ���Խ�Ų��
#include "UnrealProject_7A/Weapon/Weapon.h" //������ ��������� ���Խ�Ų��
#include "UnrealProject_7A/TFComponents/WireComponent.h" //���̾� ������Ʈ�� ���Խ�Ų��
void UTFAniminstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation(); //�θ�Ŭ������ �ʱ�ȭ�Լ� ȣ��
	if (!TFCharacter)
	{
		TFCharacter = Cast<ATimeFractureCharacter>(TryGetPawnOwner());
	}
}

void UTFAniminstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	// ĳ���� ���� Ȯ��
	if (TFCharacter == nullptr)
	{
		TFCharacter = Cast<ATimeFractureCharacter>(TryGetPawnOwner());
	}
	if (TFCharacter == nullptr) return;


	// ���̾� ���� ���� ����
	if (TFCharacter && TFCharacter->WireComponent)
	{
		bIsWireAttached = TFCharacter->WireComponent->IsAttached();
	}


	// �̵� ���� ���� ���
	FVector Velocity = TFCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = TFCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = TFCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;
	bIsCrouching = TFCharacter->bIsCrouched;


	// ���� / ���� ���� ����
	bWeaponEquipped = TFCharacter->IsWeaponEquipped();
	bIsAiming = TFCharacter->IsAiming();
	bEliminated = TFCharacter->IsElimmed();


	// ���� ȸ���� ���
	FRotator AimRotation = TFCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(TFCharacter->GetVelocity());

	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 15.f);
	YawOffset = DeltaRotation.Yaw;

	AO_Yaw = TFCharacter->GETAO_YAW();
	AO_Pitch = TFCharacter->GETAO_PITCH();


	// ���� �� IK ����
	EquippedWeapon = TFCharacter->GetEquippedWeapon();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && TFCharacter->GetMesh())
	{
		// �޼� ��ġ ���� (IK)
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);

		FVector OutPos;
		FRotator OutRot;
		TFCharacter->GetMesh()->TransformToBoneSpace(FName("RightHand"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPos, OutRot);
		LeftHandTransform.SetLocation(OutPos);
		LeftHandTransform.SetRotation(FQuat(OutRot));

		// ������ ȸ�� ���� (���� �÷��̾� ����)
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


	// FABRIK ��� ���� ����
	ECombatState CurrentCombatState = TFCharacter->GetCombatState();
	bUseFABRIK = (CurrentCombatState == ECombatState::ECS_Unoccupied) && bWeaponEquipped && !bEliminated;
}
