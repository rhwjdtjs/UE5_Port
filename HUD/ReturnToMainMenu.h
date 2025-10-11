// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnToMainMenu.generated.h"

/**
 * UReturnToMainMenu
 *
 * 인게임 도중 메인 메뉴로 복귀하거나 옵션 메뉴를 여는 UI 위젯.
 * - 멀티플레이 세션 종료 및 로비 복귀 처리 포함.
 */
UCLASS()
class UNREALPROJECT_7A_API UReturnToMainMenu : public UUserWidget
{
    GENERATED_BODY()
public:
    // 메인 메뉴로 돌아가기 버튼
    UPROPERTY(meta = (BindWidget))
    class UButton* MainMenuButton;

    // 옵션 메뉴 열기 버튼
    UPROPERTY(meta = (BindWidget))
    class UButton* OptionButton;

    // 메뉴 표시 및 입력 모드 설정
    void MenuSetup();

    // 메뉴 종료 및 입력 모드 복귀
    void MenuTearDown();

    // 메인 메뉴 버튼 클릭 시 호출
    UFUNCTION()
    void MainMenuButtonClicked();

    // 옵션 버튼 클릭 시 호출
    UFUNCTION()
    void OptionButtonClicked();

protected:
    virtual bool Initialize() override;

    // 세션 종료 완료 시 호출되는 콜백
    UFUNCTION()
    void OnDestroySession(bool bWasSuccessful);

private:
    bool bOptionOpened = false; // 옵션 메뉴가 열려있는지 여부
    UPROPERTY()
    class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem; // 세션 관리 서브시스템
    UPROPERTY()
    class APlayerController* PlayerController; // 소유 중인 플레이어 컨트롤러
};
