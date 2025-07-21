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
	void SetHUDHealth(float Health, float MaxHealth); // ����� ü���� �����ϴ� �Լ�
	void SetHUDScore(float Score); // ����� ������ �����ϴ� �Լ�
	void SetHUDDefeats(int32 Defeats); // ����� óġ ���� �����ϴ� �Լ�
	void SetHUDWeaponAmmo(int32 Ammos); // ����� óġ ���� �����ϴ� �Լ�
	void SetHUDCarriedAmmo(int32 Ammos); // ����� ���� ź���� �����ϴ� �Լ�
	virtual void OnPossess(APawn* InPawn) override; // �÷��̾ Pawn�� ������ �� ȣ��Ǵ� �Լ�
private:
	UPROPERTY()
	class ATFHUD* TfHud; // ����ĳ���� ���	
protected:
	virtual void BeginPlay() override; // �÷��̾� ��Ʈ�ѷ��� ���۵� �� ȣ��Ǵ� �Լ�
};
