#include "ChatWidget.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "GameFramework/PlayerController.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"
#include "Framework/Application/SlateApplication.h"   // �߰� �ʿ�

void UChatWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetIsFocusable(true);
    SetVisibility(ESlateVisibility::Collapsed);

    if (ChatInput)
    {
        // �� ����(Ŀ��) ó�� ���ε�
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
    // ������ ȭ�鿡 �ǵ尡 ���̵��� ���� ǥ��
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
        // �⺻ ��Ŀ��
        ChatInput->SetUserFocus(PC);
        ChatInput->SetKeyboardFocus();

        // �� Slate ���� ��Ŀ�� (�̰� ������ "Ŭ��"�� �䱸�ϴ� ��찡 ����)
        if (ChatInput->GetCachedWidget().IsValid())
        {
            FSlateApplication::Get().SetKeyboardFocus(ChatInput->GetCachedWidget(), EFocusCause::SetDirectly);
        }

        // �Է� ��� ��ȯ
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

    // �� 3�� �� �ڵ� �ݱ� ����
   // if (UWorld* World = GetWorld())
   // {
   //     World->GetTimerManager().ClearTimer(AutoCloseTimer); // Ȥ�� ���� Ÿ�̸� ������ ���
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
    if (CommitMethod == ETextCommit::OnEnter) // �� ����/���е忣�� ��� Ŀ������ ����
    {
        const FString Msg = Text.ToString().TrimStartAndEnd();
        if (!Msg.IsEmpty())
        {
            SendMessageToServer(Msg);
            if (ChatInput) ChatInput->SetText(FText::GetEmpty());
            SubmitAndClose(); // ���� �ϳ��� ���� (������ + GameOnly ����)
        }
        // 3�� �� �ڵ� �ݱ�
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

    CloseToGameOnly(false); // ������ �ݱ�
}

void UChatWidget::FocusInput()
{
    if (!ChatInput) return;

    if (APlayerController* PC = GetOwningPlayer())
    {
        ChatInput->SetUserFocus(PC);
    }

    ChatInput->SetKeyboardFocus(); // �̰ɷ� ���
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

    // �ǵ� �������� ���� ����
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
        ChatBox->ClearChildren(); // ä�� �α� �� �����
    }
    SetVisibility(ESlateVisibility::Collapsed); // â ��ü �����
}
