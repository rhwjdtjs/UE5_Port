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
    // 채팅 로그
    UPROPERTY(meta = (BindWidget))
    class UScrollBox* ChatBox;

    // 입력창
    UPROPERTY(meta = (BindWidget))
    class UEditableTextBox* ChatInput;

    // 메시지를 HUD에 추가
    UFUNCTION(BlueprintCallable)
    void AddChatMessage(const FString& PlayerName, const FString& Message);

    // 채팅 열기
    void OpenChat();

    // 메시지 전송 + 닫기
    void SubmitAndClose();

    // 취소 + 닫기
    void CancelAndClose();

    // 열려있는지 확인
    bool IsChatOpen() const { return bIsOpen; }

protected:
    virtual void NativeConstruct() override;
    void AutoClose();
private:
    UFUNCTION()                                              // ★ 델리게이트용
    void OnInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);
    bool bIsOpen = false;
    FTimerHandle AutoCloseTimer;   // 자동 닫기용 타이머 핸들
    void FocusInput();
    void CloseToGameOnly(bool bKeepVisibleForFeed);
    void SendMessageToServer(const FString& Msg);
    UFUNCTION()
    void ClearChatMessages();
};
