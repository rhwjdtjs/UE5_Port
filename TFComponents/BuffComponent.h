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
	void Heal(float HealAmount, float HealingTime); //힐 함수
	void Shield(float ShieldAmount, float ShieldTime); //쉴드 함수
	void BuffSpeed(float BaseSpeedBuff, float CrouchSpeedBuff, float SpeedBuffTime); //스피드 버프 함수
	void SetInitialSpeeds(float BaseSpeed, float CrouchBaseSpeed); //초기 스피드 설정 함수
	void SetInitialJump(float JumpZVelocity); //초기 점프 설정 함수'
	void BuffJump(float BuffJump, float JumpBuffTime); //점프 버프 함수
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void HealRampUp(float DeltaTime); //힐량 증가 함수
	void ShieldRampUp(float DeltaTime); //쉴드량 증가 함수
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	class ATimeFractureCharacter* Character; //캐릭터 클래스
	//Buff
	//Heal
	bool bHealing = false;
	float HealingRate = 0;//힐링 속도
	float AmountToHeal = 0; //힐량
	//Shield
	bool bShielding = false;
	float ShieldingRate = 0; //쉴드 속도
	float AmountToShield = 0; //쉴드량
	//Speed Buff
	FTimerHandle SpeedBuffTimer; //타이머 핸들
	void ResetSpeed(); //스피드 초기화 함수
	float InitialBaseSpeed; //기본 이동 속도
	float InitialCrouchSpeed; //기본 웅크린 속도
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed); //멀티캐스트 스피드 버프 함수
	//Jump Buff
	FTimerHandle JumpBuffTimer; //점프 타이머 핸들
	void ResetJump(); //점프 초기화 함수
	float InitialJumpZVelocity; //기본 점프 속도
	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpZVelocity); //멀티캐스트 스피드 버프 함수
};
