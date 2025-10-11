// Fill out your copyright notice in the Description page of Project Settings.

#include "Pickup.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"

/////////////////////////////////////////////////////////////
// 기능:
//   APickup의 생성자. 컴포넌트 초기화 및 기본 속성 설정.
//
// 알고리즘:
//   - bReplicates = true : 서버-클라이언트 간 동기화 활성화.
//   - RootComponent, OverlapSphere, PickupMesh, PickupEffectComponent 생성.
//   - OverlapSphere는 Pawn 채널에 대해서만 Overlap 응답.
//   - PickupMesh 충돌 비활성화 (시각적 요소 전용).
//   - PickupEffectComponent는 루트에 부착되어 지속 효과 표시.
/////////////////////////////////////////////////////////////
APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(RootComponent);
	OverlapSphere->SetSphereRadius(150.f);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(OverlapSphere);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectComponent"));
	PickupEffectComponent->SetupAttachment(RootComponent);
}

/////////////////////////////////////////////////////////////
// 기능:
//   게임 시작 시 호출. 일정 시간 후 충돌 바인딩을 활성화.
//
// 알고리즘:
//   - HasAuthority()로 서버에서만 실행.
//   - 일정 시간 후 OnComponentBeginOverlap 델리게이트에 OnSphereOverlap 함수 바인딩.
//   - 이렇게 딜레이를 주는 이유는 스폰 직후 충돌 이벤트가 즉시 발생하는 것을 방지하기 위함.
/////////////////////////////////////////////////////////////
void APickup::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
		GetWorldTimerManager().SetTimer(BindOverlapTimer, this, &APickup::BindOverlapTimerFinished, BindoverlapTime);
}

/////////////////////////////////////////////////////////////
// 기능:
//   서버에서 호출되어 모든 클라이언트에 이펙트를 재생한다.
//
// 알고리즘:
//   - NetMulticast, Reliable로 선언되어 모든 클라이언트가 수신.
//   - UGameplayStatics::PlaySoundAtLocation으로 사운드 재생.
//   - UNiagaraFunctionLibrary::SpawnSystemAtLocation으로 이펙트 생성.
/////////////////////////////////////////////////////////////
void APickup::MulticastPlayPickupEffects_Implementation()
{
	if (PickupSound)
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());

	if (PickupEffect)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PickupEffect, GetActorLocation(), GetActorRotation());
}

/////////////////////////////////////////////////////////////
// 기능:
//   일정 시간 경과 후 충돌 이벤트를 활성화.
//
// 알고리즘:
//   - OnComponentBeginOverlap에 OnSphereOverlap 함수 동적 바인딩.
//   - BindOverlapTimer를 통해 BeginPlay 후 약간의 지연을 둠.
/////////////////////////////////////////////////////////////
void APickup::BindOverlapTimerFinished()
{
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnSphereOverlap);
}

/////////////////////////////////////////////////////////////
// 기능:
//   매 프레임마다 호출되어 아이템을 회전시킨다.
//
// 알고리즘:
//   - DeltaTime에 회전 속도를 곱한 FRotator로 AddWorldRotation() 호출.
//   - 단순 회전 애니메이션을 통해 시각적으로 아이템 존재감을 높임.
/////////////////////////////////////////////////////////////
void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (PickupMesh)
	{
		PickupMesh->AddWorldRotation(FRotator(0.f, BaseTurnRate * DeltaTime, 0.f));
	}
}

/////////////////////////////////////////////////////////////
// 기능:
//   액터가 파괴될 때 호출된다.
//
// 알고리즘:
//   - 기본적으로 Super::Destroyed()만 호출.
//   - 주석 처리된 부분은 사운드/이펙트를 파괴 시점에 재생하도록 예비 코드로 남겨둠.
/////////////////////////////////////////////////////////////
void APickup::Destroyed()
{
	Super::Destroyed();
}

/////////////////////////////////////////////////////////////
// 기능:
//   플레이어가 Overlap 범위에 들어왔을 때 호출된다.
//   아이템 습득 처리 및 이펙트 재생.
//
// 알고리즘:
//   - 서버(HasAuthority())만 실행하여 네트워크 충돌 중복 방지.
//   - 이미 습득된 상태(bPickedUp == true)라면 무시.
//   - MulticastPlayPickupEffects()로 모든 클라이언트에 이펙트 재생.
//   - Destroy()로 액터 파괴하여 필드에서 제거.
/////////////////////////////////////////////////////////////
void APickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || bPickedUp) return;

	bPickedUp = true;
	MulticastPlayPickupEffects();
	Destroy();
}
