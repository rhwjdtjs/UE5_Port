// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WireComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALPROJECT_7A_API UWireComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWireComponent();

	friend class ATimeFractureCharacter;

	void FireWire();
	void ReleaseWire();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UFUNCTION(BlueprintCallable, Category = "Wire")
	bool IsAttached() const { return bIsAttached; }
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
private:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayWireEffects(const FVector& Start, const FVector& Target);
	UPROPERTY(EditDefaultsOnly, Category = "Wire|Effects")
	class UNiagaraSystem* WireShootEffect;  
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDrawWire(const FVector& Start, const FVector& End);
	UPROPERTY(EditDefaultsOnly, Category = "Wire|Effects")
	class UNiagaraSystem* WireImpactEffect;   // �浹 ��ƼŬ (Niagara)
	UPROPERTY()
	class ATimeFractureCharacter* Character;
	UPROPERTY(ReplicatedUsing = OnRep_WireState)
	FVector WireTarget; //���̾� ������
	UPROPERTY(ReplicatedUsing = OnRep_WireState)
	bool bIsAttached = false; //���̾ �ɷ��ִ���

	UPROPERTY(EditAnywhere, Category="Wire")
	float PullSpeed = 2000.f; //�̵��ӵ�
	UPROPERTY(EditaNYWHERE, Category="Wire")
	float MaxWireDistance = 3000.f; //�ִ� ���̾�Ÿ�(����Ʈ���̽�)

	UPROPERTY(EditAnywhere, Category="Wire")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility; //����Ʈ���̽� �浹ä��
	UFUNCTION(Server, Reliable)
	void ServerFireWire();
	UFUNCTION(Server, Reliable)
	void ServerReleaseWire();

	UFUNCTION()
	void OnRep_WireState();
};
