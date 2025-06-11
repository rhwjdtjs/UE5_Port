// Fill out your copyright notice in the Description page of Project Settings.


#include "TFAniminstance.h"
#include "TimeFractureCharacter.h" //ĳ������ ��������� ���Խ�Ų��
#include "GameFramework/CharacterMovementComponent.h" //ĳ������ �̵� ������Ʈ�� ���Խ�Ų��
#include "Kismet/KismetMathLibrary.h" //���� ���̺귯���� ���Խ�Ų��
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
	if(TFCharacter == nullptr)
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

	FRotator AimRotation=TFCharacter->GetBaseAimRotation(); //ĳ������ �⺻ ���� ȸ���� �����´�.
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(TFCharacter->GetVelocity()); //ĳ������ �̵� ȸ���� �����´�.
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);//�̵� ȸ���� ���� ȸ���� ���̸� ����Ͽ� Yaw �������� �����Ѵ�.
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot,DeltaTime,15.f); //ȸ�� ���̸� �����Ѵ�.
	YawOffset = DeltaRotation.Yaw; //Yaw �������� �����Ѵ�.	
	AO_Yaw = TFCharacter->GETAO_YAW(); //ĳ������ ���� ȸ�� Yaw ���� �����´�.
	AO_Pitch = TFCharacter->GETAO_PITCH(); //ĳ������ ���� ȸ�� Pitch ���� �����´�.
}
