// Fill out your copyright notice in the Description page of Project Settings.

#include "ReturnToMainMenu.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameModeBase.h"

/////////////////////////////////////////////////////////////
// 기능:
//   위젯을 화면에 표시하고, 입력 모드를 UI로 변경한다.
//   버튼 클릭 이벤트 및 세션 이벤트를 초기화한다.
//
// 알고리즘:
//   - AddToViewport()로 화면에 위젯 추가.
//   - FInputModeGameAndUI로 마우스와 UI 입력 모두 허용.
//   - 버튼 델리게이트 바인딩(AddDynamic)으로 클릭 이벤트 연결.
//   - GameInstance에서 MultiplayerSessionsSubsystem 가져와
//     세션 종료 콜백(OnDestroySession)에 바인딩한다.
/////////////////////////////////////////////////////////////
void UReturnToMainMenu::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
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
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MultiplayerSessionsSubsystem)
		{
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMainMenu::OnDestroySession);
		}
	}
}

/////////////////////////////////////////////////////////////
// 기능:
//   위젯 초기화 시 호출되는 함수.
//
// 알고리즘:
//   - 부모 클래스의 Initialize()를 호출하여 기본 초기화를 수행.
//   - 반환값을 통해 성공 여부 판단.
/////////////////////////////////////////////////////////////
bool UReturnToMainMenu::Initialize()
{
	if (!Super::Initialize())
		return false;

	return true;
}

/////////////////////////////////////////////////////////////
// 기능:
//   세션이 정상적으로 종료된 후, 메인 메뉴로 복귀 처리.
//
// 알고리즘:
//   - bWasSuccessful이 false이면 버튼을 다시 활성화 후 종료.
//   - 서버(호스트)일 경우 GameMode에서 ReturnToMainMenuHost() 호출.
//   - 클라이언트일 경우 PlayerController의
//     ClientReturnToMainMenuWithTextReason() 호출.
/////////////////////////////////////////////////////////////
void UReturnToMainMenu::OnDestroySession(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		MainMenuButton->SetIsEnabled(true);
		return;
	}

	UWorld* World = GetWorld();
	if (World)
	{
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
		if (GameMode)
		{
			GameMode->ReturnToMainMenuHost();
		}
		else
		{
			PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
			if (PlayerController)
			{
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
	}
}

/////////////////////////////////////////////////////////////
// 기능:
//   메뉴를 닫고 입력 모드를 게임으로 되돌린다.
//
// 알고리즘:
//   - RemoveFromParent()로 위젯 제거.
//   - FInputModeGameOnly로 전환하여 마우스 및 UI 입력 비활성화.
//   - 버튼 및 델리게이트 언바인딩으로 메모리 정리.
/////////////////////////////////////////////////////////////
void UReturnToMainMenu::MenuTearDown()
{
	RemoveFromParent();

	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}

	if (MainMenuButton && MainMenuButton->OnClicked.IsBound())
	{
		MainMenuButton->OnClicked.RemoveDynamic(this, &UReturnToMainMenu::MainMenuButtonClicked);
	}
	if (MultiplayerSessionsSubsystem && MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsBound())
	{
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UReturnToMainMenu::OnDestroySession);
	}
}

/////////////////////////////////////////////////////////////
// 기능:
//   메인 메뉴 버튼 클릭 시 세션을 종료한다.
//
// 알고리즘:
//   - 버튼을 비활성화하여 중복 입력 방지.
//   - 세션 서브시스템의 DestroySession() 호출로 세션 종료 요청.
/////////////////////////////////////////////////////////////
void UReturnToMainMenu::MainMenuButtonClicked()
{
	MainMenuButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->DestroySession();
	}
}

/////////////////////////////////////////////////////////////
// 기능:
//   옵션 버튼 클릭 시 옵션 메뉴를 열거나 닫는다.
//
// 알고리즘:
//   - bOptionOpened 불리언을 반전시켜 현재 상태를 토글.
//   - 열기 상태(true):
//       • CreateWidget()으로 /Game/MENU_GRAPHICS/Widgets/WBP_MenuSettings 생성.
//       • AddToViewport()로 표시 후 InputModeUIOnly 적용.
//   - 닫기 상태(false):
//       • UWidgetBlueprintLibrary::GetAllWidgetsOfClass()로 메뉴 위젯 검색 후 제거.
//       • 다시 FInputModeGameAndUI로 복귀.
/////////////////////////////////////////////////////////////
void UReturnToMainMenu::OptionButtonClicked()
{
	bOptionOpened = !bOptionOpened;

	if (bOptionOpened)
	{
		if (UWorld* World = GetWorld())
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				UUserWidget* MenuSettings = CreateWidget<UUserWidget>(
					PC, LoadClass<UUserWidget>(nullptr, TEXT("/Game/MENU_GRAPHICS/Widgets/WBP_MenuSettings.WBP_MenuSettings_C")));

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
	else
	{
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
