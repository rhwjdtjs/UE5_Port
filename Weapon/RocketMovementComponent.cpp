// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketMovementComponent.h"

URocketMovementComponent::EHandleBlockingHitResult URocketMovementComponent::HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining); // 부모 클래스의 HandleBlockingHit를 호출하여 기본 동작을 수행한다.
	return EHandleBlockingHitResult::AdvanceNextSubstep;// 로켓은 충돌상자에 부딪혔을때 폭파하지 않고 다음 서브스텝으로 넘어간다.
}

void URocketMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	//로켓은 충돌상자에 부딪혔을때만 폭파한다.
}
