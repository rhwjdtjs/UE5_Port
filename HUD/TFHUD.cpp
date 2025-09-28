// Fill out your copyright notice in the Description page of Project Settings.


#include "TFHUD.h"
#include "GameFrameWork/PlayerController.h"
#include "CharacterOverlay.h"
#include "Alert.h"
#include "ChatWidget.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "UnrealProject_7A/TFComponents/CBComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
void ATFHUD::DrawHUD()
{
	Super::DrawHUD(); //drawhud�� ���̽� �Լ��� �ҷ���
	FVector2D ViewportSize; //ȭ�� ũ�⸦ ������ ����
	if (GEngine) {
		GEngine->GameViewport->GetViewportSize(ViewportSize); //ȭ�� ũ�⸦ ������
		const FVector2D ViewportCenter(ViewportSize.X / ViewportDiv, ViewportSize.Y / ViewportDiv); //ȭ�� �߾� ��ǥ ���
		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread; //ũ�ν���� �������� �ִ밪�� ���� �������带 ���Ͽ� �������� ũ�⸦ ���
		if (HUDPackage.CrosshairsCenter) {
			FVector2D Spread(0.f, 0.f); //ũ�ν���� �������� �ʱ�ȭ
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter,Spread ,HUDPackage.CrosshairColor); //�߾� ũ�ν���� �׸���
		}
		if (HUDPackage.CrosshairsLeft) {
			FVector2D Spread(-SpreadScaled, 0.f); //���� ũ�ν���� �������� ���
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairColor); //�߾� ũ�ν���� �׸���
		}
		if (HUDPackage.CrosshairsRight) {
			FVector2D Spread(SpreadScaled, 0.f); //������ ũ�ν���� �������� ���
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, HUDPackage.CrosshairColor); //�߾� ũ�ν���� �׸���
		}
		if (HUDPackage.CrosshairsTop) {
			FVector2D Spread(0.f, -SpreadScaled); //���� ũ�ν���� �������� ���
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread, HUDPackage.CrosshairColor); //�߾� ũ�ν���� �׸���
		}
		if (HUDPackage.CrosshairsBottom) {
			FVector2D Spread(0.f, SpreadScaled); //�Ʒ��� ũ�ν���� �������� ���
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter,Spread, HUDPackage.CrosshairColor); //�߾� ũ�ν���� �׸���
		}
	}
}

void ATFHUD::AddAlert()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !AlertClass) return;

	// 0) ����Ʈ�� ���� Ŭ������ Alert�� ������ ���� ����(�ߺ� AddToViewport ����)
	TArray<UUserWidget*> Existing;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC, Existing, AlertClass, false);
	for (UUserWidget* W : Existing)
	{
		if (W) W->RemoveFromParent();
	}

	// 1) �츮�� �����ϴ� �����͵� ����
	if (Alert)
	{
		Alert->RemoveFromParent();
		Alert = nullptr;
	}

	// 2) ���� ���� + ��� ���̾ �߰�
	Alert = CreateWidget<UAlert>(PC, AlertClass);
	if (Alert)
	{
		Alert->AddToViewport(5);
	}
}

void ATFHUD::AddChatWidget()
{
	APlayerController* PC = GetOwningPlayerController();
	if (PC && ChatWidgetClass)
	{
		ChatWidget = CreateWidget<UChatWidget>(PC, ChatWidgetClass);
		if (ChatWidget)
		{
			ChatWidget->AddToViewport();
		}
	}
}

void ATFHUD::BeginPlay()
{
	Super::BeginPlay();
}

void ATFHUD::AddCharacterOverlay()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !CharacterOverlayClass) return;

	// 0) Ȥ�� ����Ʈ�� ���� Ŭ������ ������ ���� ������ ���� ����(����/�ڵ� �ߺ� �߰� ���̽� ����)
	TArray<UUserWidget*> Existing;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC, Existing, CharacterOverlayClass, false);
	for (UUserWidget* W : Existing)
	{
		if (W) W->RemoveFromParent();
	}

	// 1) �츮�� �����ϴ� �����͵� ����
	if (CharacterOverlay)
	{
		CharacterOverlay->RemoveFromParent();
		CharacterOverlay = nullptr;
	}

	// 2) ���� ���� + ����Ʈ�� �߰�
	CharacterOverlay = CreateWidget<UCharacterOverlay>(PC, CharacterOverlayClass);
	if (CharacterOverlay)
	{
		CharacterOverlay->AddToViewport(1);

		// 3) ���� ���� Combat �� HUD �� ��� ����ȭ (������/���� ���� �� ȭ�� ����)
		if (APawn* P = PC->GetPawn())
		{
			if (ATimeFractureCharacter* C = Cast<ATimeFractureCharacter>(P))
			{
				if (UCBComponent* CB = C->GetCombatComponent())
				{
					CB->PushAllHUDFromCombat();
				}
			}
		}
	}
}

void ATFHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX(); //ũ�ν���� �ؽ����� �ʺ�
	const float TextureHeight = Texture->GetSizeY(); //ũ�ν���� �ؽ����� ����
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / ViewportDiv) + Spread.X,
		ViewportCenter.Y - (TextureHeight / ViewportDiv)+ Spread.Y); //ũ�ν���� �ؽ��ĸ� �׸� ��ġ ���
	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight,
		0.f, 0.f, 1.f, 1.f, CrosshairColor); //ũ�ν���� �ؽ��ĸ� ȭ�鿡 �׸���
}
