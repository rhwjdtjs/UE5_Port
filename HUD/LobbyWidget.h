// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyWidget.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_7A_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()
public:
    // Start 버튼 바인딩
    UPROPERTY(meta = (BindWidget))
    class UButton* StartButton;

    void ShowStartButton();
    void HideStartButton();
    UFUNCTION()
    void OnStartButtonClicked();
protected:
    virtual void NativeConstruct() override;
};
