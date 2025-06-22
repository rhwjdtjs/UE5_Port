

#include "CBComponent.h"
#include "UnrealProject_7A/Weapon/Weapon.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h" //복제를 위한 헤더파일을 포함시킨다.
#include "GameFramework/CharacterMovementComponent.h" //캐릭터 이동 컴포넌트를 포함시킨다.
UCBComponent::UCBComponent()
{

	PrimaryComponentTick.bCanEverTick = false;
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

void UCBComponent::ServerSetAiming_Implementation(bool bAiming)
{
	bisAiming = bAiming; //서버에서 조준 여부를 설정한다.
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = bisAiming ? AimingWalkSpeed : baseWalkSpeed; //조준 여부에 따라 캐릭터의 최대 걷는 속도를 설정한다.
	}
}


void UCBComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


}

