// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h" // UTextBlock ��� ���� ����
void UOverheadWidget::SetDisplayText(FString TextTODisplay)
{
	if (DisplayText) {
		DisplayText->SetText(FText::FromString(TextTODisplay)); // DisplayText�� ��ȿ�� ��쿡�� �ؽ�Ʈ ����
	}
}
#include "GameFramework/PlayerState.h" // APlayerState ��� ���� ����  

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)  
{  
   FString PlayerName;  
   if (InPawn)  
   {  
       APlayerState* PlayerState = InPawn->GetPlayerState<APlayerState>(); // �÷��̾� ���� ��������  
       if (PlayerState)  
       {  
           PlayerName = PlayerState->GetPlayerName(); // �÷��̾� �̸� ��������  
       }  
   }  
   FString LocalRoleString = FString::Printf(TEXT("Name: %s"), *PlayerName); // ���� ���ڿ� ����  
   SetDisplayText(LocalRoleString); // ���� ���ڿ� ����  
}
void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent(); // �θ�κ��� ����
	Super::NativeDestruct(); // �θ� Ŭ������ �޼��� ȣ��
}

