// Fill out your copyright notice in the Description page of Project Settings.


#include "TFAniminstance.h"
#include "TimeFractureCharacter.h" //캐릭터의 헤더파일을 포함시킨다
#include "GameFramework/CharacterMovementComponent.h" //캐릭터의 이동 컴포넌트를 포함시킨다
void UTFAniminstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation(); //부모클래스의 초기화함수 호출

	TFCharacter = Cast<ATimeFractureCharacter>(TryGetPawnOwner()); //캐릭터의 포인터를 저장한다

}

void UTFAniminstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime); //부모클래스의 업데이트함수 호출
	if (TFCharacter == nullptr)
	{
		TFCharacter = Cast<ATimeFractureCharacter>(TryGetPawnOwner());
	}
	if(TFCharacter == nullptr)
	{
		return;
	}
	FVector Velocity = TFCharacter->GetVelocity(); //캐릭터의 속도를 가져온다.
	Velocity.Z = 0.f; //Z축의 속도를 0으로 만든다.
	Speed = Velocity.Size(); //캐릭터의 속도를 저장한다.
	bIsInAir = TFCharacter->GetCharacterMovement()->IsFalling(); //캐릭터가 공중에 있는지 여부를 저장한다.
	bIsAccelerating = TFCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false; //캐릭터가 가속중인지 여부를 저장한다.
	
}
