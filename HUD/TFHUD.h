// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TFHUD.generated.h"

/**
 * FHUDPakage
 *
 * ũ�ν���� ���� ����(�ؽ�ó, ����, �������� ��)�� �����ϴ� ����ü.
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
 * - ȭ�鿡 HUD ��Ҹ� ǥ���ϴ� Ŭ����.
 * - ũ�ν����, ĳ���� ���� ��������, �˸�, ä�� UI�� �����Ѵ�.
 */
UCLASS()
class UNREALPROJECT_7A_API ATFHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override; // HUD�� ���� �׸��� �Լ�
	FORCEINLINE void SetHUDPackage(const FHUDPakage& Package) { HUDPackage = Package; }

	// ĳ���� �������� ����
	UPROPERTY(EditAnywhere, Category = "Player State")
	TSubclassOf<UUserWidget> CharacterOverlayClass;
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;
	void AddCharacterOverlay();

	// �˸� ����
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> AlertClass;
	UPROPERTY()
	class UAlert* Alert;
	void AddAlert();

	// ä�� ����
	UPROPERTY(EditAnywhere, Category = "Chat")
	TSubclassOf<UUserWidget> ChatWidgetClass;
	UPROPERTY()
	class UChatWidget* ChatWidget;
	void AddChatWidget();

protected:
	virtual void BeginPlay() override;

private:
	// HUD �������� ����ü
	FHUDPakage HUDPackage;

	float ViewportDiv = 2.f;
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f; // ũ�ν���� �ִ� Ȯ�� ����

	// ũ�ν��� ȭ�鿡 ���� �׸��� ���� �Լ�
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);
};
