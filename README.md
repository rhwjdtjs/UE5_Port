# VeloCore (UE5 Hyper TPS Multiplayer)
> Unreal Engine 5 기반 하이퍼 TPS 멀티플레이 게임 (Listen Server + Steam OSS)

**Awards / Publications**
- 🏆 순천향대학교 2025 SCHU AI·SW Festival ‘SW프로젝트 경진대회’ 본선 입선 및 우수상 (작품: SW-01)  
- 📄 논문 “UE5 리슨서버에서의 네트워크 동기화” — 한국데이터사이언스학회 동계종합학술대회(12.18~19) 심사 통과

**Quick Links**
- 🎮 10인 플레이 시연 영상(온라인 Steam Listen Server): https://www.youtube.com/watch?v=35-OI47LQC0&t=1s  
- 🧩 기능 설명 영상: https://www.youtube.com/watch?v=pTvCdJSy_EI&t=1s  
- 📘 Technical Doc(아래 접기 섹션): README 하단

---

## Overview
VeloCore는 캐릭터 루트(`ATimeFractureCharacter`)에 전투 / 버프 / 와이어를 **컴포넌트 단위로 분리**하여 부착하는 구조로 설계했습니다.  
멀티플레이 환경에서 “입력은 클라이언트, 판정은 서버” 원칙을 유지하며, 연출은 멀티캐스트/복제로 동기화합니다.

**Highlights**
- **Listen Server + Steam Online Subsystem** 기반 세션 생성/탐색/조인
- **Component-driven Gameplay**: Combat / Buff / Wire 모듈 분리
- **Firebase 연동**: 닉네임/킬 수 등 매치 데이터 업로드(리더보드)

---

## Key Features
- **Combat**
  - 서버 판정 기반 사격/피격 처리(`ServerFire → MulticastFire`), 무기 교체/재장전/수류탄
- **Wire System**
  - 서버 라인트레이스 판정 후 성공/실패 분기(Client RPC) 및 전원 연출 동기화(Multicast)
- **Buff / Pickup**
  - 체력/실드/이속/점프 버프 및 탄약 픽업, HUD 동기화
- **HUD / UX**
  - 체력/실드/탄약/수류탄/킬로그/타이머/크로스헤어 등 핵심 수치 OnRep 기반 갱신

---

## Tech Stack
- **Engine**: Unreal Engine 5.5.4  
- **Language**: C++ (핵심 로직) + Blueprint (UI/에셋 연결)  
- **Network**: Listen Server, Steam OSS, Replication(OnRep), Server/Client/Multicast RPC  
- **External**: Firebase Realtime Database

---

## Media
### Gameplay (10 Players)
[![10-player demo](https://github.com/user-attachments/assets/ab280274-c78b-4092-a872-b7bf37850517)](https://www.youtube.com/watch?v=35-OI47LQC0&t=1s)

### Feature Walkthrough
[![feature demo](https://github.com/user-attachments/assets/cead5c27-3767-4ef2-8184-5710b4c96c88)](https://www.youtube.com/watch?v=pTvCdJSy_EI&t=1s)

---

<details>
  <summary><b>🏅 Awards / Poster / Certificates (펼치기)</b></summary>

- (클래스 계층도 / 포스터 / 학회 심사통과 / 상장 이미지)  

</details>

---

<details>
  <summary><b>📘 Technical Documentation (VeloCore 기술서) (펼치기)</b></summary>

## TOC
1. 게임 기획  
2. 시스템 구조  
3. 개발 기술  
4. 구현 상세  
6. 결과  

---

## 1. 게임 기획 (Game Design)  
  
### 1.1 핵심 시스템 요약
VeloCore는 기능을 **컴포넌트 단위로 분리**하여 캐릭터에 부착하는 구조로 구성했습니다. 즉, 캐릭터(ATimeFractureCharacter)는 “루트”로서 동작하고, **전투(UCBComponent)**, **버프(UBuffComponent)**, **와이어(UWireComponent)** 모듈이 독립적으로 작동하면서도 서버-클라이언트 통신 규칙(RPC/복제)에 맞춰 유기적으로 연동됩니다.   
#### 1.1.1 전투 시스템 (Combat)  
- 무기 장착/교체: `EquipWeapon`, `SwapWeapons`  
- 사격: `ServerFire -> MulticastFire` 패턴으로 서버 판정 후 연출 전파  
- 재장전: `ServerReload`  
- 수류탄: `ServerThrowGrenade`    
탄약은 무기 타입별로 `TMap` 기반 관리로 설계했습니다.  
#### 1.1.2 버프 시스템 (Buff)  
체력/실드 회복, 이동 속도 증가, 점프력 증가 등을 제공하며, 일정 시간 이후 버프 스폰이 재개됩니다. 버프 효과는 멀티캐스트를 통해 전 플레이어 HUD/상태에 반영되도록 구성했습니다.  
#### 1.1.3 와이어 시스템 (Wire)  
`Q` 입력으로 와이어를 발사하고(`ServerFireWire`), StaticMesh 표면에 성공적으로 부착되면 이동을 시작합니다. 실패 시 `ClientWireFail`로 해당 클라이언트에만 피드백을 주고, 성공 시 `MulticastWireSuccess`로 전원에게 연출/상태 변화를 전파합니다.  
이동 중에는 디버그 라인(또는 이펙트)로 와이어 이펙트를 모두에게 보여주도록 구성했습니다.  
#### 1.1.4 HUD / UI  
HUD는 `TFPlayerController`에서 생성하고 `ATFHUD`가 위젯들을 중앙에서 관리합니다. 체력/실드/탄약/수류탄/킬로그/타이머/크로스헤어 등은 캐릭터 상태에 맞춰 갱신되며
핵심 수치는 OnRep 기반으로 “서버 값 → 클라 UI”가 자연스럽게 동기화되도록 설계했습니다.  
### 1.2 게임 흐름
기본 사이클은 아래와 같습니다.  
1) 메인/로비 진입    
2) 호스트 방 생성(리슨서버) / 참가자 조인    
3) 카운트다운 동기화 후 매치 시작    
4) 전투 진행(킬 수 DB 기록 포함)    
5) 종료 시 최다 킬 플레이어 표시    
6) 일정 쿨타임 후 재시작 또는 ESC로 메인 메뉴 복귀    
- **로비**: `ULobbyWidget` 표시, 호스트만 Start 버튼 활성화. 참가자 목록은 `TFGameState::PlayerArray` 기반으로 갱신.  
- **매치 시작**: `UAlert` 카운트다운 후 `MulticastStartCountDown`으로 시작 타이밍을 전원 동일하게 유지.  
- **전투/기록**: 상대 처치 시 닉네임/킬 수를 DB에 저장.  
---  
## 2. 시스템 구조 (System Architecture)  
### 2.1 전체 아키텍처 개요  
VeloCore는 UE 게임프레임워크의 실제 플레이 로직은 캐릭터와 컴포넌트에 집중시켰습니다.  
- **규칙 및 상태 레이어**    
  - `ATFGameMode`: 매치 규칙, 스폰/리스폰, 종료 판정, 랭킹 집계  
  - `ATFGameState`: 경과 시간, 점수, `PlayerArray` 복제    
- **플레이어 제어 레이어**    
  - `ATFPlayerController`: 입력, Server RPC 진입점, HUD 생성/갱신, 채팅  
  - `ATFPlayerState`: 닉네임, 킬/데스 수치 복제   
- **캐릭터 실행 레이어(게임 플레이 핵심)**    
  - `ATimeFractureCharacter`: 이동/생존/애니메이션      
  - `UCBComponent`: 전투(무기/탄약/사격/수류탄/HUD 연동)    
  - `UBuffComponent`: 회복/실드/이속/점프 버프   
  - `UWireComponent`: 와이어 발사/이동/쿨다운/이펙트  
- **UI 레이어**    
  - `ATFHUD` + `UCharacterOverlay`, `UChatWidget`, `ULobbyWidget`, `UAlert`, `UOverheadWidget`    
- **리소스/오브젝트 레이어**    
  - 무기: `AWeapon`, `AHitScanWeapon`, `AProjectileWeapon`, `AShotGun`   
  - 투사체: `AProjectileBullet`, `AProjectileRocket`, `AProjectileGrenade`   
  - 픽업: `APickup` 기반(Health/Shield/Speed/Jump/Ammo)  
  
### 2.2 네트워크 구조 (Listen Server + Steam OSS)  
VeloCore는 **리슨 서버 + Steam OSS** 기반입니다. 호스트 플레이어가 서버 권한(Authority)을 갖고, 모든 판정/점수/리스폰은 서버에서 결정합니다.   
#### 2.2.1 권위/소유  
- **Authority(서버)**: 피격 판정, 스코어 반영, 리스폰 결정  
- **OwnerOnly 복제**: 각 Pawn/Controller의 Owner에게만 필요한 데이터는 OwnerOnly로 복제해 트래픽을 절감  
#### 2.2.2 RPC/복제  
VeloCore에서 핵심은 “입력은 클라에서 발생하지만, 결과는 서버에서 확정한다”입니다. 그래서 대부분의 액션은 아래 패턴을 따릅니다.  
- **Client Input → Server RPC → (Server 판정/상태 갱신) → NetMulticast(연출) + OnRep(수치/UI)**  
컴포넌트별로 실제 사용은 다음과 같습니다.    
- `UCBComponent(전투)`  
  - Server RPC: `ServerFire`, `ServerReload`, `ServerSetAiming`, `ServerThrowGrenade`, `ServerLaunchGrenade`, `ServerFinishReload`    
  - Multicast: `MulticastFire`    
  - OnRep: `OnRep_EquippedWeapon`, `OnRep_SecondaryWeapon`, `OnRep_CombatState`, `OnRep_CarriedAmmo`, `OnRep_Grenades`  
- `UWireComponent(와이어)`  
  - Server RPC: `ServerFireWire`, `ServerReleaseWire`    
  - Client RPC: `ClientWireFail`, `ClientWallFail`    
  - Multicast: `MulticastWireSuccess`, `MulticastPlayWireSound`, `MulticastStartZipperSound`, `MulticastPlayWireEffects`, `MulticastDrawWire`, `MulticastStopWireEffects`    
  - OnRep: `OnRep_WireState`, `OnRep_CanFireWire`  
- `UBuffComponent(버프)`   
  - Multicast: `MulticastSpeedBuff`, `MulticastJumpBuff`  
#### 2.2.3 예외처리(전투부분)
와이어/전투는 예외처리로 버그 발생 상황을 다수 억제하였습니다.
- 와이어 발사 불가: 웅크림, 사망(Elimmed), 회피 중, 수류탄 투척 상태 등  
- 무기: 리로드 중 교체 제한, 단 샷건은 리로드 중 발사 허용  
- 와이어: 벽 근접(거리 100) 감지 시 강제 해제(무한 와이어 버그 방지)  
### 2.3 애니메이션 구조  
- `UTFAnimInstance::NativeUpdateAnimation()`에서 `Speed`, `bIsInAir`, `bIsAiming`, `bIsWireAttached`, Aim Pitch/Yaw 등을 계산해 StateMachine/BlendSpace/AimOffset에 전달  
- 전투/조준은 상체 Additive(AimOffset)로 반영하고, 몽타주(`PlayFireMontage`, `PlayReloadMontage`, `PlayThrowGrenadeMontage`)와 결합  
- 와이어 상태는 `OnRep_WireState()`를 통해 AnimInstance의 `bIsWireAttached`에 연결  
무기별 자세 보정(소켓/오프셋)도 별도 정책으로 정리했습니다.  
- 샷건: Z +23    
- 권총: Z −5    
- 스나이퍼: Yaw +90, 위치 (0, 40, 22)    
- 스나이퍼 조준: 로컬 컨트롤러에서 스코프 위젯 토글    
### 2.4 데이터 흐름
#### 2.4.1 사격  
- 입력: 좌클릭 → `UCBComponent::Fire()`  
- 서버: `ServerFire(TraceHitTarget)`로 판정(히트스캔 라인트레이스/피해 계산 또는 Projectile 스폰)  
- 전파: `MulticastFire()`로 모션/총구화염/사운드 동기화  
- UI: 탄약 감소는 OwnerOnly OnRep로 반영, 크로스헤어 스프레드 증가  
#### 2.4.2 리로드  
- 입력: `R` → `Reload()`    
- 서버: `ServerReload()` → `CombatState=Reloading` → `HandleReload()`(몽타주)  
- 탄약: `AmountToReload()` → `UpdateAmmoValues()`  
- UI: `SetHUDWeaponAmmo`, `SetHUDCarriedAmmo` 갱신  
#### 2.4.3 수류탄  
- 입력: `G` → `ThrowGrenade()`    
- 서버: `ServerThrowGrenade()`로 잔량 감소 및 HUD 반영  
- 발사: `LaunchGrenade()` → `ServerLaunchGrenade(Target)`    
- 폭발: 서버 데미지 적용 후 Multicast로 FX/사운드 전파  
#### 2.4.4 와이어  
- 입력: `Q` → `FireWire()`  
- 서버: `ServerFireWire()`   
- 성공: `bIsAttached=true`, `MoveMode=Flying`, `MulticastWireSuccess()` + 연출/사운드  
- 실패: `ClientWireFail()`  
- 이동: Tick에서 PullSpeed로 끌어당김, 벽 근접/목표 근접 시 자동 해제   
- 해제: `ServerReleaseWire()` → `MoveMode=Falling` + `MulticastStopWireEffects()`  
#### 2.4.5 버프/픽업  
- 충돌: `APickup::OnSphereOverlap()`   
- 서버: 캐릭터 캐스팅 후 `UBuffComponent` 또는 `UCBComponent::PickupAmmo()`    
- 전파: `MulticastPlayEffects()`    
- UI: 체력/실드/수류탄/탄약 갱신  
#### 2.4.6 킬/데스/리스폰  
- 서버 히트 판정 → 대상 `Elim()`   
- GameMode에서 공격자 킬 스코어 증가    
- GameState/PlayerState OnRep로 스코어/킬로그 갱신    
- 타이머 후 `RestartPlayer()` 리스폰  
---  
## 3. 개발 기술  
### 3.1 엔진/언어/구조  
- Engine: UE 5.5.4 / Windows 11 / Visual Studio 2022   
- Language: C++(핵심 로직) + Blueprint(위젯/스타일/애님 에셋 연결)   
- 설계 전략: Combat/Buff/Wire 모듈 분리, Tick 최소화(로컬 소유자/상태 기반 조건 실행)  
### 3.2 네트워크 기술  
- Steam Online Subsystem + Listen Server  
- Replication: `DOREPLIFETIME` / `ReplicatedUsing=OnRep_...` 패턴    
- OwnerOnly: `CarriedAmmo` 등은 OwnerOnly 복제로 트래픽 절감  
- RPC: Server/Client/NetMulticast 역할 분리  
### 3.3 오디오/비주얼  
- Niagara: 와이어 액션/부착/이동 연출(`SpawnSystemAttached`, `SpawnSystemAtLocation`)  
- 와이어 액션 선 대체: `DrawDebugLine`로 와이어 라인 표기   
- 카메라: 조준 FOV 보간(`InterpFOV`) + 스나이퍼 스코프 위젯  
- 오디오: SoundCue/AudioComponent, 와이어 루프 사운드(ZipperLoop), 폭발/발사/와이어는 서버 트리거로 전원 동기화  
### 3.4 외부 연동 (Steam + Firebase)  
- Steam OSS: 세션 생성/탐색/조인, 로비에서 대기/Start 관리  
- Firebase Realtime Database: 매치 중 닉네임/킬 수 업로드(랭킹/리더보드), 요청 관리는 `TFGameInstance`로 분리(재시도/에러 처리 포함)   
---  
## 4. 구현 상세 (Implementation Detail)  
### 4.1 플레이어 캐릭터 시스템  
#### 4.1.1 ATimeFractureCharacter (캐릭터 루트)  
- 애니/모션: `PlayFireMontage(bAiming)`, `PlayReloadMontage()`, `PlayThrowGrendadeMontage()`  
- 상태: `IsElimmed()`, `bIsCrouched`, `bIsDodging`   
- HUD 동기화: `UpdateHUDHealth()`, `UpdateHUDShield()` 등  
#### 4.1.2 UCBComponent (전투)  
- 장착/교체: `EquipWeapon()`로 주/보조 슬롯 배치, `SwapWeapons()`로 소켓 이동 + HUD 갱신   
- 무기별 오프셋: `EquippedWeaponPositionModify()`로 샷건/권총/스나이퍼 보정  
- 발사 흐름: `FireButtonPressed()` → `Fire()` / 발사 가능 조건 `CanFire()` / `StartFireTimer()`로 쿨타임 제어  
- 조준: `InterpFOV()`로 줌 인/아웃 자연스럽게 보간   
- 탄약/리로드: `CarriedAmmoMap`(서버 초기화), `Reload()->ServerReload()->HandleReload()->FinishReload()`  
- 크로스헤어: `SetHUDCrossharis(DeltaTime)`에서 스프레드(속도/조준/사격 반동) 반영  
#### 4.1.3 UWireComponent (와이어)  
- 입력/판정: `FireWire()->ServerFireWire()`로 서버 라인트레이스 후 타겟 확정  
- 이동/해제: 서버 Tick에서 PullSpeed 이동, 벽/목표 근접 시 해제, `ServerReleaseWire()`로 Falling 복귀    
- 연출/사운드: `MulticastWireSuccess`, `MulticastPlayWireEffects`, `MulticastStartZipperSound`, 종료 시 `MulticastStopWireEffects`  
- 쿨다운/UI: 서버 타이머로 `bCanFireWire` 복구, 로컬 UI는 `TickWireCooldownUI()`로 보조 갱신  
#### 4.1.4 UBuffComponent (버프)  
- 회복/실드: `Heal(Amount, Time)`, `Shield(Amount, Time)` → Tick 기반 램프업 적용  
- 이속/점프: `BuffSpeed(..., Time)` / `BuffJump(Value, Time)` + 타이머 만료 시 Reset  
- 조준 이속: Combat과 연동(`SetAimWalkSpeed(BaseBuff - 200.f)`)  
### 4.2 무기 및 전투 시스템  
#### 4.2.1 리로드/탄약  
- `AmountToReload() = min(탄창 여유, 보유 탄약)`    
- `UpdateAmmoValues()`에서 보유 → 탄창 이동, HUD 동시 갱신  
샷건은 1발 단위 리로드(`ShotgunShellReload()`), 조건 만족 시 애님 섹션 점프(`JumpToShotgunEnd()`)로 자연스럽게 종료 처리  
또한 발사 타이머 종료 시 “빈 탄창이면 자동 리로드 시도”(`ReloadEmptyWeapon`)  
#### 4.2.2 수류탄(상태 머신)  
투척은 단순 스폰이 아니라, 전투 상태 머신(`ECS_ThrowingGrenade`) 전환 → 몽타주 → LeftHand 부착 → HUD 수량 감소 → 종료 후 복귀  
#### 4.2.3 타깃팅/조준(크로스헤어 라인트레이스)  
화면 중앙 기준으로 스크린→월드 변환 후 멀티 히트 검사로 타깃을 탐지(`TraceUnderCrosshairs`). 적 탐지 시 크로스헤어 색상을 변경하여 “맞추고 있다/없다”를 즉시 인지시키는 UX를 제공  
---  
## 6. 결과 (Result)  
### 6.1 테스트 결과 요약  
- 1차 테스트(3인): 서버 연결 OK  
- 2차 테스트(10인): 호스트가 게임 주최 후 일부 참가자들이 Join 과정에서 문제가 발생(한 명씩 들어오는 방식으로 우회)  
→ 대규모 조인을 위한 서버/세션 최적화 필요성을 확인했습니다.  
### 6.2 유저 피드백  
초기(미완성) 테스트에서는 버그 확인과 기능 검증이 목적이었고, 피드백은 “게임성은 괜찮지만 맵 구조/게임 스피드/피격 인지(UI 편의성)” 개선 요구가 있었습니다. 이후(3주 뒤) 테스트에서는 만족도가 개선되었습니다.    
---  


</details>

---

<details>
  <summary><b>🖼️ PPT / Slide Gallery (많은 이미지 접기)</b></summary>



</details>
