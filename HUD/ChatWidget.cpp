#include "ChatWidget.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "GameFramework/PlayerController.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"
#include "Framework/Application/SlateApplication.h"

/////////////////////////////////////////////////////////////
// ���:
//   ������ ������ �� �ʱ�ȭ �۾��� �����Ѵ�.
//   �Է�â ��������Ʈ�� ���ε��ϰ�, �⺻ ��Ŀ�� �Ӽ��� �����Ѵ�.
//
// �˰���:
//   - SetIsFocusable(true)�� Ű���� �Է� �����ϰ� ����.
//   - ChatInput�� ��ȿ�ϸ� OnTextCommitted ��������Ʈ�� OnInputCommitted ���ε�.
//   - �Է�â�� ���� ���� ���·� �����Ѵ�.
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
// ���:
//   ä�� �α׿� �޽����� �߰��ϰ� ȭ�鿡 ǥ���Ѵ�.
//
// �˰���:
//   - NewObject<UTextBlock>()���� ���ο� �ؽ�Ʈ ���� ����.
//   - SetText()�� "�÷��̾��̸�: �޽���" �������� ���ڿ� ǥ��.
//   - ChatBox->AddChild()�� UI�� �߰� �� ScrollToEnd()�� �ϴ����� ��ũ��.
//   - 17�� �� ClearChatMessages() ȣ���ϵ��� Ÿ�̸� ����.
//   - �޽����� ǥ�õ� �� ������ ������ ������ �ڵ����� ���̵��� ó��.
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
// ���:
//   ä��â�� ȭ�鿡 ǥ���ϰ� �Է� ��带 UI�� ��ȯ�Ѵ�.
//
// �˰���:
//   - SetVisibility(Visible)�� ǥ��.
//   - �÷��̾� ��Ʈ�ѷ��� ������ �Է� ��带 GameAndUI�� ����.
//   - SlateApplication::Get().SetKeyboardFocus()�� ���� ��Ŀ��.
//   - ���콺 Ŀ���� ���̰� �ϰ� �̵�/���� �Է��� ���´�.
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
// ���:
//   ä�� �Է� ������ ������ �����ϰ� â�� �ݴ´�.
//
// �˰���:
//   - ChatInput���� ���ڿ��� ������ ���� ���� �� �˻�.
//   - ������� ������ SendMessageToServer() ȣ��.
//   - �Է�â �ؽ�Ʈ �ʱ�ȭ �� CloseToGameOnly()�� ���� ���� ����.
//   - ���� �ڵ� �ݱ� Ÿ�̸Ӹ� ������ �� �ִ�.
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
// ���:
//   �ڵ� �ݱ� Ÿ�̸ӿ� ���� ȣ��ȴ�.
//
// �˰���:
//   - CancelAndClose() ȣ��� �Է� ��ҿ� �Բ� â �ݱ� ����.
/////////////////////////////////////////////////////////////
void UChatWidget::AutoClose()
{
    CancelAndClose();
}

/////////////////////////////////////////////////////////////
// ���:
//   �Է�â���� ���� Ű �Է� �� �޽����� �����Ѵ�.
//
// �˰���:
//   - ETextCommit::OnEnter�� ��쿡�� ó��.
//   - �Է� ���ڿ��� ������� ������ SendMessageToServer() ȣ�� �� �Է�â �ʱ�ȭ.
//   - SubmitAndClose()�� �ݱ� ó��.
//   - ���� Ÿ�̸Ӹ� ����ϰ� �� �ڵ� �ݱ� Ÿ�̸Ӹ� ����� �� �ִ�.
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
// ���:
//   ä�� �Է��� ����ϰ� â�� �ݴ´�.
//
// �˰���:
//   - �Է�â �ؽ�Ʈ �ʱ�ȭ �� CloseToGameOnly(false) ȣ��� ���� �ݱ� ����.
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
// ���:
//   ä�� �Է�â���� ��Ŀ���� ������ �̵���Ų��.
//
// �˰���:
//   - GetOwningPlayer()�� ��Ʈ�ѷ��� ������ SetUserFocus() ����.
//   - SetKeyboardFocus()�� Ű���� �Է��� ���� ���� �� �ְ� ����.
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
// ���:
//   ä�� UI �Է� ��忡�� �ٽ� ���� �Է� ���� �����Ѵ�.
//
// �˰���:
//   - PlayerController�� �Է� ��带 FInputModeGameOnly�� ����.
//   - ���콺 Ŀ�� ���� �� �̵�/���� �Է� ����.
//   - bKeepVisibleForFeed�� true�� �ǵ������ â�� ����� �Է��� ����.
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
// ���:
//   ������ ä�� �޽����� �����Ѵ�.
//
// �˰���:
//   - GetOwningPlayer()�� ���� �÷��̾� ��Ʈ�ѷ� ������.
//   - TFPlayerController�� ĳ���� �� ServerSendChatMessage() RPC ȣ��.
//   - ������ �޽����� �޾� �ٸ� Ŭ���̾�Ʈ�� ��ε�ĳ��Ʈ�Ѵ�.
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
// ���:
//   ���� �ð��� ������ ä�� �α׸� ����� â�� �ݴ´�.
//
// �˰���:
//   - ChatBox->ClearChildren()���� ��� �ؽ�Ʈ ������ ����.
//   - SetVisibility(Collapsed)�� â ���� ó��.
/////////////////////////////////////////////////////////////
void UChatWidget::ClearChatMessages()
{
    if (ChatBox)
    {
        ChatBox->ClearChildren();
    }
    SetVisibility(ESlateVisibility::Collapsed);
}
