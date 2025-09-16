// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnToMainMenu.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_7A_API UReturnToMainMenu : public UUserWidget
{
	GENERATED_BODY()
public:
    UPROPERTY(meta = (BindWidget))
    class UButton* MainMenuButton;
    UPROPERTY(meta = (BindWidget))
    class UButton* OptionButton;
    void MenuSetup();
	void MenuTearDown();
    UFUNCTION()
	void MainMenuButtonClicked();
    UFUNCTION()
	void OptionButtonClicked();
protected:
	virtual bool Initialize() override;
	UFUNCTION()	
	void OnDestroySession(bool bWasSuccessful);
private:
	bool bOptionOpened = false;
	UPROPERTY()
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;
	UPROPERTY()
	class APlayerController* PlayerController;
};
