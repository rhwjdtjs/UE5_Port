// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
// Sets default values
ACasing::ACasing()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh")); // ���̽� �޽� ����
	SetRootComponent(CasingMesh); // ��Ʈ ������Ʈ�� ����
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); // ī�޶� ä�ο� ���� �浹 ������ ����
	CasingMesh->SetSimulatePhysics(true); // ���� �ùķ��̼� Ȱ��ȭ
	CasingMesh->SetEnableGravity(true); // �߷� Ȱ��ȭ
	CasingMesh->SetNotifyRigidBodyCollision(true); // ��ü �浹 �˸� Ȱ��ȭ
	ShellEjectionImpluse = 3.f; // ���̽� ���޽� �� ����
}

// Called when the game starts or when spawned
void ACasing::BeginPlay()
{
	Super::BeginPlay();
	CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit); // ���̽� �޽��� Hit �̺�Ʈ ���ε�
	CasingMesh->AddImpulse(GetActorForwardVector()*ShellEjectionImpluse); // ���̽� �޽��� ���� �������� ���޽� �߰�
}

void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (ShellEjectionSound) {
		UGameplayStatics::PlaySoundAtLocation(this, ShellEjectionSound, GetActorLocation()); // ���̽� �߻� ���� ���
	}
	SetLifeSpan(0.15f); // ���̽��� �����ֱ⸦ 0.15�ʷ� �����Ͽ� �ڵ����� ���ŵǵ��� ��
}

