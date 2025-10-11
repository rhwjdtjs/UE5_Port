// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnToMainMenu.generated.h"

/**
 * UReturnToMainMenu
 *
 * �ΰ��� ���� ���� �޴��� �����ϰų� �ɼ� �޴��� ���� UI ����.
 * - ��Ƽ�÷��� ���� ���� �� �κ� ���� ó�� ����.
 */
UCLASS()
class UNREALPROJECT_7A_API UReturnToMainMenu : public UUserWidget
{
    GENERATED_BODY()
public:
    // ���� �޴��� ���ư��� ��ư
    UPROPERTY(meta = (BindWidget))
    class UButton* MainMenuButton;

    // �ɼ� �޴� ���� ��ư
    UPROPERTY(meta = (BindWidget))
    class UButton* OptionButton;

    // �޴� ǥ�� �� �Է� ��� ����
    void MenuSetup();

    // �޴� ���� �� �Է� ��� ����
    void MenuTearDown();

    // ���� �޴� ��ư Ŭ�� �� ȣ��
    UFUNCTION()
    void MainMenuButtonClicked();

    // �ɼ� ��ư Ŭ�� �� ȣ��
    UFUNCTION()
    void OptionButtonClicked();

protected:
    virtual bool Initialize() override;

    // ���� ���� �Ϸ� �� ȣ��Ǵ� �ݹ�
    UFUNCTION()
    void OnDestroySession(bool bWasSuccessful);

private:
    bool bOptionOpened = false; // �ɼ� �޴��� �����ִ��� ����
    UPROPERTY()
    class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem; // ���� ���� ����ý���
    UPROPERTY()
    class APlayerController* PlayerController; // ���� ���� �÷��̾� ��Ʈ�ѷ�
};
