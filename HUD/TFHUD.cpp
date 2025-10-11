#include "TFHUD.h"
#include "GameFramework/PlayerController.h"
#include "CharacterOverlay.h"
#include "Alert.h"
#include "ChatWidget.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "UnrealProject_7A/TFComponents/CBComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

/////////////////////////////////////////////////////////////
// ���:
//   HUD�� �� ������ �׸��� �Լ�.
//   ũ�ν���� �ؽ�ó�� ȭ�� �߽ɿ� ǥ���Ѵ�.
//
// �˰���:
//   - GetViewportSize()�� ���� ȭ�� ũ�⸦ ��´�.
//   - ȭ�� �߽� ��ǥ(ViewportCenter)�� ����Ѵ�.
//   - CrosshairSpreadMax�� ���� �������� ���� ���� Ȯ�� �Ÿ� ���.
//   - �� ����(�߾�, ��, ��, ��, ��)�� ���� DrawCrosshair() ȣ��.
//   - DrawTexture()�� ����� UTexture2D�� ���� �׸���.
/////////////////////////////////////////////////////////////
void ATFHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / ViewportDiv, ViewportSize.Y / ViewportDiv);
		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrosshairsCenter)
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, FVector2D(0.f, 0.f), HUDPackage.CrosshairColor);
		if (HUDPackage.CrosshairsLeft)
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, FVector2D(-SpreadScaled, 0.f), HUDPackage.CrosshairColor);
		if (HUDPackage.CrosshairsRight)
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, FVector2D(SpreadScaled, 0.f), HUDPackage.CrosshairColor);
		if (HUDPackage.CrosshairsTop)
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, FVector2D(0.f, -SpreadScaled), HUDPackage.CrosshairColor);
		if (HUDPackage.CrosshairsBottom)
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, FVector2D(0.f, SpreadScaled), HUDPackage.CrosshairColor);
	}
}

/////////////////////////////////////////////////////////////
// ���:
//   �˸�(Alert) ������ �߰��Ѵ�.
//
// �˰���:
//   - UWidgetBlueprintLibrary::GetAllWidgetsOfClass()�� ���� Alert ������ ��� ������ �ߺ� ����.
//   - CreateWidget()���� �� Alert ���� ���� �� AddToViewport(5)�� ǥ��.
//   - ���� Alert �����Ͱ� �����ϸ� ���� �� ����.
/////////////////////////////////////////////////////////////
void ATFHUD::AddAlert()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !AlertClass) return;

	TArray<UUserWidget*> Existing;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC, Existing, AlertClass, false);
	for (UUserWidget* W : Existing)
	{
		if (W) W->RemoveFromParent();
	}

	if (Alert)
	{
		Alert->RemoveFromParent();
		Alert = nullptr;
	}

	Alert = CreateWidget<UAlert>(PC, AlertClass);
	if (Alert)
	{
		Alert->AddToViewport(5);
	}
}

/////////////////////////////////////////////////////////////
// ���:
//   ä�� ������ ȭ�鿡 ǥ���Ѵ�.
//
// �˰���:
//   - �÷��̾� ��Ʈ�ѷ��� ���� CreateWidget()���� UChatWidget ����.
//   - AddToViewport()�� ����Ʈ�� �߰�.
//   - �̹� �������� ���� ��쿡�� �����Ѵ�.
/////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////
// ���:
//   ĳ���� ��������(UI)�� ȭ�鿡 �߰��Ѵ�.
//
// �˰���:
//   1. �̹� ���� Ŭ������ ������ ������ ��� ��� ����.
//   2. ���� ������(CharacterOverlay) ����.
//   3. CreateWidget()���� �� �������� ���� �� AddToViewport(1).
//   4. ĳ������ CombatComponent���� PushAllHUDFromCombat() ȣ���
//      HUD ��ġ�� ��� ���� (������ ���� �� UI ����).
/////////////////////////////////////////////////////////////
void ATFHUD::AddCharacterOverlay()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !CharacterOverlayClass) return;

	TArray<UUserWidget*> Existing;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC, Existing, CharacterOverlayClass, false);
	for (UUserWidget* W : Existing)
	{
		if (W) W->RemoveFromParent();
	}

	if (CharacterOverlay)
	{
		CharacterOverlay->RemoveFromParent();
		CharacterOverlay = nullptr;
	}

	CharacterOverlay = CreateWidget<UCharacterOverlay>(PC, CharacterOverlayClass);
	if (CharacterOverlay)
	{
		CharacterOverlay->AddToViewport(1);

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

/////////////////////////////////////////////////////////////
// ���:
//   �־��� �ؽ�ó�� ȭ���� Ư�� ��ġ�� ũ�ν����� �׸���.
//
// �˰���:
//   - �ؽ�ó�� �ʺ�/���̸� ������ �߽� ���� ��ǥ ���.
//   - ViewportCenter���� Spread ��ŭ �̵����� ��ġ ����.
//   - DrawTexture()�� ���� ȭ�鿡 ������.
//   - FLinearColor �Ķ���͸� ���� ũ�ν���� ���� ���� ����.
/////////////////////////////////////////////////////////////
void ATFHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();

	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / ViewportDiv) + Spread.X,
		ViewportCenter.Y - (TextureHeight / ViewportDiv) + Spread.Y);

	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight,
		0.f, 0.f, 1.f, 1.f, CrosshairColor);
}
