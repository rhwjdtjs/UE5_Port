#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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
	UPROPERTY(Replicated)
	bool bisAiming; //���� ����
	UPROPERTY(EditAnywhere, Category = "Combat")
	float baseWalkSpeed; //�⺻ �ȴ� �ӵ�
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AimingWalkSpeed; //���� ������ �ȴ� �ӵ�

	bool bFireButtonPressed; //�߻� ��ư�� ���ȴ��� ����
	FVector HitTarget; //���ؼ� �Ʒ��� ��ü�� ��ġ
protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bAiming); //���� ���¸� �����ϴ� �Լ�
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bAiming); //�������� ���� ���¸� �����ϴ� �Լ�
	UFUNCTION()
	void OnRep_EquippedWeapon(); //������ ���Ⱑ �ٲ� �� ȣ��Ǵ� �Լ�
	void FireButtonPressed(bool bPressed); //�߻� ��ư�� ������ �� ȣ��Ǵ� �Լ�
	UFUNCTION(Server, Reliable)
	void ServerFire(); //�������� �߻� ��ư�� ������ �� ȣ��Ǵ� �Լ�
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(); //�߻� ��Ƽĳ��Ʈ �Լ�

	void TraceUnderCrosshairs(FHitResult& TraceHitResult); //���ؼ� �Ʒ��� ��ü�� �����ϴ� �Լ�
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
