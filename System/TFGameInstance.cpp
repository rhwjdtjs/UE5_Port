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

    // �ε��� ������ �ڵ� ���� 
    Attr.bAutoCompleteWhenLoadingCompletes = true;

    // �ݵ�� �츮�� ���� ������ ��� ����
    Attr.bWaitForManualStop = false;

    // ��ŵ ����
    Attr.bMoviesAreSkippable = false;

    // �ּ� ǥ�� �ð� (���� ���̿� �����ָ� ��)
    Attr.MinimumLoadingScreenDisplayTime = 5.0f;

    Attr.bAllowInEarlyStartup = true;

    // Content/Movies/ ������ �ִ� ���� �̸� (Ȯ���� ����)
    Attr.MoviePaths = { "LoadingVideo"};

    // �� ���� ���
    Attr.PlaybackType = EMoviePlaybackType::MT_Normal;

    GetMoviePlayer()->SetupLoadingScreen(Attr);
}

void UTFGameInstance::OnPostLoadMap(class UWorld* LoadedWorld)
{
    
}