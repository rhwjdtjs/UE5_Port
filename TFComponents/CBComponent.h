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
	void EquipWeapon(class AWeapon* WeaponEquip); //무기를 장착하는 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	//복제하는 항목을 정의하는 함수
private:
	UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon)
	class AWeapon* EquippedWeapon; //장착된 무기
	class ATimeFractureCharacter* Character; //캐릭터
	UPROPERTY(Replicated)
	bool bisAiming; //조준 여부

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bAiming); //조준 상태를 설정하는 함수
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bAiming); //서버에서 조준 상태를 설정하는 함수
	UFUNCTION()
	void OnRep_EquippedWeapon(); //장착된 무기가 바뀔 때 호출되는 함수

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
