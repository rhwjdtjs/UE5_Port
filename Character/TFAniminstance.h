// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TFAniminstance.generated.h"

/*
 * UTFAnimInstance
 *
 * ĳ������ �ִϸ��̼� ���¸� �� ������ �����ϴ� Ŭ����.
 * �̵�, ����, ���� ����, ���� ���� ���¸� ����Ͽ�
 * �ִϸ��̼� �������Ʈ�� �����Ѵ�.
 */
UCLASS()
class UNREALPROJECT_7A_API UTFAniminstance : public UAnimInstance
{
	GENERATED_BODY()
	


public:
	// �ִϸ��̼� �ʱ�ȭ (BeginPlay ����)
	virtual void NativeInitializeAnimation() override;

	// �ִϸ��̼� ���� ���� (Tick ����)
	virtual void NativeUpdateAnimation(float DeltaTime) override;


	// ĳ���� ���� ����
	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	bool bIsInAir; // ���� ������ ����

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	bool bIsCrouching; // ũ���ġ �������� ����

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	float Speed; // �̵� �ӵ�

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating; // ���� ������ ����

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	bool bLocallyControlled; // ���� �÷��̾����� ����

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	bool bEliminated; // ĳ���Ͱ� ��� �������� ����


	// ���� �� ���� ����
	UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bWeaponEquipped; // ���� ���� ����

	UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bIsAiming; // ���� ������ ����

	UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bUseFABRIK; // FABRIK IK ��� ����

	UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bIsWireAttached; // ���̾ ����Ǿ� �ִ��� ����


private:
	// ���� ������
	UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
	class ATimeFractureCharacter* TFCharacter; // ĳ���� ����

	UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
	class AWeapon* EquippedWeapon; // ������ ���� ����


	// ���� ȸ�� ����
	UPROPERTY(BlueprintReadOnly, Category = "Aim", meta = (AllowPrivateAccess = "true"))
	float YawOffset; // �̵� ����� ���� ������ ����

	UPROPERTY(BlueprintReadOnly, Category = "Aim", meta = (AllowPrivateAccess = "true"))
	float AO_Yaw; // ���� ������ Yaw

	UPROPERTY(BlueprintReadOnly, Category = "Aim", meta = (AllowPrivateAccess = "true"))
	float AO_Pitch; // ���� ������ Pitch

	FRotator DeltaRotation; // ȸ�� ������ ����


	// IK ����
	UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
	FTransform LeftHandTransform; // �޼� ��ġ ������

	UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
	FRotator RightHandRotation; // ������ ȸ�� ������

	UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
	FRotator LeftHandRotation; // �޼� ȸ�� ������
};
