// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatWidget.generated.h"

/**
 * UChatWidget
 *
 * �ΰ��� ä�� ���� Ŭ����.
 * - ä�� �Է� �� ǥ�� UI�� ����Ѵ�.
 * - ������ ����Ͽ� �޽����� �����ϰ�, ���� �޽����� ǥ���Ѵ�.
 */
UCLASS()
class UNREALPROJECT_7A_API UChatWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    // ä�� �α� �ڽ� (��ũ�� ����)
    UPROPERTY(meta = (BindWidget))
    class UScrollBox* ChatBox;

    // ä�� �Է�â
    UPROPERTY(meta = (BindWidget))
    class UEditableTextBox* ChatInput;

    // �޽����� HUD�� �߰�
    UFUNCTION(BlueprintCallable)
    void AddChatMessage(const FString& PlayerName, const FString& Message);

    // ä��â ����
    void OpenChat();

    // �޽��� ���� �� �ݱ�
    void SubmitAndClose();

    // �Է� ��� �� �ݱ�
    void CancelAndClose();

    // ä��â�� �����ִ��� Ȯ��
    bool IsChatOpen() const { return bIsOpen; }

protected:
    virtual void NativeConstruct() override;
    void AutoClose(); // ���� �ð� �� �ڵ� �ݱ�

private:
    // �Է�â���� ���� �Է� �� ����Ǵ� ��������Ʈ �Լ�
    UFUNCTION()
    void OnInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);

    // ä��â ���� ������
    bool bIsOpen = false;

    // �ڵ� �ݱ�� Ÿ�̸�
    FTimerHandle AutoCloseTimer;

    // �Է�â ��Ŀ�� �̵�
    void FocusInput();

    // ���� ��� �Է����� �ǵ�����
    void CloseToGameOnly(bool bKeepVisibleForFeed);

    // ������ ä�� �޽��� ����
    void SendMessageToServer(const FString& Msg);

    // ä�� �α� �ʱ�ȭ
    UFUNCTION()
    void ClearChatMessages();
};
