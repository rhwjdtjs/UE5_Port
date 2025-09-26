// Fill out your copyright notice in the Description page of Project Settings.


#include "TFGameInstance.h"
#include "MoviePlayer.h"                
#include "Misc/CoreDelegates.h"         
void UTFGameInstance::Init()
{
	Super::Init();

	// �� �ε� ����/���� �̺�Ʈ ���ε�
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UTFGameInstance::OnPreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UTFGameInstance::OnPostLoadMap);
}

void UTFGameInstance::Shutdown()
{
	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

	Super::Shutdown();
}

void UTFGameInstance::OnPreLoadMap(const FString& MapName)
{
    FLoadingScreenAttributes Attr;

    Attr.bAutoCompleteWhenLoadingCompletes = true;
    Attr.bWaitForManualStop = false;
    Attr.bMoviesAreSkippable = false;
    Attr.bAllowInEarlyStartup = true;
    Attr.PlaybackType = EMoviePlaybackType::MT_Normal;

    if (!bLogoPlayed)
    {
        // ó�� ���� �� �ΰ� ����
        Attr.MoviePaths = { LogoMovie };
        Attr.MinimumLoadingScreenDisplayTime = 10.0f;
        bLogoPlayed = true; // �������� �ΰ� �� ������
    }
    else
    {
        // ���� �ε� ����
        Attr.MoviePaths = { LoadingVideo };
        Attr.MinimumLoadingScreenDisplayTime = 5.0f;
    }

    GetMoviePlayer()->SetupLoadingScreen(Attr);
}

void UTFGameInstance::OnPostLoadMap(class UWorld* LoadedWorld)
{
    
}