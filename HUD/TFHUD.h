// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TFHUD.generated.h"

/**
 * FHUDPakage
 *
 * 크로스헤어 관련 정보(텍스처, 색상, 스프레드 등)를 저장하는 구조체.
 */
USTRUCT(BlueprintType)
struct FHUDPakage {
	GENERATED_BODY()
public:
	class UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;
	float CrosshairSpread;
	FLinearColor CrosshairColor;
};

/**
 * ATFHUD
 *
 * - 화면에 HUD 요소를 표시하는 클래스.
 * - 크로스헤어, 캐릭터 상태 오버레이, 알림, 채팅 UI를 관리한다.
 */
UCLASS()
class UNREALPROJECT_7A_API ATFHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override; // HUD를 직접 그리는 함수
	FORCEINLINE void SetHUDPackage(const FHUDPakage& Package) { HUDPackage = Package; }

	// 캐릭터 오버레이 위젯
	UPROPERTY(EditAnywhere, Category = "Player State")
	TSubclassOf<UUserWidget> CharacterOverlayClass;
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;
	void AddCharacterOverlay();

	// 알림 위젯
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> AlertClass;
	UPROPERTY()
	class UAlert* Alert;
	void AddAlert();

	// 채팅 위젯
	UPROPERTY(EditAnywhere, Category = "Chat")
	TSubclassOf<UUserWidget> ChatWidgetClass;
	UPROPERTY()
	class UChatWidget* ChatWidget;
	void AddChatWidget();

protected:
	virtual void BeginPlay() override;

private:
	// HUD 렌더링용 구조체
	FHUDPakage HUDPackage;

	float ViewportDiv = 2.f;
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f; // 크로스헤어 최대 확산 정도

	// 크로스헤어를 화면에 직접 그리는 내부 함수
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);
};
