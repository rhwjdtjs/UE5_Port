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
private:
	class AWeapon* EquippedWeapon; //������ ����
	class ATimeFractureCharacter* Character; //ĳ����
protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
