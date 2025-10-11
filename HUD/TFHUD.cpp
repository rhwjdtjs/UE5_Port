#include "TFHUD.h"
#include "GameFramework/PlayerController.h"
#include "CharacterOverlay.h"
#include "Alert.h"
#include "ChatWidget.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "UnrealProject_7A/TFComponents/CBComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

/////////////////////////////////////////////////////////////
// 기능:
//   HUD를 매 프레임 그리는 함수.
//   크로스헤어 텍스처를 화면 중심에 표시한다.
//
// 알고리즘:
//   - GetViewportSize()로 현재 화면 크기를 얻는다.
//   - 화면 중심 좌표(ViewportCenter)를 계산한다.
//   - CrosshairSpreadMax와 현재 스프레드 값을 곱해 확산 거리 계산.
//   - 각 방향(중앙, 좌, 우, 상, 하)에 대해 DrawCrosshair() 호출.
//   - DrawTexture()를 사용해 UTexture2D를 직접 그린다.
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
// 기능:
//   알림(Alert) 위젯을 추가한다.
//
// 알고리즘:
//   - UWidgetBlueprintLibrary::GetAllWidgetsOfClass()로 기존 Alert 위젯을 모두 제거해 중복 방지.
//   - CreateWidget()으로 새 Alert 위젯 생성 후 AddToViewport(5)로 표시.
//   - 기존 Alert 포인터가 존재하면 정리 후 갱신.
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
// 기능:
//   채팅 위젯을 화면에 표시한다.
//
// 알고리즘:
//   - 플레이어 컨트롤러를 통해 CreateWidget()으로 UChatWidget 생성.
//   - AddToViewport()로 뷰포트에 추가.
//   - 이미 존재하지 않을 경우에만 생성한다.
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
// 기능:
//   캐릭터 오버레이(UI)를 화면에 추가한다.
//
// 알고리즘:
//   1. 이미 동일 클래스의 위젯이 존재할 경우 모두 제거.
//   2. 기존 포인터(CharacterOverlay) 정리.
//   3. CreateWidget()으로 새 오버레이 생성 후 AddToViewport(1).
//   4. 캐릭터의 CombatComponent에서 PushAllHUDFromCombat() 호출로
//      HUD 수치를 즉시 갱신 (리스폰 직후 빈 UI 방지).
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
// 기능:
//   주어진 텍스처를 화면의 특정 위치에 크로스헤어로 그린다.
//
// 알고리즘:
//   - 텍스처의 너비/높이를 가져와 중심 기준 좌표 계산.
//   - ViewportCenter에서 Spread 만큼 이동시켜 위치 보정.
//   - DrawTexture()로 실제 화면에 렌더링.
//   - FLinearColor 파라미터를 통해 크로스헤어 색상 지정 가능.
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
