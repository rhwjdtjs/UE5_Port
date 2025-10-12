// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "TFGameInstance.generated.h"

class IHttpRequest;



UCLASS()
class UNREALPROJECT_7A_API UTFGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	virtual void Shutdown() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movies")
	FString LogoMovie = TEXT("LogoMovie");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movies")
	FString LoadingVideo = TEXT("LoadingVideo");
	bool bFirstWarmupDone = false;
	// �ΰ� ���� �̹� ����ߴ��� üũ
	bool bLogoPlayed = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firebase")
	FString FirebaseDatabaseUrl = TEXT("PERSONAL SITE");
	/* ���̾�̽� ��� ų���� ���ε� */
	void SendKillToFirebase(const FString& PlayerName, int32 Kills);
private:
	void OnPreLoadMap(const FString& MapName);
	void OnPostLoadMap(class UWorld* LoadedWorld);
	TArray<TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>> PendingHttpRequests;
	void TestHttpGoogle();
};
