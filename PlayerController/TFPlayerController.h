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
private:
	class ATFHUD* TfHud; // 메인캐릭터 허드	

protected:
	virtual void BeginPlay() override; // 플레이어 컨트롤러가 시작될 때 호출되는 함수
};
