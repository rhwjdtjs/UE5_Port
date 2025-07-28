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
	virtual void DrawHUD() override; //HUD를 그리는 함수, 자식 클래스에서 구현할 수 있다.
	FORCEINLINE void SetHUDPackage(const FHUDPakage& Package) { HUDPackage = Package; } //HUD 패키지를 설정하는 함수
	UPROPERTY(EditAnywhere, Category="Player State")
	TSubclassOf<UUserWidget> CharacterOverlayClass; //캐릭터 오버레이 위젯 클래스
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay; //캐릭터 오버레이 위젯을 저장하는 변수
	void AddCharacterOverlay(); //캐릭터 오버레이 위젯을 추가하는 함수
protected:
	virtual void BeginPlay() override; //게임 시작 시 호출되는 함수, 자식 클래스에서 구현할 수 있다.
	
	private:
	FHUDPakage HUDPackage; //HUD 패키지 구조체를 사용하여 크로스헤어 텍스쳐를 저장
	float ViewportDiv = 2.f;
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f; //크로스헤어 스프레드 최대값
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);
};
