// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyWidget.h"
#include "Components/Button.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"
void ULobbyWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (StartButton)
    {
        StartButton->SetVisibility(ESlateVisibility::Collapsed); // 기본은 숨김

        // 버튼 클릭 시 PlayerController의 RequestStartMatch 호출
        StartButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnStartButtonClicked);
    }
}

void ULobbyWidget::ShowStartButton()
{
    if (StartButton)
    {
        StartButton->SetVisibility(ESlateVisibility::Visible);
    }
}

void ULobbyWidget::HideStartButton()
{
    if (StartButton)
    {
        StartButton->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void ULobbyWidget::OnStartButtonClicked()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (ATFPlayerController* TFPC = Cast<ATFPlayerController>(PC))
        {
            TFPC->RequestStartMatch();
        }
    }
}

