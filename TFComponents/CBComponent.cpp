

#include "CBComponent.h"
#include "UnrealProject_7A/Weapon/Weapon.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h" //복제를 위한 헤더파일을 포함시킨다.
UCBComponent::UCBComponent()
{

	PrimaryComponentTick.bCanEverTick = false;

}

void UCBComponent::EquipWeapon(AWeapon* WeaponEquip)
{
	if (Character == nullptr || WeaponEquip==nullptr) return; //캐릭터가 없으면 리턴
	EquippedWeapon = WeaponEquip; //장착된 무기를 설정한다.
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped); //장착된 무기 상태를 설정한다.
	const USkeletalMeshSocket* HandSocket=Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));//캐릭터의 오른손 소켓을 가져온다.
	if (HandSocket) //소켓이 존재하면
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh()); //소켓에 무기를 장착한다.
	}
	EquippedWeapon->SetOwner(Character); //무기의 소유자를 캐릭터로 설정한다.
	EquippedWeapon->ShowPickupWidget(false); //무기 위젯을 숨긴다.
	EquippedWeapon->GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision); //무기 스피어의 충돌을 비활성화한다.
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


}

void UCBComponent::SetAiming(bool bAiming)
{
	bisAiming = bAiming; //조준 여부를 설정한다.
	ServerSetAiming(bAiming); //서버에서 조준 여부를 설정한다.
}

void UCBComponent::ServerSetAiming_Implementation(bool bAiming)
{
	bisAiming = bAiming; //서버에서 조준 여부를 설정한다.
}


void UCBComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


}

