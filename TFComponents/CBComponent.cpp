

#include "CBComponent.h"
#include "UnrealProject_7A/Weapon/Weapon.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h" //복제를 위한 헤더파일을 포함시킨다.
#include "GameFramework/CharacterMovementComponent.h" //캐릭터 이동 컴포넌트를 포함시킨다.
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
UCBComponent::UCBComponent()
{

	PrimaryComponentTick.bCanEverTick = true;
//	//baseWalkSpeed = 600.f; //기본 걷는 속도를 설정한다.
//	AimingWalkSpeed = 300.f; //조준 상태의 걷는 속도를 설정한다.
}

void UCBComponent::EquipWeapon(AWeapon* WeaponEquip)
{
	if (Character == nullptr || WeaponEquip == nullptr) return;

	EquippedWeapon = WeaponEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	EquippedWeapon->AttachToComponent(
		Character->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		FName("RightHandSocket")
	);

	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->ShowPickupWidget(false);
	EquippedWeapon->GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UCBComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCBComponent, EquippedWeapon); //장착된 무기를 복제한다.
	DOREPLIFETIME(UCBComponent, bisAiming);// 조준 여부를 복제한다.
	//DOREPLIFETIME_CONDITION(UCBComponent, EquippedWeapon, COND_OwnerOnly); //장착된 무기를 복제하는데, 조건은 소유자만 복제한다는 뜻이다.
}


void UCBComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = baseWalkSpeed; //캐릭터의 최대 걷는 속도를 기본 속도로 설정한다.
	}
}

void UCBComponent::SetAiming(bool bAiming)
{
	bisAiming = bAiming; //조준 여부를 설정한다.
	ServerSetAiming(bAiming); //서버에서 조준 여부를 설정한다.
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = bisAiming ? AimingWalkSpeed : baseWalkSpeed; //조준 여부에 따라 캐릭터의 최대 걷는 속도를 설정한다.
	}
}

void UCBComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon) {


	}
}

void UCBComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed; //발사 버튼이 눌렸는지 여부를 설정한다.
	if (bFireButtonPressed) {
		FHitResult HitResult; // 트레이스 결과를 저장할 변수
		TraceUnderCrosshairs(HitResult); // 매 프레임마다 화면 중앙 아래의 물체를 추적한다.
		ServerFire(HitResult.ImpactPoint); //서버에서 발사 버튼이 눌렸는지 여부를 설정한다.
	}
}

void UCBComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTargert)
{
	MulticastFire(TraceHitTargert); //서버에서 발사 멀티캐스트 함수를 호출한다.
}

void UCBComponent::ServerSetAiming_Implementation(bool bAiming)
{
	bisAiming = bAiming; //서버에서 조준 여부를 설정한다.
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = bisAiming ? AimingWalkSpeed : baseWalkSpeed; //조준 여부에 따라 캐릭터의 최대 걷는 속도를 설정한다.
	}
}


void UCBComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTargert)
{
	if (EquippedWeapon == nullptr) return;
	if (Character) {
		Character->PlayFireMontage(bisAiming); //캐릭터의 발사 모션을 재생한다.
		EquippedWeapon->Fire(TraceHitTargert); //무기를 발사한다.
	}
}

void UCBComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize; // 화면 크기를 저장할 변수
	if (GEngine && GEngine->GameViewport) {
		GEngine->GameViewport->GetViewportSize(ViewportSize); // 화면 크기를 가져온다.
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f); // 화면 중앙의 좌표를 계산한다.
	FVector CrosshairWorldPostion; // 화면 중앙의 월드 위치를 저장할 변수
	FVector CrosshairWorldDirection; // 화면 중앙의 월드 방향을 저장할 변수
	bool bScreenToWorld =UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation, CrosshairWorldPostion, CrosshairWorldDirection); // 화면 중앙의 위치와 방향을 계산한다.
	if (bScreenToWorld) {
		FVector Start = CrosshairWorldPostion; // 시작 위치는 화면 중앙의 월드 위치
		FVector End = Start + (CrosshairWorldDirection * 80000.f); // 끝 위치는 시작 위치에서 월드 방향으로 10000 단위 떨어진 위치

		GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility); // 라인 트레이스를 사용하여 화면 중앙 아래의 물체를 추적한다.
	}
}

void UCBComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}

