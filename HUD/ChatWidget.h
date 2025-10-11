// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatWidget.generated.h"

/**
 * UChatWidget
 *
 * 인게임 채팅 위젯 클래스.
 * - 채팅 입력 및 표시 UI를 담당한다.
 * - 서버와 통신하여 메시지를 전송하고, 수신 메시지를 표시한다.
 */
UCLASS()
class UNREALPROJECT_7A_API UChatWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    // 채팅 로그 박스 (스크롤 영역)
    UPROPERTY(meta = (BindWidget))
    class UScrollBox* ChatBox;

    // 채팅 입력창
    UPROPERTY(meta = (BindWidget))
    class UEditableTextBox* ChatInput;

    // 메시지를 HUD에 추가
    UFUNCTION(BlueprintCallable)
    void AddChatMessage(const FString& PlayerName, const FString& Message);

    // 채팅창 열기
    void OpenChat();

    // 메시지 전송 후 닫기
    void SubmitAndClose();

    // 입력 취소 후 닫기
    void CancelAndClose();

    // 채팅창이 열려있는지 확인
    bool IsChatOpen() const { return bIsOpen; }

protected:
    virtual void NativeConstruct() override;
    void AutoClose(); // 일정 시간 후 자동 닫기

private:
    // 입력창에서 엔터 입력 시 실행되는 델리게이트 함수
    UFUNCTION()
    void OnInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);

    // 채팅창 상태 관리용
    bool bIsOpen = false;

    // 자동 닫기용 타이머
    FTimerHandle AutoCloseTimer;

    // 입력창 포커스 이동
    void FocusInput();

    // 게임 모드 입력으로 되돌리기
    void CloseToGameOnly(bool bKeepVisibleForFeed);

    // 서버로 채팅 메시지 전송
    void SendMessageToServer(const FString& Msg);

    // 채팅 로그 초기화
    UFUNCTION()
    void ClearChatMessages();
};
