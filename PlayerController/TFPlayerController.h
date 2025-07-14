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
private:
	class ATFHUD* TfHud; // ����ĳ���� ���	

protected:
	virtual void BeginPlay() override; // �÷��̾� ��Ʈ�ѷ��� ���۵� �� ȣ��Ǵ� �Լ�
};
