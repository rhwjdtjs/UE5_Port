// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h" // UTextBlock 헤더 파일 포함
#include "GameFramework/PlayerState.h" // APlayerState 헤더 파일 포함  
#include "Net/UnrealNetwork.h"
void UOverheadWidget::SetDisplayText(FString TextTODisplay)
{
	if (DisplayText) {
		DisplayText->SetText(FText::FromString(TextTODisplay)); // DisplayText가 유효한 경우에만 텍스트 설정
	}
}


void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)  
{  
	APlayerState* PlayerState = InPawn->GetPlayerState();
	if (PlayerState)
	{
		FString PlayerName = PlayerState->GetPlayerName();
		FString LocalRoleString = FString::Printf(TEXT("%s"), *PlayerName);
		SetDisplayText(LocalRoleString);
	}
 
}
void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent(); // 부모로부터 제거
	Super::NativeDestruct(); // 부모 클래스의 메서드 호출
}

