// Fill out your copyright notice in the Description page of Project Settings.

#include "TFGameInstance.h"
#include "MoviePlayer.h"
#include "Misc/CoreDelegates.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

void UTFGameInstance::Init()
{
    Super::Init();
#if defined(EHttpSslBackend)
    FHttpModule::Get().SetHttpSslBackend(EHttpSslBackend::WinInet); // �� WinInet�� �ٲ㺸�� �����Ϳ��� �ȵǼ� �̰����� �õ��� ����..
#endif
    FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UTFGameInstance::OnPreLoadMap);
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UTFGameInstance::OnPostLoadMap);
    TestHttpGoogle(); //�̰͵� �����Ϳ����ȵǼ�..
    //�ᱹ�� �������Ϸ� ���� �׽�Ʈ�ϴ� ��� �����̵�
}
void UTFGameInstance::TestHttpGoogle()
{
    auto R = FHttpModule::Get().CreateRequest();
    R->SetURL(TEXT("https://www.google.com"));
    R->SetVerb(TEXT("GET"));
    R->SetTimeout(15.f);
    R->OnProcessRequestComplete().BindLambda([](FHttpRequestPtr, FHttpResponsePtr Resp, bool bOk) {
        UE_LOG(LogTemp, Warning, TEXT("[Firebase :: TEST] google success=%d code=%d"),
            bOk ? 1 : 0, Resp.IsValid() ? Resp->GetResponseCode() : -1);
        });
    R->ProcessRequest();
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
        Attr.MoviePaths = { LogoMovie };
        Attr.MinimumLoadingScreenDisplayTime = 10.0f;
        bLogoPlayed = true;
    }
    else
    {
        Attr.MoviePaths = { LoadingVideo };
        Attr.MinimumLoadingScreenDisplayTime = 5.0f;
    }

    GetMoviePlayer()->SetupLoadingScreen(Attr);
}

void UTFGameInstance::OnPostLoadMap(class UWorld* LoadedWorld)
{
    // �ʿ�� �ε� �� ó��
}
//���̾�̽� �����ͺ��̽��� �� �÷��̾��� ų���� ����
void UTFGameInstance::SendKillToFirebase(const FString& PlayerName, int32 Kills)
{
    // Firebase Database URL�� ��������� �������� ����
    if (FirebaseDatabaseUrl.IsEmpty())
    {
        return;
    }
    // Firebase�� ��ο� Ư�����ڸ� ������� �ʱ� ������ �÷��̾� �̸��� ������ ���ڿ��� ġȯ�Ѵ�.
    FString SafeName = PlayerName;
    SafeName.ReplaceInline(TEXT("."), TEXT("_"));
    SafeName.ReplaceInline(TEXT("#"), TEXT("_"));
    SafeName.ReplaceInline(TEXT("$"), TEXT("_"));
    SafeName.ReplaceInline(TEXT("["), TEXT("_"));
    SafeName.ReplaceInline(TEXT("]"), TEXT("_"));
    SafeName.ReplaceInline(TEXT("/"), TEXT("_"));
    // ���� ���ε��� Firebase ��� ����
    const FString Url = FString::Printf(TEXT("%s/leaderboard/%s.json"), *FirebaseDatabaseUrl, *SafeName);

    //������ JSON ������ ���� HTTP Body�� ���)
    TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
    Root->SetStringField(TEXT("name"), PlayerName);
    Root->SetNumberField(TEXT("kills"), Kills);
    Root->SetNumberField(TEXT("updatedAt"), FPlatformTime::Seconds());
    // JSON ��ü�� ���ڿ��� ����ȭ
    FString Body;
    {
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
        FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);
    }
    //HTTP ��û ��ü ����
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
    Req->SetURL(Url);
    Req->SetVerb(TEXT("PUT"));
    Req->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
    Req->SetContentAsString(Body);
    Req->SetTimeout(30.f);

    //��û ��ü�� ������ �����ϱ� ���� �迭�� ����
     // (���� �Ϸ� �ݹ��� ȣ��� ������ Req�� GC���� �ʵ���)
    PendingHttpRequests.Add(Req);

    // HTTP ��û �Ϸ� �� ����Ǵ� �ݹ� (����/���� ó��)
    Req->OnProcessRequestComplete().BindLambda([this, Req](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            // ��û �Ϸ� �� ���� ������ �迭���� ����
            PendingHttpRequests.Remove(Req);

            if (!bWasSuccessful || !Response.IsValid())
            {
                UE_LOG(LogTemp, Error, TEXT("[Firebase] HTTP failed"));
                return;
            }
            // ���� �ڵ� Ȯ�� (200 ����, 401 ���� ���� ��)
            const int32 Code = Response->GetResponseCode();
        });

    // ��û ���� ���� (true�� ���������� ���۵�)
    const bool bStarted = Req->ProcessRequest();
}
