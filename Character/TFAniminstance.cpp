// Fill out your copyright notice in the Description page of Project Settings.


#include "TFAniminstance.h"
#include "TimeFractureCharacter.h" //캐릭터의 헤더파일을 포함시킨다
#include "GameFramework/CharacterMovementComponent.h" //캐릭터의 이동 컴포넌트를 포함시킨다
#include "Kismet/KismetMathLibrary.h" //수학 라이브러리를 포함시킨다
#include "UnrealProject_7A/Weapon/Weapon.h" //무기의 헤더파일을 포함시킨다
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
	bWeaponEquipped = TFCharacter->IsWeaponEquipped(); //캐릭터가 무기를 장착했는지 여부를 저장한다.
	bIsCrouching = TFCharacter->bIsCrouched; //캐릭터가 크라우치 상태인지 여부를 저장한다.
	bIsAiming = TFCharacter->IsAiming(); //캐릭터가 조준 상태인지 여부를 저장한다.
	bEliminated = TFCharacter->IsElimmed(); //캐릭터가 제거되었는지 여부를 저장한다.
	FRotator AimRotation=TFCharacter->GetBaseAimRotation(); //캐릭터의 기본 조준 회전을 가져온다.
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(TFCharacter->GetVelocity()); //캐릭터의 이동 회전을 가져온다.
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);//이동 회전과 조준 회전의 차이를 계산하여 Yaw 오프셋을 저장한다.
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot,DeltaTime,15.f); //회전 차이를 보간한다.
	YawOffset = DeltaRotation.Yaw; //Yaw 오프셋을 저장한다.	
	AO_Yaw = TFCharacter->GETAO_YAW(); //캐릭터의 조준 회전 Yaw 값을 가져온다.
	AO_Pitch = TFCharacter->GETAO_PITCH(); //캐릭터의 조준 회전 Pitch 값을 가져온다.
	EquippedWeapon = TFCharacter->GetEquippedWeapon(); //캐릭터가 장착한 무기를 가져온다.

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && TFCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World); //왼손의 변환 정보를 가져온다.

		FVector OutPos;
		FRotator OutRot;
		TFCharacter->GetMesh()->TransformToBoneSpace(FName("RightHand"), LeftHandTransform.GetLocation(),
			FRotator::ZeroRotator, OutPos, OutRot);
		LeftHandTransform.SetLocation(OutPos); //왼손의 위치를 설정한다.
		LeftHandTransform.SetRotation(FQuat(OutRot)); //왼손의 회전을 설정한다.

		if (TFCharacter->IsLocallyControlled()) {
			bLocallyControlled = true; //로컬 컨트롤러인 경우 true로 설정한다.
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("RightHand"), ERelativeTransformSpace::RTS_World); //오른손의 변환 정보를 가져온다.
			RightHandRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), TFCharacter->GetHitTarget()); //오른손의 회전을 계산한다.
			FRotator RightHandAdd = RightHandRotation + FRotator(90.f, 90.f, 0.f); //오른손 회전에 90도씩 추가한다.
			RightHandRotation = RightHandAdd; //오른손 회전을 설정한다.
		}
		
		
	}
}
