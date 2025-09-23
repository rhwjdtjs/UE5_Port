#include "ChatWidget.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "GameFramework/PlayerController.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"
#include "Framework/Application/SlateApplication.h"   // 추가 필요

void UChatWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetIsFocusable(true);
    SetVisibility(ESlateVisibility::Collapsed);

    if (ChatInput)
    {
        // ★ 엔터(커밋) 처리 바인딩
        ChatInput->OnTextCommitted.AddDynamic(this, &UChatWidget::OnInputCommitted);
        ChatInput->SetIsReadOnly(false);
    }
}

void UChatWidget::AddChatMessage(const FString& PlayerName, const FString& Message)
{
    UE_LOG(LogTemp, Warning, TEXT("AddChatMessage Called: %s : %s"), *PlayerName, *Message);

    if (!ChatBox) { UE_LOG(LogTemp, Warning, TEXT("ChatBox is NULL")); return; }

    UTextBlock* NewLine = NewObject<UTextBlock>(this, UTextBlock::StaticClass());
    NewLine->SetText(FText::FromString(PlayerName + TEXT(": ") + Message));
    ChatBox->AddChild(NewLine);

    ChatBox->ScrollToEnd();
    // 수신자 화면에 피드가 보이도록 강제 표시
    if (GetVisibility() == ESlateVisibility::Collapsed)
    {
        SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AutoCloseTimer);
        World->GetTimerManager().SetTimer(
            AutoCloseTimer, this, &UChatWidget::ClearChatMessages, 3.f, false
        );
    }
}

void UChatWidget::OpenChat()
{
    if (!ChatInput) return;

    SetVisibility(ESlateVisibility::Visible);

    if (APlayerController* PC = GetOwningPlayer())
    {
        // 기본 포커스
        ChatInput->SetUserFocus(PC);
        ChatInput->SetKeyboardFocus();

        // ★ Slate 강제 포커스 (이거 없으면 "클릭"을 요구하는 경우가 생김)
        if (ChatInput->GetCachedWidget().IsValid())
        {
            FSlateApplication::Get().SetKeyboardFocus(ChatInput->GetCachedWidget(), EFocusCause::SetDirectly);
        }

        // 입력 모드 전환
        PC->bShowMouseCursor = true;
        PC->SetIgnoreLookInput(true);
        PC->SetIgnoreMoveInput(true);

        FInputModeGameAndUI Mode;
        Mode.SetWidgetToFocus(ChatInput->TakeWidget());
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(Mode);
    }
    bIsOpen = true;
}

void UChatWidget::SubmitAndClose()
{
    if (!ChatInput) return;

    FString Msg = ChatInput->GetText().ToString().TrimStartAndEnd();
    if (!Msg.IsEmpty())
    {
        SendMessageToServer(Msg);
        ChatInput->SetText(FText::GetEmpty());
    }

    CloseToGameOnly(true);

    // ★ 3초 뒤 자동 닫기 예약
   // if (UWorld* World = GetWorld())
   // {
   //     World->GetTimerManager().ClearTimer(AutoCloseTimer); // 혹시 이전 타이머 있으면 취소
  //      World->GetTimerManager().SetTimer(
 //           AutoCloseTimer, this, &UChatWidget::AutoClose, 3.f, false
//    }
}


void UChatWidget::AutoClose()
{
    CancelAndClose();
}

void UChatWidget::OnInputCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (CommitMethod == ETextCommit::OnEnter) // ★ 엔터/넘패드엔터 모두 커밋으로 들어옴
    {
        const FString Msg = Text.ToString().TrimStartAndEnd();
        if (!Msg.IsEmpty())
        {
            SendMessageToServer(Msg);
            if (ChatInput) ChatInput->SetText(FText::GetEmpty());
            SubmitAndClose(); // 여기 하나로 정리 (보내기 + GameOnly 복귀)
        }
        // 3초 뒤 자동 닫기
      //  if (UWorld* World = GetWorld())
     //   {
     //       World->GetTimerManager().ClearTimer(AutoCloseTimer);
     //       World->GetTimerManager().SetTimer(
     //           AutoCloseTimer, this, &UChatWidget::AutoClose, 3.f, false
    //        );
   //     }
    }
}

void UChatWidget::CancelAndClose()
{
    if (ChatInput)
    {
        ChatInput->SetText(FText::GetEmpty());
    }

    CloseToGameOnly(false); // 완전히 닫기
}

void UChatWidget::FocusInput()
{
    if (!ChatInput) return;

    if (APlayerController* PC = GetOwningPlayer())
    {
        ChatInput->SetUserFocus(PC);
    }

    ChatInput->SetKeyboardFocus(); // 이걸로 충분
}

void UChatWidget::CloseToGameOnly(bool bKeepVisibleForFeed)
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        PC->bShowMouseCursor = false;
        PC->SetIgnoreLookInput(false);
        PC->SetIgnoreMoveInput(false);

        FInputModeGameOnly Mode;
        PC->SetInputMode(Mode);
    }

    // 피드 유지할지 여부 선택
    SetVisibility(bKeepVisibleForFeed ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
    bIsOpen = false;
}

void UChatWidget::SendMessageToServer(const FString& Msg)
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (ATFPlayerController* TFPC = Cast<ATFPlayerController>(PC))
        {
            TFPC->ServerSendChatMessage(Msg);
        }
    }
}

void UChatWidget::ClearChatMessages()
{
    if (ChatBox)
    {
        ChatBox->ClearChildren(); // 채팅 로그 싹 지우기
    }
    SetVisibility(ESlateVisibility::Collapsed); // 창 자체 숨기기
}
