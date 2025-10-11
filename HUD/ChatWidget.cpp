#include "ChatWidget.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "GameFramework/PlayerController.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"
#include "Framework/Application/SlateApplication.h"

/////////////////////////////////////////////////////////////
// 기능:
//   위젯이 생성될 때 초기화 작업을 수행한다.
//   입력창 델리게이트를 바인딩하고, 기본 포커스 속성을 설정한다.
//
// 알고리즘:
//   - SetIsFocusable(true)로 키보드 입력 가능하게 설정.
//   - ChatInput이 유효하면 OnTextCommitted 델리게이트에 OnInputCommitted 바인딩.
//   - 입력창을 편집 가능 상태로 설정한다.
/////////////////////////////////////////////////////////////
void UChatWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetIsFocusable(true);
    SetVisibility(ESlateVisibility::Collapsed);

    if (ChatInput)
    {
        ChatInput->OnTextCommitted.AddDynamic(this, &UChatWidget::OnInputCommitted);
        ChatInput->SetIsReadOnly(false);
    }
}

/////////////////////////////////////////////////////////////
// 기능:
//   채팅 로그에 메시지를 추가하고 화면에 표시한다.
//
// 알고리즘:
//   - NewObject<UTextBlock>()으로 새로운 텍스트 라인 생성.
//   - SetText()로 "플레이어이름: 메시지" 형식으로 문자열 표시.
//   - ChatBox->AddChild()로 UI에 추가 후 ScrollToEnd()로 하단으로 스크롤.
//   - 17초 후 ClearChatMessages() 호출하도록 타이머 예약.
//   - 메시지가 표시될 때 위젯이 숨겨져 있으면 자동으로 보이도록 처리.
/////////////////////////////////////////////////////////////
void UChatWidget::AddChatMessage(const FString& PlayerName, const FString& Message)
{
    UE_LOG(LogTemp, Warning, TEXT("AddChatMessage Called: %s : %s"), *PlayerName, *Message);

    if (!ChatBox) return;

    UTextBlock* NewLine = NewObject<UTextBlock>(this, UTextBlock::StaticClass());
    NewLine->SetText(FText::FromString(PlayerName + TEXT(": ") + Message));
    ChatBox->AddChild(NewLine);
    ChatBox->ScrollToEnd();

    if (GetVisibility() == ESlateVisibility::Collapsed)
    {
        SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AutoCloseTimer);
        World->GetTimerManager().SetTimer(
            AutoCloseTimer, this, &UChatWidget::ClearChatMessages, 17.f, false
        );
    }
}

/////////////////////////////////////////////////////////////
// 기능:
//   채팅창을 화면에 표시하고 입력 모드를 UI로 전환한다.
//
// 알고리즘:
//   - SetVisibility(Visible)로 표시.
//   - 플레이어 컨트롤러를 가져와 입력 모드를 GameAndUI로 변경.
//   - SlateApplication::Get().SetKeyboardFocus()로 강제 포커스.
//   - 마우스 커서를 보이게 하고 이동/시점 입력을 막는다.
/////////////////////////////////////////////////////////////
void UChatWidget::OpenChat()
{
    if (!ChatInput) return;

    SetVisibility(ESlateVisibility::Visible);

    if (APlayerController* PC = GetOwningPlayer())
    {
        ChatInput->SetUserFocus(PC);
        ChatInput->SetKeyboardFocus();

        if (ChatInput->GetCachedWidget().IsValid())
        {
            FSlateApplication::Get().SetKeyboardFocus(ChatInput->GetCachedWidget(), EFocusCause::SetDirectly);
        }

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

/////////////////////////////////////////////////////////////
// 기능:
//   채팅 입력 내용을 서버로 전송하고 창을 닫는다.
//
// 알고리즘:
//   - ChatInput에서 문자열을 가져와 공백 제거 후 검사.
//   - 비어있지 않으면 SendMessageToServer() 호출.
//   - 입력창 텍스트 초기화 후 CloseToGameOnly()로 게임 모드로 복귀.
//   - 이후 자동 닫기 타이머를 설정할 수 있다.
/////////////////////////////////////////////////////////////
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
}

/////////////////////////////////////////////////////////////
// 기능:
//   자동 닫기 타이머에 의해 호출된다.
//
// 알고리즘:
//   - CancelAndClose() 호출로 입력 취소와 함께 창 닫기 수행.
/////////////////////////////////////////////////////////////
void UChatWidget::AutoClose()
{
    CancelAndClose();
}

/////////////////////////////////////////////////////////////
// 기능:
//   입력창에서 엔터 키 입력 시 메시지를 전송한다.
//
// 알고리즘:
//   - ETextCommit::OnEnter인 경우에만 처리.
//   - 입력 문자열이 비어있지 않으면 SendMessageToServer() 호출 후 입력창 초기화.
//   - SubmitAndClose()로 닫기 처리.
//   - 이전 타이머를 취소하고 새 자동 닫기 타이머를 등록할 수 있다.
/////////////////////////////////////////////////////////////
void UChatWidget::OnInputCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (CommitMethod == ETextCommit::OnEnter)
    {
        const FString Msg = Text.ToString().TrimStartAndEnd();
        if (!Msg.IsEmpty())
        {
            SendMessageToServer(Msg);
            if (ChatInput) ChatInput->SetText(FText::GetEmpty());
            SubmitAndClose();
        }
    }
}

/////////////////////////////////////////////////////////////
// 기능:
//   채팅 입력을 취소하고 창을 닫는다.
//
// 알고리즘:
//   - 입력창 텍스트 초기화 후 CloseToGameOnly(false) 호출로 완전 닫기 수행.
/////////////////////////////////////////////////////////////
void UChatWidget::CancelAndClose()
{
    if (ChatInput)
    {
        ChatInput->SetText(FText::GetEmpty());
    }
    CloseToGameOnly(false);
}

/////////////////////////////////////////////////////////////
// 기능:
//   채팅 입력창으로 포커스를 강제로 이동시킨다.
//
// 알고리즘:
//   - GetOwningPlayer()로 컨트롤러를 가져와 SetUserFocus() 적용.
//   - SetKeyboardFocus()로 키보드 입력을 직접 받을 수 있게 설정.
/////////////////////////////////////////////////////////////
void UChatWidget::FocusInput()
{
    if (!ChatInput) return;

    if (APlayerController* PC = GetOwningPlayer())
    {
        ChatInput->SetUserFocus(PC);
    }

    ChatInput->SetKeyboardFocus();
}

/////////////////////////////////////////////////////////////
// 기능:
//   채팅 UI 입력 모드에서 다시 게임 입력 모드로 복귀한다.
//
// 알고리즘:
//   - PlayerController의 입력 모드를 FInputModeGameOnly로 설정.
//   - 마우스 커서 숨김 및 이동/시점 입력 복원.
//   - bKeepVisibleForFeed가 true면 피드용으로 창만 남기고 입력은 차단.
/////////////////////////////////////////////////////////////
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

    SetVisibility(bKeepVisibleForFeed ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
    bIsOpen = false;
}

/////////////////////////////////////////////////////////////
// 기능:
//   서버로 채팅 메시지를 전송한다.
//
// 알고리즘:
//   - GetOwningPlayer()로 현재 플레이어 컨트롤러 가져옴.
//   - TFPlayerController로 캐스팅 후 ServerSendChatMessage() RPC 호출.
//   - 서버는 메시지를 받아 다른 클라이언트로 브로드캐스트한다.
/////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////
// 기능:
//   일정 시간이 지나면 채팅 로그를 지우고 창을 닫는다.
//
// 알고리즘:
//   - ChatBox->ClearChildren()으로 모든 텍스트 라인을 제거.
//   - SetVisibility(Collapsed)로 창 숨김 처리.
/////////////////////////////////////////////////////////////
void UChatWidget::ClearChatMessages()
{
    if (ChatBox)
    {
        ChatBox->ClearChildren();
    }
    SetVisibility(ESlateVisibility::Collapsed);
}
