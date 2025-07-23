// Fill out your copyright notice in the Description page of Project Settings.


#include "TFAniminstance.h"
#include "TimeFractureCharacter.h" //ĳ������ ��������� ���Խ�Ų��
#include "GameFramework/CharacterMovementComponent.h" //ĳ������ �̵� ������Ʈ�� ���Խ�Ų��
#include "Kismet/KismetMathLibrary.h" //���� ���̺귯���� ���Խ�Ų��
#include "UnrealProject_7A/TFComponents/CombatStates.h" //���� ���¸� ���Խ�Ų��
#include "UnrealProject_7A/Weapon/Weapon.h" //������ ��������� ���Խ�Ų��
void UTFAniminstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation(); //�θ�Ŭ������ �ʱ�ȭ�Լ� ȣ��

	TFCharacter = Cast<ATimeFractureCharacter>(TryGetPawnOwner()); //ĳ������ �����͸� �����Ѵ�

}

void UTFAniminstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime); //�θ�Ŭ������ ������Ʈ�Լ� ȣ��
	if (TFCharacter == nullptr)
	{
		TFCharacter = Cast<ATimeFractureCharacter>(TryGetPawnOwner());
	}
	if (TFCharacter == nullptr)
	{
		return;
	}
	FVector Velocity = TFCharacter->GetVelocity(); //ĳ������ �ӵ��� �����´�.
	Velocity.Z = 0.f; //Z���� �ӵ��� 0���� �����.
	Speed = Velocity.Size(); //ĳ������ �ӵ��� �����Ѵ�.
	bIsInAir = TFCharacter->GetCharacterMovement()->IsFalling(); //ĳ���Ͱ� ���߿� �ִ��� ���θ� �����Ѵ�.
	bIsAccelerating = TFCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false; //ĳ���Ͱ� ���������� ���θ� �����Ѵ�.
	bWeaponEquipped = TFCharacter->IsWeaponEquipped(); //ĳ���Ͱ� ���⸦ �����ߴ��� ���θ� �����Ѵ�.
	bIsCrouching = TFCharacter->bIsCrouched; //ĳ���Ͱ� ũ���ġ �������� ���θ� �����Ѵ�.
	bIsAiming = TFCharacter->IsAiming(); //ĳ���Ͱ� ���� �������� ���θ� �����Ѵ�.
	bEliminated = TFCharacter->IsElimmed(); //ĳ���Ͱ� ���ŵǾ����� ���θ� �����Ѵ�.
	FRotator AimRotation = TFCharacter->GetBaseAimRotation(); //ĳ������ �⺻ ���� ȸ���� �����´�.
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(TFCharacter->GetVelocity()); //ĳ������ �̵� ȸ���� �����´�.
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);//�̵� ȸ���� ���� ȸ���� ���̸� ����Ͽ� Yaw �������� �����Ѵ�.
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 15.f); //ȸ�� ���̸� �����Ѵ�.
	YawOffset = DeltaRotation.Yaw; //Yaw �������� �����Ѵ�.	
	AO_Yaw = TFCharacter->GETAO_YAW(); //ĳ������ ���� ȸ�� Yaw ���� �����´�.
	AO_Pitch = TFCharacter->GETAO_PITCH(); //ĳ������ ���� ȸ�� Pitch ���� �����´�.
	EquippedWeapon = TFCharacter->GetEquippedWeapon(); //ĳ���Ͱ� ������ ���⸦ �����´�.

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && TFCharacter->GetMesh())
	{
		// �޼� IK ���� (���� �ڵ�� ����)
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPos;
		FRotator OutRot;
		TFCharacter->GetMesh()->TransformToBoneSpace(FName("RightHand"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPos, OutRot);
		LeftHandTransform.SetLocation(OutPos);
		LeftHandTransform.SetRotation(FQuat(OutRot));

		if (TFCharacter->IsLocallyControlled()) { // ���� �÷��̾ �����ϴ� ĳ�������� Ȯ��
			bLocallyControlled = true; // ���� �÷��̾ �����ϴ� ĳ�������� ���θ� �����Ѵ�.
			FTransform MuzzleTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("muzz"), ERelativeTransformSpace::RTS_World); // ���� �޽��� "muzz" ������ ��ȯ ������ �����´�.
			FVector MuzzleLocation = MuzzleTransform.GetLocation(); // ������ ��ġ�� �����´�.
			FVector AimTarget = TFCharacter->GetHitTarget(); // ĳ������ ���� ��ǥ ��ġ�� �����´�.
			FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, AimTarget); // ���� ��ġ���� ���� ��ǥ ��ġ�� �ٶ󺸴� ȸ���� ����Ѵ�.
			FVector LookAtDirection = (AimTarget - MuzzleLocation).GetSafeNormal();// ���� ��ǥ ������ ����Ѵ�.
			FVector UpDirection = FVector::UpVector;// �� ���� ���͸� ����Ͽ� ȸ���� ����Ѵ�.
			FRotator BaseRotation = UKismetMathLibrary::MakeRotFromYZ(-LookAtDirection, UpDirection);// YZ ��鿡�� ȸ���� ����Ѵ�.
			FQuat CorrectionQuat(FVector::YAxisVector, FMath::DegreesToRadians(-90.f));// Y���� �������� -90�� ȸ���ϴ� ���ʹϾ��� �����Ѵ�.
			FQuat TargetWorldQuat = BaseRotation.Quaternion() * CorrectionQuat;// ���� ȸ���� ���ʹϾ����� ��ȯ�ϰ� -90�� ȸ���� �����Ѵ�.
			const FTransform& ComponentToWorld = TFCharacter->GetMesh()->GetComponentToWorld();// ĳ���� �޽��� ������Ʈ ���� ��ȯ�� �����´�.
			FQuat TargetComponentQuat = ComponentToWorld.InverseTransformRotation(TargetWorldQuat);// ������Ʈ ���� ��ȯ�� ����Ͽ� Ÿ�� ���ʹϾ��� ������Ʈ �������� ��ȯ�Ѵ�.
			RightHandRotation = FMath::RInterpTo(
				RightHandRotation,
				TargetComponentQuat.Rotator(),
				DeltaTime,
				3000.f // ���� �ӵ�
			);// ������ ȸ�� ����
				
		}
	}
	// FABRIK ���� Ȯ�� �� ����� �α�
	ECombatState CurrentCombatState = TFCharacter->GetCombatState();
	bUseFABRIK = (CurrentCombatState == ECombatState::ECS_Unoccupied) && bWeaponEquipped && !bEliminated;
}
