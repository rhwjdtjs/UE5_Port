// Fill out your copyright notice in the Description page of Project Settings.


#include "TFAniminstance.h"
#include "TimeFractureCharacter.h" //캐릭터의 헤더파일을 포함시킨다
#include "GameFramework/CharacterMovementComponent.h" //캐릭터의 이동 컴포넌트를 포함시킨다
#include "Kismet/KismetMathLibrary.h" //수학 라이브러리를 포함시킨다
#include "UnrealProject_7A/TFComponents/CombatStates.h" //전투 상태를 포함시킨다
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
	if (TFCharacter == nullptr)
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
	FRotator AimRotation = TFCharacter->GetBaseAimRotation(); //캐릭터의 기본 조준 회전을 가져온다.
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(TFCharacter->GetVelocity()); //캐릭터의 이동 회전을 가져온다.
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);//이동 회전과 조준 회전의 차이를 계산하여 Yaw 오프셋을 저장한다.
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 15.f); //회전 차이를 보간한다.
	YawOffset = DeltaRotation.Yaw; //Yaw 오프셋을 저장한다.	
	AO_Yaw = TFCharacter->GETAO_YAW(); //캐릭터의 조준 회전 Yaw 값을 가져온다.
	AO_Pitch = TFCharacter->GETAO_PITCH(); //캐릭터의 조준 회전 Pitch 값을 가져온다.
	EquippedWeapon = TFCharacter->GetEquippedWeapon(); //캐릭터가 장착한 무기를 가져온다.

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && TFCharacter->GetMesh())
	{
		// 왼손 IK 설정 (기존 코드와 동일)
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPos;
		FRotator OutRot;
		TFCharacter->GetMesh()->TransformToBoneSpace(FName("RightHand"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPos, OutRot);
		LeftHandTransform.SetLocation(OutPos);
		LeftHandTransform.SetRotation(FQuat(OutRot));

		if (TFCharacter->IsLocallyControlled()) { // 로컬 플레이어가 조작하는 캐릭터인지 확인
			bLocallyControlled = true; // 로컬 플레이어가 조작하는 캐릭터인지 여부를 저장한다.
			FTransform MuzzleTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("muzz"), ERelativeTransformSpace::RTS_World); // 무기 메쉬의 "muzz" 소켓의 변환 정보를 가져온다.
			FVector MuzzleLocation = MuzzleTransform.GetLocation(); // 소켓의 위치를 가져온다.
			FVector AimTarget = TFCharacter->GetHitTarget(); // 캐릭터의 조준 목표 위치를 가져온다.
			FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, AimTarget); // 소켓 위치에서 조준 목표 위치를 바라보는 회전을 계산한다.
			FVector LookAtDirection = (AimTarget - MuzzleLocation).GetSafeNormal();// 조준 목표 방향을 계산한다.
			FVector UpDirection = FVector::UpVector;// 위 방향 벡터를 사용하여 회전을 계산한다.
			FRotator BaseRotation = UKismetMathLibrary::MakeRotFromYZ(-LookAtDirection, UpDirection);// YZ 평면에서 회전을 계산한다.
			FQuat CorrectionQuat(FVector::YAxisVector, FMath::DegreesToRadians(-90.f));// Y축을 기준으로 -90도 회전하는 쿼터니언을 생성한다.
			FQuat TargetWorldQuat = BaseRotation.Quaternion() * CorrectionQuat;// 계산된 회전을 쿼터니언으로 변환하고 -90도 회전을 적용한다.
			const FTransform& ComponentToWorld = TFCharacter->GetMesh()->GetComponentToWorld();// 캐릭터 메쉬의 컴포넌트 월드 변환을 가져온다.
			FQuat TargetComponentQuat = ComponentToWorld.InverseTransformRotation(TargetWorldQuat);// 컴포넌트 월드 변환을 사용하여 타겟 쿼터니언을 컴포넌트 공간으로 변환한다.
			RightHandRotation = FMath::RInterpTo(
				RightHandRotation,
				TargetComponentQuat.Rotator(),
				DeltaTime,
				3000.f // 보간 속도
			);// 오른손 회전 보간
				
		}
	}
	// FABRIK 상태 확인 및 디버그 로그
	ECombatState CurrentCombatState = TFCharacter->GetCombatState();
	bUseFABRIK = (CurrentCombatState == ECombatState::ECS_Unoccupied) && bWeaponEquipped && !bEliminated;
}
