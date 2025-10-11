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
// ���:
//   APickup�� ������. ������Ʈ �ʱ�ȭ �� �⺻ �Ӽ� ����.
//
// �˰���:
//   - bReplicates = true : ����-Ŭ���̾�Ʈ �� ����ȭ Ȱ��ȭ.
//   - RootComponent, OverlapSphere, PickupMesh, PickupEffectComponent ����.
//   - OverlapSphere�� Pawn ä�ο� ���ؼ��� Overlap ����.
//   - PickupMesh �浹 ��Ȱ��ȭ (�ð��� ��� ����).
//   - PickupEffectComponent�� ��Ʈ�� �����Ǿ� ���� ȿ�� ǥ��.
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
// ���:
//   ���� ���� �� ȣ��. ���� �ð� �� �浹 ���ε��� Ȱ��ȭ.
//
// �˰���:
//   - HasAuthority()�� ���������� ����.
//   - ���� �ð� �� OnComponentBeginOverlap ��������Ʈ�� OnSphereOverlap �Լ� ���ε�.
//   - �̷��� �����̸� �ִ� ������ ���� ���� �浹 �̺�Ʈ�� ��� �߻��ϴ� ���� �����ϱ� ����.
/////////////////////////////////////////////////////////////
void APickup::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
		GetWorldTimerManager().SetTimer(BindOverlapTimer, this, &APickup::BindOverlapTimerFinished, BindoverlapTime);
}

/////////////////////////////////////////////////////////////
// ���:
//   �������� ȣ��Ǿ� ��� Ŭ���̾�Ʈ�� ����Ʈ�� ����Ѵ�.
//
// �˰���:
//   - NetMulticast, Reliable�� ����Ǿ� ��� Ŭ���̾�Ʈ�� ����.
//   - UGameplayStatics::PlaySoundAtLocation���� ���� ���.
//   - UNiagaraFunctionLibrary::SpawnSystemAtLocation���� ����Ʈ ����.
/////////////////////////////////////////////////////////////
void APickup::MulticastPlayPickupEffects_Implementation()
{
	if (PickupSound)
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());

	if (PickupEffect)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PickupEffect, GetActorLocation(), GetActorRotation());
}

/////////////////////////////////////////////////////////////
// ���:
//   ���� �ð� ��� �� �浹 �̺�Ʈ�� Ȱ��ȭ.
//
// �˰���:
//   - OnComponentBeginOverlap�� OnSphereOverlap �Լ� ���� ���ε�.
//   - BindOverlapTimer�� ���� BeginPlay �� �ణ�� ������ ��.
/////////////////////////////////////////////////////////////
void APickup::BindOverlapTimerFinished()
{
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnSphereOverlap);
}

/////////////////////////////////////////////////////////////
// ���:
//   �� �����Ӹ��� ȣ��Ǿ� �������� ȸ����Ų��.
//
// �˰���:
//   - DeltaTime�� ȸ�� �ӵ��� ���� FRotator�� AddWorldRotation() ȣ��.
//   - �ܼ� ȸ�� �ִϸ��̼��� ���� �ð������� ������ ���簨�� ����.
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
// ���:
//   ���Ͱ� �ı��� �� ȣ��ȴ�.
//
// �˰���:
//   - �⺻������ Super::Destroyed()�� ȣ��.
//   - �ּ� ó���� �κ��� ����/����Ʈ�� �ı� ������ ����ϵ��� ���� �ڵ�� ���ܵ�.
/////////////////////////////////////////////////////////////
void APickup::Destroyed()
{
	Super::Destroyed();
}

/////////////////////////////////////////////////////////////
// ���:
//   �÷��̾ Overlap ������ ������ �� ȣ��ȴ�.
//   ������ ���� ó�� �� ����Ʈ ���.
//
// �˰���:
//   - ����(HasAuthority())�� �����Ͽ� ��Ʈ��ũ �浹 �ߺ� ����.
//   - �̹� ����� ����(bPickedUp == true)��� ����.
//   - MulticastPlayPickupEffects()�� ��� Ŭ���̾�Ʈ�� ����Ʈ ���.
//   - Destroy()�� ���� �ı��Ͽ� �ʵ忡�� ����.
/////////////////////////////////////////////////////////////
void APickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || bPickedUp) return;

	bPickedUp = true;
	MulticastPlayPickupEffects();
	Destroy();
}
