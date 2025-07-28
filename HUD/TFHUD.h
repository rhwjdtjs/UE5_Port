// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TFHUD.generated.h"
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
 * 
 */
UCLASS()
class UNREALPROJECT_7A_API ATFHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override; //HUD�� �׸��� �Լ�, �ڽ� Ŭ�������� ������ �� �ִ�.
	FORCEINLINE void SetHUDPackage(const FHUDPakage& Package) { HUDPackage = Package; } //HUD ��Ű���� �����ϴ� �Լ�
	UPROPERTY(EditAnywhere, Category="Player State")
	TSubclassOf<UUserWidget> CharacterOverlayClass; //ĳ���� �������� ���� Ŭ����
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay; //ĳ���� �������� ������ �����ϴ� ����
	void AddCharacterOverlay(); //ĳ���� �������� ������ �߰��ϴ� �Լ�
protected:
	virtual void BeginPlay() override; //���� ���� �� ȣ��Ǵ� �Լ�, �ڽ� Ŭ�������� ������ �� �ִ�.
	
	private:
	FHUDPakage HUDPackage; //HUD ��Ű�� ����ü�� ����Ͽ� ũ�ν���� �ؽ��ĸ� ����
	float ViewportDiv = 2.f;
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f; //ũ�ν���� �������� �ִ밪
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);
};
