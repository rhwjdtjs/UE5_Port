// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatWidget.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_7A_API UChatWidget : public UUserWidget
{
	GENERATED_BODY()
public:
    // ä�� �α�
    UPROPERTY(meta = (BindWidget))
    class UScrollBox* ChatBox;

    // �Է�â
    UPROPERTY(meta = (BindWidget))
    class UEditableTextBox* ChatInput;

    // �޽����� HUD�� �߰�
    UFUNCTION(BlueprintCallable)
    void AddChatMessage(const FString& PlayerName, const FString& Message);

    // ä�� ����
    void OpenChat();

    // �޽��� ���� + �ݱ�
    void SubmitAndClose();

    // ��� + �ݱ�
    void CancelAndClose();

    // �����ִ��� Ȯ��
    bool IsChatOpen() const { return bIsOpen; }

protected:
    virtual void NativeConstruct() override;
    void AutoClose();
private:
    UFUNCTION()                                              // �� ��������Ʈ��
    void OnInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);
    bool bIsOpen = false;
    FTimerHandle AutoCloseTimer;   // �ڵ� �ݱ�� Ÿ�̸� �ڵ�
    void FocusInput();
    void CloseToGameOnly(bool bKeepVisibleForFeed);
    void SendMessageToServer(const FString& Msg);
    UFUNCTION()
    void ClearChatMessages();
};
