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
		//trygetpawnowner�� �� �ִ� �ν��Ͻ��� ���� �ִ� ���̷�Ż �޽��� ���� ���� �޾ƿ´�.
		//���⼱ ���� ���� ĳ���� Ŭ������ ĳ�����ؼ� �޾ƿ�
		//�ִ� �ν��Ͻ��� ������ �� ĳ������ ���¸� �����ϰԵ�

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

	//ĳ���� �����͸� ��ã���� �ٽ� �õ� ��ã���� �׳� ���� ��Ƽ�÷��̳� ���� Ÿ�̹� �̽����� ������ġ
	// ���̾� ���� ���� ����
	if (TFCharacter && TFCharacter->WireComponent)
	{
		bIsWireAttached = TFCharacter->WireComponent->IsAttached();
	}

	//���̾ �پ��ִ��� Ȯ���Ͽ� ������� ������
	// �̵� ���� ���� ���
	FVector Velocity = TFCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size(); //xy��鿡���� �ӵ� ũ�� z�� 0�ΰ��� ���� ���϶����� �ӵ��� ������ �޸��� �ִϸ��̼����� �ٲ�°� ��������

	bIsInAir = TFCharacter->GetCharacterMovement()->IsFalling(); //����������
	bIsAccelerating = TFCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f; //�÷��̾ ������ �̲������°��� �ƴ� ����Ű �̵�Ű�� ������ �ִ��� ����
	bIsCrouching = TFCharacter->bIsCrouched; //�ɱ����

	//�������� 3��Ī ���� �ִ��ν��Ͻ� ó��
	// ���� / ���� ���� ����
	bWeaponEquipped = TFCharacter->IsWeaponEquipped(); //���⸦ �������
	bIsAiming = TFCharacter->IsAiming(); //����������
	bEliminated = TFCharacter->IsElimmed(); //�����������


	// ���� ȸ���� ���
	FRotator AimRotation = TFCharacter->GetBaseAimRotation(); //ĳ���Ͱ� ���� ���콺�� �������� ����
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(TFCharacter->GetVelocity()); //ĳ���Ͱ� ������ �̵��ϰ� �ִ� ����

	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	//�ΰ��� ���̸� ���ϸ� �� ���� ������ �޸��� �ִµ� �ü��� ���ʹ����̴� ��� ������ ����
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 15.f);
	YawOffset = DeltaRotation.Yaw;
	//������ġ RInterpTo�� �̿��� �ε巴�� ��ȭ��Ű�� (��ü Ʈ����Ʈ �ִϸ��̼��� Ƣ�� �ʵ��� �ϱ����ؼ�
	//yaw�� �̵��� ��ü�� ������� Ʋ�������ϴ��� ����

	AO_Yaw = TFCharacter->GETAO_YAW(); //�¿� ���� ����
	AO_Pitch = TFCharacter->GETAO_PITCH(); //���Ʒ� ����
	//�ִԱ׷������� ���ӿ����� ��忡 ���µ�, ��ü�� ���� �迧 ������ ���̴�.
	//�� ũ�ν��� ��� �ΰ� �ֵ� ��ü �� �ѱ��� �� ������ ���󰡰���

	// ���� �� IK ����
	EquippedWeapon = TFCharacter->GetEquippedWeapon(); //���� ������� ��Ȯ�� ��� ����� �ϴ� �� �����κ�
	//IK / Inverse Kinematics (�����)

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && TFCharacter->GetMesh())
	{
		// �޼� ��ġ ���� (IK)
		//LefthandSocket�� �ִµ�, �޼��� ���� �κп� �ڿ������� ��ƾ���
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		//������ ���� ��ǥ�� ������
		FVector OutPos;
		FRotator OutRot;
		TFCharacter->GetMesh()->TransformToBoneSpace(FName("RightHand"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPos, OutRot);
		//ĳ���� ������ �� ������ ���� ��ǥ��� ��ȯ
		//�ֳĸ� ���� �����տ� �پ��ְ� �޼��� �� ���� �����ִ� ���̶� �޼��� ��ǥ ��ġ�� ������ ���� ��� ��ġ�� �Ѱ���� �ִ� �׷����� ��� IK���� �ڿ������� ���� �� ����.
		LeftHandTransform.SetLocation(OutPos);
		LeftHandTransform.SetRotation(FQuat(OutRot));

		// ������ ȸ�� ���� (���� �÷��̾� ����)
		if (TFCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			//�ѱ� ������ ������ġ�� ���Ѵ�.

			FTransform MuzzleTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("muzz"), ERelativeTransformSpace::RTS_World);
			FVector MuzzleLocation = MuzzleTransform.GetLocation();
			FVector AimTarget = TFCharacter->GetHitTarget();//ĳ���Ͱ� ���� ������ �������� ���� ��Ʈ Ÿ����ġ�� ������

			FVector LookAtDirection = (AimTarget - MuzzleLocation).GetSafeNormal();
			//�ѱ����� ��Ʈ Ÿ�ٱ����� ���⺤�͸� ����
			FRotator BaseRotation = UKismetMathLibrary::MakeRotFromYZ(-LookAtDirection, FVector::UpVector);

			FQuat CorrectionQuat(FVector::YAxisVector, FMath::DegreesToRadians(-90.f));
			FQuat TargetWorldQuat = BaseRotation.Quaternion() * CorrectionQuat;

			const FTransform& ComponentToWorld = TFCharacter->GetMesh()->GetComponentToWorld();
			FQuat TargetComponentQuat = ComponentToWorld.InverseTransformRotation(TargetWorldQuat); //ȸ���踦 �޽� ���� �������� �ٽ� ��ȯ

			RightHandRotation = FMath::RInterpTo(RightHandRotation, TargetComponentQuat.Rotator(), DeltaTime, 3000.f);
			//�����Ͽ� �ε巴�� ���󰡵����� (3000.F -> �Ｎ ����)
			//�� ������ �������� �������� ȸ���� �缳����
		}
	}


	// FABRIK ��� ���� ����
	ECombatState CurrentCombatState = TFCharacter->GetCombatState();
	bUseFABRIK = (CurrentCombatState == ECombatState::ECS_Unoccupied) && bWeaponEquipped && !bEliminated;
	//���� ��ų� �����ڼ� ������ ������� �ʰ� ����
	//FABRIK �� FORWARD AND BACKWARD REACHING INVERSE KINEMATICS �� �����̸� , �𸮾󿡼� �� �� , �Ȳ�ġ, ������� �ڿ������� ���߰� �ϴ� IK �ֹ��� �ϳ��̴�.
	//���������϶� �޼� IK�� ��
}
