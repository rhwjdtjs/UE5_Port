#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UnrealProject_7a/HUD/TFHUD.h"
#include "UnrealProject_7A/Weapon/WeaponTypes.h"
#include "CombatStates.h"
#include "CBComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALPROJECT_7A_API UCBComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCBComponent();
	friend class ATimeFractureCharacter;
	void EquipWeapon(class AWeapon* WeaponEquip); //���⸦ �����ϴ� �Լ�
	void SwapWeapons(); //���⸦ ��ü�ϴ� �Լ�
	void EquippedWeaponPositionModify();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	//�����ϴ� �׸��� �����ϴ� �Լ�
	UFUNCTION(BlueprintCallable)
	void FinishReload(); //������ �Ϸ� �Լ�
	UFUNCTION(BlueprintCallable)
	void PushAllHUDFromCombat();
	UFUNCTION(Server, Reliable)
	void ServerFinishReload();
	void FireButtonPressed(bool bPressed); //�߻� ��ư�� ������ �� ȣ��Ǵ� �Լ�
	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload(); //���� ź�� ������ �Լ�

	void JumpToShotgunEnd();
	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished(); //����ź ��ô �Ϸ� �Լ�
	UFUNCTION(BlueprintCallable)
	void LaunchGrenade(); //����ź �߻� �Լ�
	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);
private:
	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState=ECombatState::ECS_Unoccupied; //���� ����
	UFUNCTION()
	void OnRep_CombatState(); //���� ���°� ����Ǿ��� �� ȣ��Ǵ� �Լ�
	UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon)
	class AWeapon* EquippedWeapon; //������ ����
	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon; //���� ����
	UFUNCTION()
	void OnRep_SecondaryWeapon(); //���� ���Ⱑ ����Ǿ��� �� ȣ��Ǵ� �Լ�
	class ATimeFractureCharacter* Character; //ĳ����
	class ATFPlayerController* Controller; //�÷��̾� ��Ʈ�ѷ�
	class ATFHUD* TFHUD; //HUD
	UPROPERTY(Replicated)
	bool bisAiming; //���� ����
	UPROPERTY(EditAnywhere, Category = "Combat")
	float baseWalkSpeed; //�⺻ �ȴ� �ӵ�
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AimingWalkSpeed; //���� ������ �ȴ� �ӵ�

	bool bFireButtonPressed; //�߻� ��ư�� ���ȴ��� ����
	//��� & ũ�ν����
	FHUDPakage HUDPackage; //HUD ��Ű�� ����ü
	float CrosshairVelocityFactor; //ũ�ν���� �ӵ� ���
	FVector  HitTarget; //��Ʈ Ÿ��
	float CrosshairAimFactor; //ũ�ν���� ���� ���
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AimFactorValue1 = 0.58f; //���� ��� �� 1
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AimFactorValue2 = 30.f; //���� ��� �� 1
	float CrosshairShootingFactor; //ũ�ν���� ��� ���
	//FOV ���̹�
	//�������� �ƴҶ� FOV
	float DefaultFOV; //�⺻ FOV
	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedFOV=30.f; //�ܵ� FOV
	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomInterpSpeed=20.f; //�� ���������̼� �ӵ�
	UPROPERTY(EditAnywhere, Category = "Combat")
	float TargetDistance;
	float CurrentFOV; //���� FOV
	void InterpFOV(float DeltaTime); //FOV�� �����ϴ� �Լ�
	//�ڵ�ȭ��
	FTimerHandle FireTimer; //�߻� Ÿ�̸� �ڵ�
	void StartFireTimer(); //�߻� Ÿ�̸Ӹ� �����ϴ� �Լ�
	void FireTimerFinished(); //�߻� Ÿ�̸Ӱ� ������ �� ȣ��Ǵ� �Լ�
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bCanFire = true; //�߻� ���� ����

	bool CanFire(); //�߻� ���� ���θ� Ȯ���ϴ� �Լ�
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo; //���� ź��
	UFUNCTION()
	void OnRep_CarriedAmmo(); //���� ź���� ����Ǿ��� �� ȣ��Ǵ� �Լ�
	void HandleReload(); //������ ó�� �Լ�
	int32 AmountToReload(); //�������� ��
	void UpdateAmmoValues(); //ź�� ���� ������Ʈ�ϴ� �Լ�
	void UpdateShotgunAmmoValues(); //���� ź�� ���� ������Ʈ�ϴ� �Լ�
	void ShowAttachedGrenade(bool bShowGrenade); //������ ����ź�� ǥ���ϴ� �Լ�
	UPROPERTY(EditAnywhere)
	int32 StartingCarriedAmmo = 30; //���� ���� ź�� ��
	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 0; //���� ���� ź�� ��
	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 60; //���� ���� ź�� ��
	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 100; //���� ���� ź�� ��
	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 14; //���� ���� ź�� ��
	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 10; //���� ���� ź�� ��
	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeAmmo = 10; //���� ���� ź�� ��
	UPROPERTY(ReplicatedUsing=OnRep_Grenades)
	int32 Grenades = 3; //����ź ����
	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 9; //�ִ� ����ź ����

	void UpdateHUDGrenades(); //HUD�� ����ź ������ ������Ʈ�ϴ� �Լ�
	UFUNCTION()
	void OnRep_Grenades(); //����ź ������ ����Ǿ��� �� ȣ��Ǵ� �Լ�
	void InitializeCarriedAmmo(); //���� ź���� �ʱ�ȭ�ϴ� �Լ�
    // TMap�� �𸮾� ������ ���ø� �����̳ʷ�, Ű-�� ���� �����ϴ� �ؽ� ���Դϴ�.
    // TMap<KeyType, ValueType> ���·� ���Ǹ�, Ű�� ���� ���� ������ ������ �� �ֽ��ϴ�.

    // ���ӿ��� TMap�� �ַ� ���Ǵ� ���õ�:
    // 1. �κ��丮 �ý��� - TMap<int32, FItemData> Inventory; (������ ID -> ������ ����)
    // 2. �÷��̾� ���� - TMap<FString, float> PlayerStats; ("Health" -> 100.0f, "Mana" -> 50.0f)
    // 3. ���⺰ ź�� ���� - TMap<EWeaponType, int32> AmmoCount; (���� Ÿ�� -> ź�� ��)
    // 4. ����Ʈ �����Ȳ - TMap<int32, bool> QuestProgress; (����Ʈ ID -> �Ϸ� ����)
    // 5. ���� ���� - TMap<FString, FString> GameSettings; ("Resolution" -> "1920x1080")

    // ���� �ڵ忡���� ��� ����:
	TMap<EWeaponType, int32> CarriedAmmoMap;  // ���� Ÿ�Ժ� ���� ź���� �����ϴ� ��
	UPROPERTY(EditAnywhere)
	int32 MaxCarriedAmmo = 999;
    /*
    ���� Ÿ�Ժ� ���� ź���� �����ϴ� ��
    - Ű(Key): EWeaponType - ������ ���� (��: ����, ����, ���� ��)
    - ��(Value): int32 - �ش� ���⿡ ���� ���� ź�� ��

    ��� ����:
    CarriedAmmoMap.Add(EWeaponType::AssaultRifle, 120);  // ���ݼ��� ź�� 120�� �߰�
    CarriedAmmoMap.Add(EWeaponType::Pistol, 60);         // ���� ź�� 60�� �߰�

    int32 RifleAmmo = CarriedAmmoMap[EWeaponType::AssaultRifle]; // ���ݼ��� ź�� ��ȸ
    CarriedAmmoMap[EWeaponType::Pistol] = 30;            // ���� ź���� 30�߷� ����

    // �ֿ� TMap �Լ���:
    // - Add(Key, Value): ���ο� Ű-�� �� �߰�
    // - Remove(Key): ������ Ű�� �׸� ����
    // - Find(Key): Ű�� �ش��ϴ� ���� ������ ��ȯ (������ nullptr)
    // - Contains(Key): Ű�� �����ϴ��� Ȯ��
    // - Num(): ����� �׸� �� ��ȯ
    // - Empty(): ��� �׸� ����

    ����:
    - O(1) ��� �ð����⵵�� ���� �˻�, ����, ����
    - Ű�� ���� �������� ������ ����
    - ���� ũ�� ����

    ����:
    - �޸� ������� ����
    - ������ ������� ���� (������ �ʿ��ϸ� TSortedMap ���)
    */
protected:
	virtual void BeginPlay() override;
	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);
	UFUNCTION(Server, Reliable)
	void ServerReload(); //�������� ������ ��û�� ó���ϴ� �Լ�
	void SetAiming(bool bAiming); //���� ���¸� �����ϴ� �Լ�
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bAiming); //�������� ���� ���¸� �����ϴ� �Լ�
	UFUNCTION()
	void OnRep_EquippedWeapon(); //������ ���Ⱑ �ٲ� �� ȣ��Ǵ� �Լ�
	
	void DropEquippedWeapon(); //������ ���⸦ ����߸��� �Լ�
	void AttachActorToRightHand(AActor* ActorToAttach); //�����տ� ���͸� �����ϴ� �Լ�
	void AttachActorToLeftHand(AActor* ActorToAttach); //�޼տ� ���͸� �����ϴ� �Լ�
	void AttachActorToBack(AActor* ActorToAttach); //� ���͸� �����ϴ� �Լ�
	void UpdateCarriedAmmo(); //���� ź���� ������Ʈ�ϴ� �Լ�
	void PlayEquipSound(AWeapon* WeaponToEquip); //���� ���带 ����ϴ� �Լ�
	void ReloadEmptyWeapon(); //�� ���� ���ε� �Լ�
	void Fire();
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTargert); //�������� �߻� ��ư�� ���ȴ��� ���θ� �����ϴ� �Լ�
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTargert); //�߻� ��Ƽĳ��Ʈ �Լ�

	void TraceUnderCrosshairs(FHitResult& TraceHitResult); //���ؼ� �Ʒ��� ��ü�� �����ϴ� �Լ�
	void SetHUDCrossharis(float DeltaTime); //HUD�� ũ�ν��� �����ϴ� �Լ�
	void ThrowGrenade(); //����ź ��ô �Լ�
	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade(); //�������� ����ź ��ô ��û�� ó���ϴ� �Լ�
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GrenadeClass; //����ź Ŭ����
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void Reload();
	FORCEINLINE int32	GetGrenades() const { return Grenades; } //����ź ������ ��ȯ�ϴ� �Լ�
	FORCEINLINE bool IsAiming() const { return bisAiming; } //���� ���θ� ��ȯ�ϴ� �Լ�
	FORCEINLINE float SetBaseWalkSpeed(float Speed) { return baseWalkSpeed = Speed; } //�⺻ �ȴ� �ӵ��� �����ϴ� �Լ�
	FORCEINLINE float SetAimWalkSpeed(float Speed) { return AimingWalkSpeed = Speed; } //���� ������ �ȴ� �ӵ��� �����ϴ� �Լ�
	bool ShouldSwapWeapons(); //���⸦ ��ü�� �� �ִ��� ���θ� Ȯ���ϴ� �Լ�
	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount); //ź���� �ݴ� �Լ�
};
