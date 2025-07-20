#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UnrealProject_7a/HUD/TFHUD.h"
#include "CBComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALPROJECT_7A_API UCBComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCBComponent();
	friend class ATimeFractureCharacter;
	void EquipWeapon(class AWeapon* WeaponEquip); //���⸦ �����ϴ� �Լ�
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	//�����ϴ� �׸��� �����ϴ� �Լ�
private:
	UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon)
	class AWeapon* EquippedWeapon; //������ ����
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
protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bAiming); //���� ���¸� �����ϴ� �Լ�
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bAiming); //�������� ���� ���¸� �����ϴ� �Լ�
	UFUNCTION()
	void OnRep_EquippedWeapon(); //������ ���Ⱑ �ٲ� �� ȣ��Ǵ� �Լ�
	void FireButtonPressed(bool bPressed); //�߻� ��ư�� ������ �� ȣ��Ǵ� �Լ�
	void Fire();
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTargert); //�������� �߻� ��ư�� ���ȴ��� ���θ� �����ϴ� �Լ�
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTargert); //�߻� ��Ƽĳ��Ʈ �Լ�

	void TraceUnderCrosshairs(FHitResult& TraceHitResult); //���ؼ� �Ʒ��� ��ü�� �����ϴ� �Լ�
	void SetHUDCrossharis(float DeltaTime); //HUD�� ũ�ν��� �����ϴ� �Լ�
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
