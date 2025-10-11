// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "Net/UnrealNetwork.h"//네트워크 관련 헤더 파일 포함
#include "Animation/AnimationAsset.h"//애니메이션 자산 관련 헤더 파일 포함
#include "Components/SkeletalMeshComponent.h"
#include "Casing.h"//케이싱 클래스 포함
#include "Engine/SkeletalMeshSocket.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"//플레이어 컨트롤러 클래스 포함
#include "UnrealProject_7A/TFComponents/CBComponent.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;//이 객체가 네트워크에서 복제 가능하도록 설정
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));//스켈레탈 메시 컴포넌트 생성
	WeaponMesh->SetupAttachment(RootComponent);//루트 컴포넌트에 부착
	SetRootComponent(WeaponMesh);//루트 컴포넌트 설정

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);//모든 채널에 대해 충돌 응답을 차단
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);//Pawn 채널에 대해 충돌 응답을 무시
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);//충돌 비활성화

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_A);//사용자 정의 깊도 스텐실 값 설정
	WeaponMesh->MarkRenderStateDirty();//렌더 상태를 더럽혀서 업데이트 필요
	EnableCustomDepth(true);//사용자 정의 깊도 활성화
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));//스피어 컴포넌트 생성
	AreaSphere->SetupAttachment(RootComponent);//루트 컴포넌트에 부착
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);//모든 채널에 대해 충돌 응답을 무시
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//충돌 비활성화

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));//위젯 컴포넌트 생성
	PickupWidget->SetupAttachment(RootComponent);//루트 컴포넌트에 부착
}
void AWeapon::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh) {
		WeaponMesh->SetRenderCustomDepth(bEnable);//무기 메시의 사용자 정의 깊도를 설정
	}
}
void AWeapon::ShowPickupWidget(bool bShowPickupWidget)
{
	if (PickupWidget) {
		PickupWidget->SetVisibility(bShowPickupWidget);//위젯의 가시성 설정
	}
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AWeapon, WeaonState);//무기 상태를 복제
	DOREPLIFETIME(AWeapon, Ammo);//탄약을 복제
}

void AWeapon::SetHUDAmmo()
{
	TFOwnerCharacter = TFOwnerCharacter == nullptr ? Cast<ATimeFractureCharacter>(GetOwner()) : TFOwnerCharacter; //소유자가 캐릭터인지 확인
	if (TFOwnerCharacter) {
		TFOwnerController = TFOwnerController == nullptr ? Cast<ATFPlayerController>(TFOwnerCharacter->Controller) : TFOwnerController; //소유자의 컨트롤러가 플레이어 컨트롤러인지 확인
		if (TFOwnerController) {
			TFOwnerController->SetHUDWeaponAmmo(Ammo); //HUD에 남은 탄약을 업데이트
		}
	}
}

void AWeapon::SpendRound() {
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity); //탄약 감소 및 범위 제한
	SetHUDAmmo();
}
void AWeapon::OnRep_Ammo() {
	TFOwnerCharacter = TFOwnerCharacter == nullptr ? Cast<ATimeFractureCharacter>(GetOwner()) : TFOwnerCharacter; //소유자가 캐릭터인지 확인
	if(TFOwnerCharacter && TFOwnerCharacter->GetCombatComponent() && IsFull()) {
		TFOwnerCharacter->GetCombatComponent()->JumpToShotgunEnd(); //샷건의 경우 애니메이션을 끝으로 이동
	}
	SetHUDAmmo();
}
void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (Owner == nullptr) {
		TFOwnerCharacter = nullptr; //소유자 캐릭터를 nullptr로 설정
		TFOwnerController = nullptr;//소유자 캐릭터와 플레이어 컨트롤러를 nullptr로 설정
	}
	else {
		TFOwnerCharacter = TFOwnerCharacter == nullptr ? Cast<ATimeFractureCharacter>(Owner) : TFOwnerCharacter; //소유자가 캐릭터인지 확인
		if (TFOwnerCharacter && TFOwnerCharacter->GetEquippedWeapon() && TFOwnerCharacter->GetEquippedWeapon() == this) {
			SetHUDAmmo();//소유자가 변경되면 HUD의 탄약을 업데이트
		}
	}
	
}
bool AWeapon::IsEmpty()
{
	return Ammo <= 0; //탄약이 0 이하인 경우 true 반환
}
bool AWeapon::IsFull()
{
	return Ammo == MagCapacity; //탄약이 탄창 용량과 같은 경우 true 반환
}
// ============================================================
// [발사 처리] Fire()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 발사 애니메이션 실행, 탄피 생성, 탄약 차감.
// 알고리즘 설명 : 
//   1. FireAnimation 재생
//   2. CasingClass를 AmmoEject 소켓 위치에서 Spawn
//   3. SpendRound() 호출하여 탄약 감소
// ============================================================
void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation) {
		WeaponMesh->PlayAnimation(FireAnimation, false);//발사 애니메이션 재생
	}
	if (CasingClass) {
		const USkeletalMeshSocket* AmmoEjectSockt = WeaponMesh->GetSocketByName(FName("AmmoEject")); //탄피 배출 소켓을 가져옴
		if (AmmoEjectSockt) {
			FTransform SocketTransform = AmmoEjectSockt->GetSocketTransform(GetWeaponMesh()); //소켓의 변환 정보를 가져옴
				UWorld* World = GetWorld();
				if (World) {
					World->SpawnActor<ACasing>(CasingClass, SocketTransform.GetLocation(),
						SocketTransform.GetRotation().Rotator()); //발사체를 생성
			}
		}
	}
	SpendRound();//탄약 감소
}



void AWeapon::DropWeapon()
{
	SetWeaponState(EWeaponState::EWS_Dropped);//무기 상태를 떨어뜨린 상태로 설정
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);//무기의 소유자를 해제
	TFOwnerCharacter = nullptr; //소유자 캐릭터를 nullptr로 설정
	TFOwnerController = nullptr;//소유자 캐릭터와 플레이어 컨트롤러를 nullptr로 설정
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo - AmmoToAdd, 0, MagCapacity); //탄약을 추가하고 범위를 제한
	SetHUDAmmo(); //HUD의 탄약 업데이트
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	if (PickupWidget) {//위젯이 존재하는 경우
		PickupWidget->SetVisibility(false);//위젯 숨기기
	}
	if (HasAuthority()) {//서버에서만 실행
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);//충돌 활성화
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);//Pawn 채널에 대해 충돌 응답을 겹침으로 설정
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);//스피어 겹침 이벤트에 함수 바인딩
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnShpereEndOverlap);//스피어 겹침 종료 이벤트에 함수 바인딩
	}
}



void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	ATimeFractureCharacter* Character = Cast<ATimeFractureCharacter>(OtherActor);//겹친 액터가 캐릭터인지 확인
	if (Character) {
		Character->SetOverlappingWeapon(this);//캐릭터에 겹치는 무기 설정
		
	}
}

void AWeapon::OnShpereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ATimeFractureCharacter* Character = Cast<ATimeFractureCharacter>(OtherActor);//겹친 액터가 캐릭터인지 확인
	if (Character) {
		Character->SetOverlappingWeapon(nullptr);//캐릭터에 겹치는 무기 해제
	}
}



void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaonState = State;//무기 상태 설정
	OnWeaponStateSet();//무기 상태에 따른 설정 적용
	
}
void AWeapon::OnWeaponStateSet()
{
	switch (WeaonState)
	{
	case EWeaponState::EWS_Equipped:
		OnEquipped();//무기 장착 시 설정 적용
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary();//무기 보조 장착 시 설정 적용
		break;
	case EWeaponState::EWS_Dropped:
		OnDropped();//무기 떨어뜨림 시 설정 적용
		break;
	}
}
void AWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();
}
void AWeapon::OnEquipped()
{
	ShowPickupWidget(false);//무기 위젯 숨기기
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//스피어 충돌 비활성화
	WeaponMesh->SetSimulatePhysics(false);//무기 메시 물리 시뮬레이션 활성화
	WeaponMesh->SetEnableGravity(false);//무기 메시 중력 활성화
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);//무기 메시 충돌 활성화
	EnableCustomDepth(false); //장착된 무기의 커스텀 깊이를 비활성화한다.
}
void AWeapon::OnDropped()
{
	if (HasAuthority()) {
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);//스피어 충돌 활성화
	}
	WeaponMesh->SetSimulatePhysics(true);//무기 메시 물리 시뮬레이션 활성화
	WeaponMesh->SetEnableGravity(true);//무기 메시 중력 활성화
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);//무기 메시 충돌 활성화
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_A);//사용자 정의 깊도 스텐실 값 설정
	WeaponMesh->MarkRenderStateDirty();//렌더 상태를 더럽혀서 업데이트 필요
	EnableCustomDepth(true);//사용자 정의 깊도 활성화
}

void AWeapon::OnEquippedSecondary()
{
	ShowPickupWidget(false);//무기 위젯 숨기기
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//스피어 충돌 비활성화
	WeaponMesh->SetSimulatePhysics(false);//무기 메시 물리 시뮬레이션 활성화
	WeaponMesh->SetEnableGravity(false);//무기 메시 중력 활성화
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);//무기 메시 충돌 활성화
	EnableCustomDepth(true); //장착된 무기의 커스텀 깊이를 비활성화한다.
	if (WeaponMesh)
	{
		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_C);//사용자 정의 깊도 스텐실 값 설정
		WeaponMesh->MarkRenderStateDirty();//렌더 상태를 더럽혀서 업데이트 필요
	}
}


