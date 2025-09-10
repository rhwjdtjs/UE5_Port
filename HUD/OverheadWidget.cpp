// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h" // UTextBlock ��� ���� ����
#include "GameFramework/PlayerState.h" // APlayerState ��� ���� ����  
#include "Net/UnrealNetwork.h"
void UOverheadWidget::SetDisplayText(FString TextTODisplay)
{
	if (DisplayText) {
		DisplayText->SetText(FText::FromString(TextTODisplay)); // DisplayText�� ��ȿ�� ��쿡�� �ؽ�Ʈ ����
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
	RemoveFromParent(); // �θ�κ��� ����
	Super::NativeDestruct(); // �θ� Ŭ������ �޼��� ȣ��
}

