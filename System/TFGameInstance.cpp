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
    FHttpModule::Get().SetHttpSslBackend(EHttpSslBackend::WinInet); // ← WinInet로 바꿔보기 에디터에서 안되서 이것저것 시도한 흔적..
#endif
    FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UTFGameInstance::OnPreLoadMap);
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UTFGameInstance::OnPostLoadMap);
    TestHttpGoogle(); //이것도 에디터에서안되서..
    //결국에 실행파일로 만들어서 테스트하니 디비에 저장이됨
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
    // 필요시 로딩 후 처리
}
//파이어베이스 데이터베이스에 각 플레이어의 킬수를 보냄
void UTFGameInstance::SendKillToFirebase(const FString& PlayerName, int32 Kills)
{
    // Firebase Database URL이 비어있으면 실행하지 않음
    if (FirebaseDatabaseUrl.IsEmpty())
    {
        return;
    }
    // Firebase는 경로에 특수문자를 허용하지 않기 때문에 플레이어 이름을 안전한 문자열로 치환한다.
    FString SafeName = PlayerName;
    SafeName.ReplaceInline(TEXT("."), TEXT("_"));
    SafeName.ReplaceInline(TEXT("#"), TEXT("_"));
    SafeName.ReplaceInline(TEXT("$"), TEXT("_"));
    SafeName.ReplaceInline(TEXT("["), TEXT("_"));
    SafeName.ReplaceInline(TEXT("]"), TEXT("_"));
    SafeName.ReplaceInline(TEXT("/"), TEXT("_"));
    // 최종 업로드할 Firebase 경로 생성
    const FString Url = FString::Printf(TEXT("%s/leaderboard/%s.json"), *FirebaseDatabaseUrl, *SafeName);

    //전송할 JSON 데이터 생성 HTTP Body에 담김)
    TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
    Root->SetStringField(TEXT("name"), PlayerName);
    Root->SetNumberField(TEXT("kills"), Kills);
    Root->SetNumberField(TEXT("updatedAt"), FPlatformTime::Seconds());
    // JSON 객체를 문자열로 직렬화
    FString Body;
    {
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
        FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);
    }
    //HTTP 요청 객체 생성
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
    Req->SetURL(Url);
    Req->SetVerb(TEXT("PUT"));
    Req->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
    Req->SetContentAsString(Body);
    Req->SetTimeout(30.f);

    //요청 객체의 수명을 유지하기 위해 배열에 보관
     // (람다 완료 콜백이 호출될 때까지 Req가 GC되지 않도록)
    PendingHttpRequests.Add(Req);

    // HTTP 요청 완료 시 실행되는 콜백 (성공/실패 처리)
    Req->OnProcessRequestComplete().BindLambda([this, Req](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            // 요청 완료 후 수명 관리용 배열에서 제거
            PendingHttpRequests.Remove(Req);

            if (!bWasSuccessful || !Response.IsValid())
            {
                UE_LOG(LogTemp, Error, TEXT("[Firebase] HTTP failed"));
                return;
            }
            // 응답 코드 확인 (200 성공, 401 인증 오류 등)
            const int32 Code = Response->GetResponseCode();
        });

    // 요청 전송 시작 (true면 성공적으로 시작됨)
    const bool bStarted = Req->ProcessRequest();
}
