// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h" // UTextBlock 헤더 파일 포함
void UOverheadWidget::SetDisplayText(FString TextTODisplay)
{
	if (DisplayText) {
		DisplayText->SetText(FText::FromString(TextTODisplay)); // DisplayText가 유효한 경우에만 텍스트 설정
	}
}
#include "GameFramework/PlayerState.h" // APlayerState 헤더 파일 포함  

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)  
{  
   FString PlayerName;  
   if (InPawn)  
   {  
       APlayerState* PlayerState = InPawn->GetPlayerState<APlayerState>(); // 플레이어 상태 가져오기  
       if (PlayerState)  
       {  
           PlayerName = PlayerState->GetPlayerName(); // 플레이어 이름 가져오기  
       }  
   }  
   FString LocalRoleString = FString::Printf(TEXT("Name: %s"), *PlayerName); // 역할 문자열 생성  
   SetDisplayText(LocalRoleString); // 역할 문자열 설정  
}
void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent(); // 부모로부터 제거
	Super::NativeDestruct(); // 부모 클래스의 메서드 호출
}

