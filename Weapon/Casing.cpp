// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
// Sets default values
ACasing::ACasing()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh")); // 케이싱 메쉬 생성
	SetRootComponent(CasingMesh); // 루트 컴포넌트로 설정
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); // 카메라 채널에 대해 충돌 응답을 무시
	CasingMesh->SetSimulatePhysics(true); // 물리 시뮬레이션 활성화
	CasingMesh->SetEnableGravity(true); // 중력 활성화
	CasingMesh->SetNotifyRigidBodyCollision(true); // 강체 충돌 알림 활성화
	ShellEjectionImpluse = 3.f; // 케이싱 임펄스 값 설정
}

// Called when the game starts or when spawned
void ACasing::BeginPlay()
{
	Super::BeginPlay();
	CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit); // 케이싱 메쉬에 Hit 이벤트 바인딩
	CasingMesh->AddImpulse(GetActorForwardVector()*ShellEjectionImpluse); // 케이싱 메쉬에 전방 방향으로 임펄스 추가
}

void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (ShellEjectionSound) {
		UGameplayStatics::PlaySoundAtLocation(this, ShellEjectionSound, GetActorLocation()); // 케이싱 발사 사운드 재생
	}
	SetLifeSpan(0.15f); // 케이싱의 생명주기를 0.15초로 설정하여 자동으로 제거되도록 함
}

