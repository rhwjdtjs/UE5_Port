// Fill out your copyright notice in the Description page of Project Settings.


#include "ReturnToMainMenu.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameModeBase.h"
void UReturnToMainMenu::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;
	UWorld* World = GetWorld();
	if (World) {
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController) {
			FInputModeGameAndUI InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	if (MainMenuButton && OptionButton && !MainMenuButton->OnClicked.IsBound())
	{
		MainMenuButton->OnClicked.AddDynamic(this, &UReturnToMainMenu::MainMenuButtonClicked);
		OptionButton->OnClicked.AddDynamic(this, &UReturnToMainMenu::OptionButtonClicked);
	}
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance) {
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MultiplayerSessionsSubsystem) {
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMainMenu::OnDestroySession);
		}
	}
}
bool UReturnToMainMenu::Initialize()
{
	if(!Super::Initialize())
		return false;


	return true;
}
void UReturnToMainMenu::OnDestroySession(bool bWasSuccessful) {
	if (!bWasSuccessful) {
		MainMenuButton->SetIsEnabled(true);
		return;
	}
	UWorld* World = GetWorld();
	if (World) {
		AGameModeBase* GameMode=World->GetAuthGameMode<AGameModeBase>();
		if (GameMode) {
			GameMode->ReturnToMainMenuHost();
		}
		else {
			PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
			if(PlayerController) {
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
	}
}
void UReturnToMainMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World) {
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController) {
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
	if(MainMenuButton && MainMenuButton->OnClicked.IsBound())
	{
		MainMenuButton->OnClicked.RemoveDynamic(this, &UReturnToMainMenu::MainMenuButtonClicked);
	}
	if(MultiplayerSessionsSubsystem && MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsBound())
	{
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UReturnToMainMenu::OnDestroySession);
	}
}

void UReturnToMainMenu::MainMenuButtonClicked()
{
	MainMenuButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem) {
		MultiplayerSessionsSubsystem->DestroySession();
	}
}
void UReturnToMainMenu::OptionButtonClicked() {
	bOptionOpened = !bOptionOpened;
	if (bOptionOpened) {
		if (UWorld* World = GetWorld())
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				// WBP_MenuSettings À§Á¬ »ý¼º
				UUserWidget* MenuSettings = CreateWidget<UUserWidget>(PC,
					LoadClass<UUserWidget>(nullptr, TEXT("/Game/MENU_GRAPHICS/Widgets/WBP_MenuSettings.WBP_MenuSettings_C")));

				if (MenuSettings)
				{
					MenuSettings->AddToViewport();
					PC->bShowMouseCursor = true;
					FInputModeUIOnly InputMode;
					InputMode.SetWidgetToFocus(MenuSettings->TakeWidget());
					PC->SetInputMode(InputMode);
				}
			}
		}
	}
	else {
		if (UWorld* World = GetWorld())
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				TArray<UUserWidget*> FoundWidgets;
				UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World, FoundWidgets,
					LoadClass<UUserWidget>(nullptr, TEXT("/Game/MENU_GRAPHICS/Widgets/WBP_MenuSettings.WBP_MenuSettings_C")), false);

				for (UUserWidget* Widget : FoundWidgets)
				{
					Widget->RemoveFromParent();
				}
						FInputModeGameAndUI InputModeData;
						InputModeData.SetWidgetToFocus(TakeWidget());
						PC->SetInputMode(InputModeData);
						PC->SetShowMouseCursor(true);
			}
		}
	}
}


