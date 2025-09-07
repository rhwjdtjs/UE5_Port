// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALPROJECT_7A_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBuffComponent();
	friend class ATimeFractureCharacter;
	void Heal(float HealAmount, float HealingTime); //�� �Լ�
	void Shield(float ShieldAmount, float ShieldTime); //���� �Լ�
	void BuffSpeed(float BaseSpeedBuff, float CrouchSpeedBuff, float SpeedBuffTime); //���ǵ� ���� �Լ�
	void SetInitialSpeeds(float BaseSpeed, float CrouchBaseSpeed); //�ʱ� ���ǵ� ���� �Լ�
	void SetInitialJump(float JumpZVelocity); //�ʱ� ���� ���� �Լ�'
	void BuffJump(float BuffJump, float JumpBuffTime); //���� ���� �Լ�
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void HealRampUp(float DeltaTime); //���� ���� �Լ�
	void ShieldRampUp(float DeltaTime); //���差 ���� �Լ�
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	class ATimeFractureCharacter* Character; //ĳ���� Ŭ����
	//Buff
	//Heal
	bool bHealing = false;
	float HealingRate = 0;//���� �ӵ�
	float AmountToHeal = 0; //����
	//Shield
	bool bShielding = false;
	float ShieldingRate = 0; //���� �ӵ�
	float AmountToShield = 0; //���差
	//Speed Buff
	FTimerHandle SpeedBuffTimer; //Ÿ�̸� �ڵ�
	void ResetSpeed(); //���ǵ� �ʱ�ȭ �Լ�
	float InitialBaseSpeed; //�⺻ �̵� �ӵ�
	float InitialCrouchSpeed; //�⺻ ��ũ�� �ӵ�
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed); //��Ƽĳ��Ʈ ���ǵ� ���� �Լ�
	//Jump Buff
	FTimerHandle JumpBuffTimer; //���� Ÿ�̸� �ڵ�
	void ResetJump(); //���� �ʱ�ȭ �Լ�
	float InitialJumpZVelocity; //�⺻ ���� �ӵ�
	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpZVelocity); //��Ƽĳ��Ʈ ���ǵ� ���� �Լ�
};
