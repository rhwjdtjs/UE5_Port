// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TFAniminstance.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_7A_API UTFAniminstance : public UAnimInstance
{
	GENERATED_BODY()
	


public:
		virtual void NativeInitializeAnimation() override; //beginplay와 비슷하다
		virtual void NativeUpdateAnimation(float DeltaTime) override; //tick과 비슷하다
private:
	class AWeapon* EquippedWeapon; //장착된 무기를 저장하는 변수
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true")) //블루프린트에서 읽기전용으로 사용가능하다.
	class ATimeFractureCharacter* TFCharacter; //캐릭터의 포인터를 저장한다.
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float Speed; //캐릭터의 속도를 저장한다.
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir; //캐릭터가 공중에 있는지 여부를 저장한다.
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating; //캐릭터가 가속중인지 여부를 저장한다.
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	bool bWeaponEquipped;// 캐릭터가 무기를 장착했는지 여부를 저장한다.
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	bool bIsCrouching; //캐릭터가 크라우치 상태인지 여부를 저장한다.
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	bool bIsAiming; //캐릭터가 조준 상태인지 여부를 저장한다.
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float YawOffset; //캐릭터의 Yaw 오프셋을 저장한다.

	FRotator DeltaRotation; //회전 차이를 저장하는 변수
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float AO_Yaw; //캐릭터의 조준 회전 Yaw 값을 저장한다.
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float AO_Pitch; //캐릭터의 조준 회전 Pitch 값을 저장한다.

	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	FTransform LeftHandTransform; //왼손의 변환 정보를 저장한다.
};
