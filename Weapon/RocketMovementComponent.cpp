// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketMovementComponent.h"

URocketMovementComponent::EHandleBlockingHitResult URocketMovementComponent::HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining); // �θ� Ŭ������ HandleBlockingHit�� ȣ���Ͽ� �⺻ ������ �����Ѵ�.
	return EHandleBlockingHitResult::AdvanceNextSubstep;// ������ �浹���ڿ� �ε������� �������� �ʰ� ���� ���꽺������ �Ѿ��.
}

void URocketMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	//������ �浹���ڿ� �ε��������� �����Ѵ�.
}
