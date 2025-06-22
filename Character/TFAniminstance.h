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
		virtual void NativeInitializeAnimation() override; //beginplay�� ����ϴ�
		virtual void NativeUpdateAnimation(float DeltaTime) override; //tick�� ����ϴ�
private:
	class AWeapon* EquippedWeapon; //������ ���⸦ �����ϴ� ����
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true")) //�������Ʈ���� �б��������� ��밡���ϴ�.
	class ATimeFractureCharacter* TFCharacter; //ĳ������ �����͸� �����Ѵ�.
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float Speed; //ĳ������ �ӵ��� �����Ѵ�.
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir; //ĳ���Ͱ� ���߿� �ִ��� ���θ� �����Ѵ�.
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating; //ĳ���Ͱ� ���������� ���θ� �����Ѵ�.
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	bool bWeaponEquipped;// ĳ���Ͱ� ���⸦ �����ߴ��� ���θ� �����Ѵ�.
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	bool bIsCrouching; //ĳ���Ͱ� ũ���ġ �������� ���θ� �����Ѵ�.
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	bool bIsAiming; //ĳ���Ͱ� ���� �������� ���θ� �����Ѵ�.
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float YawOffset; //ĳ������ Yaw �������� �����Ѵ�.

	FRotator DeltaRotation; //ȸ�� ���̸� �����ϴ� ����
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float AO_Yaw; //ĳ������ ���� ȸ�� Yaw ���� �����Ѵ�.
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float AO_Pitch; //ĳ������ ���� ȸ�� Pitch ���� �����Ѵ�.

	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	FTransform LeftHandTransform; //�޼��� ��ȯ ������ �����Ѵ�.
};
