// Fill out your copyright notice in the Description page of Project Settings.


#include "TFAniminstance.h"
#include "TimeFractureCharacter.h" //ĳ������ ��������� ���Խ�Ų��
#include "GameFramework/CharacterMovementComponent.h" //ĳ������ �̵� ������Ʈ�� ���Խ�Ų��
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
	
}
