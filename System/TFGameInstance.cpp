// Fill out your copyright notice in the Description page of Project Settings.


#include "TFGameInstance.h"
#include "MoviePlayer.h"                
#include "Misc/CoreDelegates.h"         
void UTFGameInstance::Init()
{
	Super::Init();

	// 맵 로딩 직전/직후 이벤트 바인딩
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

    // 로딩이 끝나도 자동 종료 
    Attr.bAutoCompleteWhenLoadingCompletes = true;

    // 반드시 우리가 끄기 전까진 재생 유지
    Attr.bWaitForManualStop = false;

    // 스킵 금지
    Attr.bMoviesAreSkippable = false;

    // 최소 표시 시간 (영상 길이와 맞춰주면 됨)
    Attr.MinimumLoadingScreenDisplayTime = 5.0f;

    Attr.bAllowInEarlyStartup = true;

    // Content/Movies/ 폴더에 있는 파일 이름 (확장자 제거)
    Attr.MoviePaths = { "LoadingVideo"};

    // 한 번만 재생
    Attr.PlaybackType = EMoviePlaybackType::MT_Normal;

    GetMoviePlayer()->SetupLoadingScreen(Attr);
}

void UTFGameInstance::OnPostLoadMap(class UWorld* LoadedWorld)
{
    
}