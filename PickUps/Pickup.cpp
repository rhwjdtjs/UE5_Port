// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Net/UnrealNetwork.h"
#include "UnrealProject_7A/UnrealProject_7A.h"
#include "NiagaraComponent.h"
#include "NiagarafunctionLibrary.h"
// Sets default values
APickup::APickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true; //이 객체가 네트워크에서 복제 가능하도록 설정

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));//루트 컴포넌트 생성

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));//스피어 컴포넌트 생성
	OverlapSphere->SetupAttachment(RootComponent);//루트 컴포넌트에 부착
	OverlapSphere->SetSphereRadius(150.f);//스피어 반지름 설정
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);//충돌 쿼리만 활성화
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);//모든 채널에 대해 충돌 응답을 무시
	OverlapSphere->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);//Pawn 채널에 대해 충돌 응답을 겹침으로 설정

	PickupMesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));//스태틱 메시 컴포넌트 생성
	PickupMesh->SetupAttachment(OverlapSphere);//루트 컴포넌트에 부착
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);//충돌 비활성화
	PickupEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectComponent"));
	PickupEffectComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority()) //서버에서만 실행
		GetWorldTimerManager().SetTimer(BindOverlapTimer, this, &APickup::BindOverlapTimerFinished, BindoverlapTime);
}
void APickup::BindOverlapTimerFinished()
{
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnSphereOverlap);
}
// Called every frame
void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (PickupMesh) {
		PickupMesh->AddWorldRotation(FRotator(0.f, BaseTurnRate * DeltaTime, 0.f));
	}
}

void APickup::Destroyed()
{
	Super::Destroyed();
	if (PickupSound) {
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
	}
	if (PickupEffect) {
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			PickupEffect,
			GetActorLocation(),
			GetActorRotation()
		);
	}
}

void APickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}



