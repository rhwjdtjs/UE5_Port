// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TFPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_7A_API ATFPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	void SetHUDHealth(float Health, float MaxHealth); // 허드의 체력을 설정하는 함수
	void SetHUDScore(float Score); // 허드의 점수를 설정하는 함수
	void SetHUDDefeats(int32 Defeats); // 허드의 처치 수를 설정하는 함수
	void SetHUDWeaponAmmo(int32 Ammos); // 허드의 처치 수를 설정하는 함수
	void SetHUDCarriedAmmo(int32 Ammos); // 허드의 보유 탄약을 설정하는 함수
	void SetHUDMatchCountdown(float CountdownTime); // 허드의 매치 카운트다운을 설정하는 함수 
	virtual void OnPossess(APawn* InPawn) override; // 플레이어가 Pawn을 소유할 때 호출되는 함수
	virtual void Tick(float DeltaTime) override; // 매 프레임마다 호출되는 함수
private:
	UPROPERTY()
	class ATFHUD* TfHud; // 메인캐릭터 허드	
	uint32 CountdownInt; // 카운트다운 시간 (초 단위)
	float MatchTime = 60.f; // 매치 시간 (초 단위)
protected:
	virtual void BeginPlay() override; // 플레이어 컨트롤러가 시작될 때 호출되는 함수
	void SetHUDTime(); // HUD의 시간을 설정하는 함수
};
