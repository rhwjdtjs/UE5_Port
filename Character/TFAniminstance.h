// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TFAniminstance.generated.h"

/*
 * UTFAnimInstance
 *
 * 캐릭터의 애니메이션 상태를 매 프레임 갱신하는 클래스.
 * 이동, 점프, 무기 장착, 조준 등의 상태를 계산하여
 * 애니메이션 블루프린트에 전달한다.
 */
UCLASS()
class UNREALPROJECT_7A_API UTFAniminstance : public UAnimInstance
{
	GENERATED_BODY()
	


public:
	// 애니메이션 초기화 (BeginPlay 유사)
	virtual void NativeInitializeAnimation() override;

	// 애니메이션 상태 갱신 (Tick 유사)
	virtual void NativeUpdateAnimation(float DeltaTime) override;


	// 캐릭터 상태 관련
	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	bool bIsInAir; // 점프 중인지 여부

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	bool bIsCrouching; // 크라우치 상태인지 여부

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	float Speed; // 이동 속도

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating; // 가속 중인지 여부

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	bool bLocallyControlled; // 로컬 플레이어인지 여부

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	bool bEliminated; // 캐릭터가 사망 상태인지 여부


	// 전투 및 무기 관련
	UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bWeaponEquipped; // 무기 장착 여부

	UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bIsAiming; // 조준 중인지 여부

	UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bUseFABRIK; // FABRIK IK 사용 여부

	UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bIsWireAttached; // 와이어가 연결되어 있는지 여부


private:
	// 참조 포인터
	UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
	class ATimeFractureCharacter* TFCharacter; // 캐릭터 참조

	UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
	class AWeapon* EquippedWeapon; // 장착된 무기 참조


	// 조준 회전 관련
	UPROPERTY(BlueprintReadOnly, Category = "Aim", meta = (AllowPrivateAccess = "true"))
	float YawOffset; // 이동 방향과 조준 방향의 차이

	UPROPERTY(BlueprintReadOnly, Category = "Aim", meta = (AllowPrivateAccess = "true"))
	float AO_Yaw; // 조준 오프셋 Yaw

	UPROPERTY(BlueprintReadOnly, Category = "Aim", meta = (AllowPrivateAccess = "true"))
	float AO_Pitch; // 조준 오프셋 Pitch

	FRotator DeltaRotation; // 회전 보간용 변수


	// IK 관련
	UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
	FTransform LeftHandTransform; // 왼손 위치 보정값

	UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
	FRotator RightHandRotation; // 오른손 회전 보정값

	UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
	FRotator LeftHandRotation; // 왼손 회전 보정값
};
