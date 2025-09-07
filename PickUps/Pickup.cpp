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
	bReplicates = true; //�� ��ü�� ��Ʈ��ũ���� ���� �����ϵ��� ����

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));//��Ʈ ������Ʈ ����

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));//���Ǿ� ������Ʈ ����
	OverlapSphere->SetupAttachment(RootComponent);//��Ʈ ������Ʈ�� ����
	OverlapSphere->SetSphereRadius(150.f);//���Ǿ� ������ ����
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);//�浹 ������ Ȱ��ȭ
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);//��� ä�ο� ���� �浹 ������ ����
	OverlapSphere->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);//Pawn ä�ο� ���� �浹 ������ ��ħ���� ����

	PickupMesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));//����ƽ �޽� ������Ʈ ����
	PickupMesh->SetupAttachment(OverlapSphere);//��Ʈ ������Ʈ�� ����
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);//�浹 ��Ȱ��ȭ
	PickupEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectComponent"));
	PickupEffectComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority()) //���������� ����
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



