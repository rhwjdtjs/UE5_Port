

#include "CBComponent.h"
#include "UnrealProject_7A/Weapon/Weapon.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h" //������ ���� ��������� ���Խ�Ų��.
UCBComponent::UCBComponent()
{

	PrimaryComponentTick.bCanEverTick = false;

}

void UCBComponent::EquipWeapon(AWeapon* WeaponEquip)
{
	if (Character == nullptr || WeaponEquip==nullptr) return; //ĳ���Ͱ� ������ ����
	EquippedWeapon = WeaponEquip; //������ ���⸦ �����Ѵ�.
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped); //������ ���� ���¸� �����Ѵ�.
	const USkeletalMeshSocket* HandSocket=Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));//ĳ������ ������ ������ �����´�.
	if (HandSocket) //������ �����ϸ�
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh()); //���Ͽ� ���⸦ �����Ѵ�.
	}
	EquippedWeapon->SetOwner(Character); //������ �����ڸ� ĳ���ͷ� �����Ѵ�.
	EquippedWeapon->ShowPickupWidget(false); //���� ������ �����.
	EquippedWeapon->GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision); //���� ���Ǿ��� �浹�� ��Ȱ��ȭ�Ѵ�.
}

void UCBComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCBComponent, EquippedWeapon); //������ ���⸦ �����Ѵ�.
	DOREPLIFETIME(UCBComponent, bisAiming);// ���� ���θ� �����Ѵ�.
	//DOREPLIFETIME_CONDITION(UCBComponent, EquippedWeapon, COND_OwnerOnly); //������ ���⸦ �����ϴµ�, ������ �����ڸ� �����Ѵٴ� ���̴�.
}


void UCBComponent::BeginPlay()
{
	Super::BeginPlay();


}

void UCBComponent::SetAiming(bool bAiming)
{
	bisAiming = bAiming; //���� ���θ� �����Ѵ�.
	ServerSetAiming(bAiming); //�������� ���� ���θ� �����Ѵ�.
}

void UCBComponent::ServerSetAiming_Implementation(bool bAiming)
{
	bisAiming = bAiming; //�������� ���� ���θ� �����Ѵ�.
}


void UCBComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


}

