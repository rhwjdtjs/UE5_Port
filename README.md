  🥂위 작품은 순천향대학교 11월 4~5일에 진행한 제2회 2025 SCHU AIºSW Festival 'SW프로젝트 경진대회'에서 본선 입선 및 우수상 수여받았습니다.🥂  
                                     🥂작품 번호: SW-01 / 작품 주제: 언리얼엔진5로 만든 하이퍼 TPS 멀티플레이 게임🥂   
  
  
[![Video Link](https://github.com/user-attachments/assets/ab280274-c78b-4092-a872-b7bf37850517)](https://www.youtube.com/watch?v=35-OI47LQC0&t=1s)  
  
                                                    <<이미지 클릭시 유튜브 링크로 연결됩니다.>>  
                                                             
                                             <<온라인스팀서비스 리슨 서버로 10인 플레이 시연 동영상입니다.>>  

[![Video Link](https://github.com/user-attachments/assets/cead5c27-3767-4ef2-8184-5710b4c96c88)](https://www.youtube.com/watch?v=pTvCdJSy_EI&t=1s)    

                                                    <<이미지 클릭시 유튜브 링크로 연결됩니다.>>
                                                         <<기본 기능 설명 동영상입니다.>>
                                                              
  <img width="2081" height="1281" alt="Image" src="https://github.com/user-attachments/assets/5e1137fa-31e6-4e13-91b1-30eb59cecdca" />  
                                                          
                                                                <<클래스 계층도>>  
  
언리얼엔진5를 이용한 하이퍼 TPS 멀티플레이 게임 VELOCORE 기술서    
  
목차  
1. 개요(Introduction)  
1.1. 프로젝트명  
1.2. 개발 기간 / 참여 인원  
1.3. 개발 목적 및 배경  
  
2. 게임기획(Game Design)      
2.1. 핵심 시스템 요약    
2.2. 게임 흐름도  
    
3. 시스템 구조(System Architecture)  
3.1. 전체 시스템 구조도  
3.2. 네트워크 구조  
3.3. 애니메이션 구조  
3.4. 클래스 다이어그램  
3.5. 데이터 흐름  
     
4. 개발 기술(Development Technology)  
4.1 개발 엔진 및 언어  
4.2. 네트워크 기술  
4.3. 외부 연동 기술  
     
5. 구현 상세(Implementation Detail)(핵심만).   
5.1. 플레이어 캐릭터 시스템  
5.2. 무기 및 전투 시스템  
5.3. UI 로직.   
5.4. RPC 로직.  
5.5. 데이터동기화(Firebase DB)
          
7. 결과(Result)  
6.1. 테스트 결과 요약  
6.2. 테스트 후 유저 피드백
    
1. 개요(Introduction)   
1.1. 프로젝트명  
프로젝트의 명칭: VeloCore로 Velocity(속도)와 Core의 합성어로 하이퍼TPS의 본질인 속도를 중심으로 한 게임이다. 빠른 움직임과 반응을 핵심으로 하는 하이퍼 TPS 장르의 멀티플레이 게임으로 와이어액션과 이동능력에 관한 버프를 이용하여 정신없고 빠른템포의   게임을 이용할 수 있도록 제작되었다.  
  
1.2. 개발 기간 / 참여 인원  
개발기간: 25년 1월부터 25년 10월로 본디 유니티게임으로 캡스톤디자인 / 산학캡스톤디자인 강의를 수강했지만, 그래픽스를 잘 다뤄본적이 없어 초보자로써 유니티엔진의 그래픽 한계와 엔진 편의성, DB까지는 구현을 잘했지만 (PlayFab) 처음 제작할 때 멀티는 따  로 신경쓴적이 없어서 멀티까지 추가할려면 그동안 작성한 코드 모두를 수정해야해서 언리얼엔진으로 바꿨다.  
개발인원: 1인개발  
개발환경: UnrealEngine 5.5.4 / Visual Studio 2022 / Window11  
개발역할:  
1. 모든 프로그래밍(C++ / 블루프린트)  
2. 네트워크 및 서버-클라이언트 구조 설계  
3. UI/UX 구현(HUD / 로비 / 채팅 / 킬로그 / 킬보드 등)  
4. 애니메이션 및 사운드 시스템 연동  
5. Firebase DB를 통한 킬수 통신
  
1.3. 개발 목적 및 배경  
1.3.1. 게임 제작 계기  
1인 개발로 부담없이 만들 수 있는 장르중에 무엇이 있을까 고민하다가, 본디 원래 스토리를 즐기면서 천천히 즐기는 싱글플레이 게임을 좋아했지만, 다만 이러한 게임은 학술제 작품으로써 보여주기엔 힘들고, 최근에는 이것저것 하다보니 여유롭게 게임을 즐길시간  이 없어서 간단히 30분~1시간정도 1~2판만해도 즐길 수 있는 그런 게임을 좋아하게되었는데, 그래서 오버워치나 에이펙스와 같은 게임을 하게되었지만 오버워치는 팀원으로 인한 스트레스를 받게되고 에이펙스는 한 게임 한게임 너무 피로가심해서 (죽으면 획득한    아이템이 다 없어짐) 죽음에 대한 스트레스도 없고 팀원으로 인한 스트레스도 안받는 그런 게임을 구현하고 싶어 그리고 빠른 템포의 정신없는 그런 게임을 원하여 이번 게임을 제작하게되었다.  
  
1.3.2. 문제 인식 및 기획 의도  
대부분 TPS를 한번 살펴보면 일단 출시때부터 인기많았으며 몇 년이 지나도 인기가 많은 장르인 배틀그라운드가 있는데, 배틀그라운드는 1인칭 변환도 가능하여 FPS/TPS 둘다 가능하지만 사실 TPS 시점을 이용하여 벽을 비비면서 벽 너머의 플레이어 또는 사물을      인식하여 게임이 조금 루즈해지는 느낌이 들고, 또한 총알 낙차, 스프레이(반동)과 같은 거 때문에 총을 못쏘는 사람들은 사실 멀리있는 적을 맞추기 힘들고 즐기기가 힘들다. 사실 이러한 에러사항 때문에 FPS. TPS 의 진입 장벽이 높은 것이다. 또한 대부분의      하이퍼TPS 장르는 공중전이 별로 없다(오버워치도 사실 특수 캐릭터만 공중전을 하지 타 캐릭터들은 대부분 바닥에 있음) 그래서 공중전 + 속도감을 중심으로한 액션성을 중시 (특히 와이어액션) 하였고 총기의 반동애니메이션은 있지만 실제 에임시에 반동은        없으면 탄환 낙차는 있지만 맵크기 때문에 낙차는 이뤄지지 않아 FPS의 진입장벽도 낮추었다. 마지막으로 데스매치로 팀원의 인한 스트레스도 없으면 플레이어 사망시 3초이내에 부활하는 시스템 때문에 죽음에 대한 스트레스 없이 아무생각 없이 플레이 가능하여 게임을 가볍게 즐길 수 있다.  
  
2. 게임기획(Game Design)  
2.1. 핵심 시스템 요약  
VeloCore는 UnrealEnigne 5.5.4를 기반으로 개발된 하이퍼 TPS 멀티플레이 게임으로, 핵심은 모든 기능이 컴포넌트 단위로 설계되어 TimeFractureCharacter(VeloCore으로 이름 짓기전 이름)에 부착된 모듈들이 서로 독립적으로 작동하면서 유기적으로 연동되어, 캐릭터 이동, 전투, 상태, HUD등 모든 것들이 서버-클라이언트 간 통신하며 동작한다.    
  
2.1.1. 전투시스템  
주요 클래스 ( CBComponent, Weapon, Projectile …)    
무기장착(EquipWeapon), 사격(ServerFire->MulticastFire), 재장전(ServerReload), 수류탄(ServerThrowGrenade)가 주요 역할로, 네트워크 구조는 서버에서 모든 공격 판정을 수행 후 -> Multicast로 전파함(특히 이펙트). 탄약은 Tmap으로 무기 타입별 관리됨   
  
2.1.2. 버프시스템  
주요 클래스 (BuffComponent)  
체력회복이나 쉴드 충전, 이동속도 증가, 점프력 증가로 해당 버프를 먹고나면 일정시간 이후에 랜덤으로 해당 배열에 있는 버프가 자동생성됨. multicast RPC로 모든 플레이어 HUD에 버프 효과 반영  
  
2.1.3. 와이어시스템  
주요 클래스 (WireComponent)  
Q키를 누르면 와이어 발사함(ServerFireWire) -> 맵에 StaticMesj인 표면에 충돌하면 이동  
실패시에 ClientWireFail 호출하며 성공시, MulticastWireSuccess로 이펙트를 표시한다. 이동시에 Multicast로 디버그 라인을 그려 와이어 줄을 모두에게 표시  
  
2.1.4.네트워크 구조  
Listen Server 방식의 Steam Online Subsystem Steam 사용  
서버 권위(HasAuthority or IsLocallyControllered) 기반으로 모든 게임 로직관리  
모든 핵심 로직(피격 판정, 점수 리스폰 등)은 서버에서만 수행되고  
클라이언트는 단지 입력 요청과 UI표시에 역할이 집중된다.  
GameMode: 경기 규칙 및 스코어  
GameState: 시간/ 점수 동기화  
PlayerController: 플레이어 UI 동기화  
PlayerState: 닉네임 및 킬 데스 같은 수치 동기화  
주요 네트워크 함수 패턴  
UFUNCTION(Server, Reliable): Client->Server : 클라이언트의 입력을 서버로 전달함  
UFUNCTION(Client, Reliable): Server->특정 Client: 서버가 특정클라이언트 에게만 정보 전달  
UFUNCTION(NetMulticast, Reliable) : Server->ALL CLIENT: 서버가 전 클라이언트에게 이벤트 전파(이펙트나 사운드)  
UFUNCTION(ReplicatedUsing=OnRep_함수명): Server->Client: 변수 복제시 자동 갱신  
  
2.1.5.HUD 및 UI 시스템  
주요 클래스(TFHUD, CharacterOvelay, ChatWidget. LobbyWidget, Alert)  
HUD는 TFPlayerController를 통해 생성되며, TFHUD가 각 위젯을 중앙에서 관리  
크로스헤어, 체력/쉴드바,수류탄,탄약,킬로그,경기 타이머와 같은 UI정보는 매 프레임 캐릭터 상태를 반영함.  
HUD의 모든 수치는 OnRep기반으로 클라이언트에 실시간으로 반영됨.  
  
2.2. 게임 흐름도  
기본 사이클 -> 로비 -> 호스트(매치조인) -> 전투 -> 종료 -> 결과화면 -> 재시작 Or 로비후 종료  
  
2.2.1. 로비 진입  
LobbyWidget이 표시되어 플레이어 목록, 시작버튼이 활성화됨(호스트만 시작버튼)  
호스트가 시작버튼을 누르면 전투맵으로 맵을 전환함 (Listen)  
참가자 목록은 TFGmaeState::PlayerArray로 매프레임 갱신한다.  
  
2.2.2. 매치 시작  
Alert 위젯으로 설정한 시간초의 카운트 다운 표시후에 . MulticastStartCountDown으로 모든 클라이언트에게 동기화되어 시작 타이밍이 동일하게 유지되어 게임을 시작함.  
  
2.2.3. 전투 단계  
플레이어는 전투를 하며, 만약 상대방을 죽일시에 킬수가 반영되어 DB에 저장됨 ( 닉네임 킬수)  
  
2.2.4. 매치 종료  
매치종료시 킬수가 가장 많은 플레이어의 닉네임을 표시함  
  
2.2.5.재시작 OR 로비후 종료  
매치가 끝난뒤 쿨타임이 지나면 매치는 재시작이 되고 ESC를 눌러 나갈사람은 메인메뉴로 돌아가 게임을 종료 가능하다.  
  
3. 시스템 구조  
3.1. 전체 시스템 구조도  
3.1.1 레이어별 책임  
   
게임 규칙/상태 레이어  
ATFGameMode: 매치 규칙, 스폰/리스폰, 종료 판정, 랭킹 집계  
ATFGameState: 경과 시간, 점수(스코어), PlayerArray 복제  
  
플레이어 제어 레이어  
ATFPlayerController: 입력, Server RPC 진입점, HUD 생성/갱신, 채팅  
ATFPlayerState: 닉네임, 킬/데스 수치 복제    
  
캐릭터 실행 레이어  
ATimeFractureCharacter: 이동/생존 로직(플레이어 본인의 모든 로직) (C++ + AnimBP)  
UCBComponent: 전투(무기/탄약/사격/수류탄/HUD 연동)  
UBuffComponent: 힐/실드/이속/점프 버프   
UWireComponent: 와이어 발사/이동/쿨다운/이펙트  
  
UI  
ATFHUD: 위젯 총괄  
UCharacterOverlay, UChatWidget, ULobbyWidget, UAlert, UOverheadWidget  
  
리소스/오브젝트 레이어  
무기: AWeapon(기반), AHitScanWeapon, AProjectileWeapon, AShotGun  
투사체: AProjectileBullet, AProjectileRocket, AProjectileGrenade  
픽업: APickup기반(Health/Shield/Speed/Jump/Ammo)  
이펙트/사운드: Niagara 시스템, Cue/AudioComponent  
  
3.1.2 아키텍처 개요  
[ TFGameMode ] ──(규칙/스폰/종료)──▶ [ TFGameState(복제) ]  
        ▲                                     ▲  
        │                                     │  
  Server RPC                                OnRep  
        │                                     │  
[ TFPlayerController ] ──HUD/입력──▶ [ TFHUD + Widgets ]  
        │  
        ▼  
[ TimeFractureCharacter ]  
   ├─ CBComponent   (Combat: 무기/탄약/사격/수류탄/HUD)  
   ├─ BuffComponent (Heal/Shield/Speed/Jump + Timer)  
   └─ WireComponent (Fire/Cooldown/FX)  
        │  
        ▼  
[ Weapons / Projectiles / Pickups / FX ]  
  
3.2. 네트워크 구조  
3.2.1 세션/권위/소유  
세션: Listen Server + Steam OSS (호스트 플레이어가 서버)  
권위: HasAuthority()측(서버)이 판정/스코어/리스폰을 결정  
소유: 각 Pawn/Controller의 Owner에게만 필요한 데이터는 OwnerOnly 복제  
  
3.2.2 클래스별 RPC     
UCBComponent (전투)  
Server RPC: ServerFire, ServerReload, ServerSetAiming, ServerThrowGrenade, ServerLaunchGrenade, ServerFinishReload  
Multicast: MulticastFire  
OnRep: OnRep_EquippedWeapon, OnRep_SecondaryWeapon, OnRep_CombatState, OnRep_CarriedAmmo, OnRep_Grenades  
  
UWireComponent (와이어)  
Server RPC: ServerFireWire, ServerReleaseWire  
Client RPC: ClientWireFail, ClientWallFail  
Multicast: MulticastWireSuccess, MulticastPlayWireSound, MulticastStartZipperSound, MulticastPlayWireEffects, MulticastDrawWire, MulticastStopWireEffects  
OnRep: OnRep_WireState, OnRep_CanFireWire  
  
UBuffComponent (버프)  
Multicast: MulticastSpeedBuff, MulticastJumpBuff  
  
3.2.3 흐름 패턴  
입력 -> 서버 판정 -> 전송/복제 -> 클라 UI  
Client 입력 -> Server() -> 서버 판정/갱신 -> NetMulticast(이펙트) + OnRep(수치) -> HUD 갱신  
OwnerOnly 최적화  
탄약(CarriedAmmo)은 OwnerOnly로 복제하여 네트워크 트래픽 절감  
  
쿨다운/타이머  
서버: FTimerManager로 로직 타이밍 관리  
클라: 와이어 쿨타임 UI는 로컬 타이머로 보조 갱신 (TickWireCooldownUI)  
  
3.2.4 예외처리  
와이어: 웅크림, Elimmed, 회피 중, 수류탄 투척 상태 -> 발사 불가  
무기: 리로드 중 교체 제한, 샷건은 리로드 중 발사 허용  
충돌: 와이어는 가까운 벽(100)감지 시 강제 해제 (무한 와이어액션 버그)   
  
3.3. 애니메이션 구조  
3.3.1 AnimBP 파이프라인  
소스: ATimeFractureCharacter, UCBComponent, UWireComponent  
전달: UTFAnimInstance::NativeUpdateAnimation()에서  
Speed, bIsInAir, bIsAiming,bIsWireAttached, Aim Pitch/Yaw 계산  
AnimBlueprint의 State Machine / BlendSpace / AimOffset에 입력  
  
3.3.2 상태  
Locomotion: Idle/Walk/Run <-> InAir(점프/낙하)  
Combat Additive: 조준 시 상체 AimOffset 적용 (상체 본 분리하여 상하체 애님 분리(재장전)    
Montage: PlayFireMontage, PlayReloadMontage, PlayThrowGrenadeMontage  
와이어: OnRep_WireState() -> AnimInstance bIsWireAttached갱신  
  
3.3.3 소켓/무기 자세 보정  
부착 소켓: RightHandSocket, LeftHandSocket, BackWeaponSocket  
무기별 오프셋:  
샷건: 위치 Z +23  
권총: 위치 Z −5  
스나이퍼: 회전 Yaw +90, 위치 (0, 40, 22)  
스나이퍼 조준: 로컬 컨트롤러에 스코프 위젯 토글(시점 전환 UX)  
   
3.4. 클래스 다이어그램  
3.4.1 구조  
AActor  
 ├─ ACharacter  
 │   └─ ATimeFractureCharacter  
 │       ├─ UCBComponent  
 │       ├─ UBuffComponent  
 │       └─ UWireComponent  
 ├─ AWeapon  
 │   ├─ AHitScanWeapon  
 │   ├─ AProjectileWeapon  
 │   ├─ AShotGun  
 │   └─ ACasing  
 ├─ AProjectile  
 │   ├─ AProjectileBullet  
 │   ├─ AProjectileRocket(URocketMovementComponent)  
 │   └─ AProjectileGrenade  
 ├─ APickup  
 │   ├─ AHealthPickup / AShieldPickup  
 │   ├─ ASpeedPickup  / AJumpPickup  
 │   └─ AAmmoPickup   / APickupSpawnPoint  
 ├─ AHUD → ATFHUD   
 ├─ APlayerController -> ATFPlayerController  
 ├─ APlayerState -> ATFPlayerState  
 ├─ AGameStateBase -> ATFGameState  
 └─ AGameModeBase -> ATFGameMode / ALBGameMode  
   
3.5. 데이터 흐름  
3.5.1 사격  
입력: 마우스 좌클릭→ UCBComponent::Fire()  
서버 판정: ServerFire(TraceHitTarget)  
히트스캔: 서버 라인트레이스/피해 계산  
총알: 서버가 AProjectile스폰  
클라전송: MulticastFire()(모션/총구화염/사운드)  
HUD: 탄약 감소(OnRep_CarriedAmmoOwnerOnly), 크로스헤어 스프레드 증가  
  
3.5.2 리로드   
입력: R -> Reload()  
서버: ServerReload() -> CombatState = Reloading -> HandleReload()(Montage)  
탄약: AmountToReload() -> UpdateAmmoValues()  
HUD: SetHUDWeaponAmmo, SetHUDCarriedAmmo즉시 갱신  
  
3.5.3 수류탄(투척/폭발)  
입력: G -> ThrowGrenade()(상태/잔량 체크)  
서버: ServerThrowGrenade() -> 잔량 감소/HUD 반영  
발사: LaunchGrenade() -> ServerLaunchGrenade(Target) -> 수류탄 투척  
효과: 폭발 시 서버 데미지 적용 -> Multicast 폭발 FX/사운드  
  
3.5.4 와이어(발사/이동/해제)  
입력: Q -> FireWire()  
서버: ServerFireWire()(라인트레이스/타겟 결정)  
성공: bIsAttached=true, MoveMode=Flying, MulticastWireSuccess()+ FX/SOUND
실패: ClientWireFail()  
이동: Tick에서 PullSpeed로 목표점까지 끌어당김; 벽거리<100또는 근접시 자동 해제  
해제: ServerReleaseWire() -> MoveMode=Falling, MulticastStopWireEffects() 
  
3.5.5 버프/픽업    
충돌: APickup::OnSphereOverlap()  
서버: 대상 캐릭터 캐스팅 -> UBuffComponent::또는 UCBComponent::PickupAmmo()  
클라전송: MulticastPlayEffects()   
HUD: UpdateHUDHealth/Shield/Grenades/CarriedAmmo  
  
3.5.6 킬/데스/리스폰  
서버 히트 판정 -> 피해 대상 Elim()  
ATFGameMode가 상대방을 죽인 플레이어의 킬 스코어 증가  
ATFGameState/PlayerState OnRep로 스코어·킬로그 갱신  
타이머 후 사망자 RestartPlayer()리스폰  
  
4. 개발 기술  
4.1 개발 엔진 및 언어  
개발 환경  
Engine: Unreal Engine 5.5.4  
OS/IDE: Windows 11 / Visual Studio 2022  
언어: C++(UE API)+ Blueprint하이브리드  
핵심 로직 C++: 전투(무기/탄약/재장전/수류탄), 와이어 액션, 버프, 네트워크 RPC/복제  
블루프린트: 위젯 배치/스타일, 일부 애님 에셋 연결
  
프로젝트 구조  
캐릭터 본체: ATimeFractureCharacter  
기능: UCBComponent(전투), UBuffComponent(버프), UWireComponent(와이어)  
게임프레임워크:ATFGameMode,ATFGameState,ATFPlayerController,ATFPlayerState,UTFGameInstance  
UI/HUD: ATFHUD+ UCharacterOverlay, UChatWidget, ULobbyWidget, UAlert, UOverheadWidget  
무기/투사체: AWeapon파생(AHitScanWeapon, AProjectileWeapon, AShotGun) / AProjectileBullet, AProjectileGrenade, AProjectileRocket(+ URocketMovementComponent)  
아이템픽업: APickup기반(Health/Shield/Speed/Jump/Ammo)  
  
성능·코딩 전략  
모듈형 컴포넌트 설계(Combat/Buff/Wire 분리): 유지보수·확장성 확보  
Tick 최소화/조건부 실행: 로컬 소유자·상태에 따라 분기(IsLocallyControlled)    
  
4.2 네트워크 기술  
세션/플랫폼  
Steam Online Subsystem (OSS)+ Listen Server  
호스트가 서버 권한을 갖고, 클라이언트가 Steam 세션을 통해 조인   
  
권위 모델 / 데이터 복제  
서버 권위(Authority): 피격/스코어/리스폰/버프 확정은 서버 전용  
복제(Replication): DOREPLIFETIME/ReplicatedUsing=OnRep_…로 상태 동기화  
예: EquippedWeapon, SecondaryWeapon, bisAiming, CombatState, Grenades  
OwnerOnly 복제: 탄약 CarriedAmmo는 소유자 전용(COND_OwnerOnly) -> 트래픽 절감  
  
RPC 패턴  
UFUNCTION(Server,Reliable):클라입력->서버판정(예: ServerFire, ServerReload, ServerThrowGrenade, ServerFireWire)  
UFUNCTION(Client, Reliable): 서버 -> 특정 클라 피드백 (예: ClientWireFail, ClientWallFail)  
UFUNCTION(NetMulticast, Reliable): 서버->전 클라 공용 이벤트(이펙트/사운드) (예: MulticastFire, MulticastPlayWireEffects, MulticastWireSuccess)  
OnRep_:변수 복제 시 HUD 자동 반용(예: OnRep_CarriedAmmo, OnRep_Grenades, Onrep_Equippedweapon)  
  
4.3 외부 연동 기술  
Steam Online Subsystem  
세션 생성/탐색/조인: 메인 메뉴에서 Create/Join  
Listen Server로 호스트 전환, 클라들은 Steam 세션을 통해 접속  
로비 : ALBGameMode/ULobbyWidget조합으로 대기/Start 관리  
  
Firebase (랭킹/리더보드)  
매치중 실시간으로 닉네임/킬 수를 Firebase Realtime Database에 업로드  
(프로젝트 내 TFGameInstance에서 요청 관리하도록 분리: 결과 전송/에러 전송/재시도)   
  
5. 구현 상세(핵심만)  
5.1. 플레이어 캐릭터 시스템  
5.1.1 캐릭터 본체: ATimeFractureCharacter  
역할: 이동/생존/애니메이션 루트, 전투·와이어·버프 컴포넌트의 소유자(Owner).  
핵심 인터페이스  
애님/모션: PlayFireMontage(bool bAiming), PlayReloadMontage(), PlayThrowGrendadeMontage()  
상태: IsElimmed(), bIsCrouched, bIsDodging  
카메라: GetFollowCamera()(FOV 보간 대상)  
부착물: GetAttachedGrenade()(수류탄 메시)  
HUD 동기화: UpdateHUDHealth(), UpdateHUDShield(), 등
  
5.1.2 전투 컴포넌트: UCBComponent  
장착/교체  
EquipWeapon(AWeapon*) -> 주/보조 무기 슬롯 자동 배치  
SwapWeapons() -> RightHand(주무기)/Back(보조무기) 소켓 이동및 HUD 탄약 갱신  
위치 보정: EquippedWeaponPositionModify()(샷건/권총/스나이퍼별 오프셋)  
발사/쿨타임/사격감  
입력 처리: FireButtonPressed(bool) -> Fire()  
발사 가능 조건: CanFire()  
샷건은 리로드 중에도 발사허용(대다수의 게임의 로직)  
발사: StartFireTimer()/FireTimerFinished()  
조준 FOV: InterpFOV()(조준 중 ZoomedFOV <-> 기본 FOV 보간)  
  
탄약/리로드  
보유탄약: TMap<EWeaponType,int32> CarriedAmmoMap(서버 초기화: InitializeCarriedAmmo())  
리로드: Reload() -> ServerReload() -> HandleReload() -> (Montage notify) FinishReload()  
탄약 반영: AmountToReload()→ UpdateAmmoValues()/ (샷건) UpdateShotgunAmmoValues()  
수류탄  
ThrowGrenade()(상태/잔량/HUD체크)->ServerThrowGrenade()->LaunchGrenade()->ServerLaunchGrenade(Target)  
HUD 연동  
프레임 갱신: SetHUDCrossharis(DeltaTime)(스프레드=속도·조준·사격 반동)  
  
5.1.3 와이어 컴포넌트: UWireComponent  
발사/판정  
입력: FireWire()->ServerFireWire()  
서버 라인트레이스 후 WireTarget확정, bIsAttached=true, Movement=Flying  
이동/해제  
Tick(서버): 목표까지 PullSpeed로 가속 이동  
벽 근접(<100)또는 목표 근접(≤120)시 ReleaseWire()  
해제: ServerReleaseWire() -> Movement=Falling  
이펙트/사운드  
성공: MulticastWireSuccess()(쿨다운 시작 & UI)  
연출: MulticastPlayWireEffects(Start,Target)(팔·다리 소켓 Niagara), MulticastStartZipperSound()(루프 사운드)  
종료 정리: MulticastStopWireEffects()  
쿨다운/UI  
서버 쿨다운 타이머 종료 시 ResetWireCooldown() -> bCanFireWire=true  
로컬UI:WireCooldownWidget/TickWireCooldownUI() / UpdateWireCooldownUI()    
  
5.1.4 버프 컴포넌트: UBuffComponent  
회복/실드  
Heal(Amount, Time)/ Shield(Amount, Time) -> 매 Tick HealRampUp()/ShieldRampUp()적용  
이속/점프 버프  
BuffSpeed(BaseBuff, CrouchBuff, Time) -> 타이머 만료 시 ResetSpeed()  
조준 이속은 Combat과 동기화(CB -> SetAimWalkSpeed(BaseBuff-200.f))  
BuffJump(Value, Time) -> 타이머 만료 시 ResetJump()  
  
5.2. 무기 및 전투 시스템  
5.2.1 리로드/탄약  
리로드  
AmountToReload()= min(탄창 여유, 보유 탄약)  
UpdateAmmoValues()에서 보유->탄창이동, HUD 동시 갱신  
샷건 리로드  
1발 단위: ShotgunShellReload()  
조건 만족 시 섹션 점프(애니메이션 섹션 점프): JumpToShotgunEnd()  
자동 리로드  
FireTimerFinished()→ ReloadEmptyWeapon()(빈 탄창 시 자동 시도)  
  
5.2.2 수류탄  
상태 머신  
ThrowGrenade()에서 ECS_ThrowingGrenade전환 -> 몽타주 시작, LeftHand 부착, HUD 수량 감소  
투척 종료 시 ThrowGrenadeFinished()에서 복귀/애니메이션 종료  
  
5.2.3 타깃팅/조준  
Crosshair 라인트레이스: TraceUnderCrosshairs()  
화면 중앙 스크린->월드 변환 후 멀티 히트 검사, 허공이면 Null반환  
만약 적 탐지시 크로스헤어 색상(RED)변경  

5.3. UI 로직  
HUD 초기화:
ATFPlayerController::BeginPlay()->TFHUD생성및CharacterOverlay,ChatWidget,KillFeed,Scoreboard등 추가  
HUD 데이터 갱신 흐름:  
Character가 체력, 실드, 탄약 등 변경 시 OnRep_Health()등 호출  
PlayerController->SetHUDHealth()/ SetHUDShield()호출  
UTFHUD가 내부 위젯(CharacterOverlay)에 전달  
채팅 시스템:  
UChatWidget::AddChatMessage() ->ScrollBox 갱신 + AutoClose 타이머  
OnTextCommitted()으로 입력 후 서버 브로드캐스트    
  
5.4. RPC 로직  
VeloCore는 Listen Server 기반의 Steam Online Subsystem(OSS)을 사용하며,  
서버가 게임의 모든 로직을 authority로서 판단하고,  
클라이언트는 입력 요청과 HUD/UI 반영에만 집중하는 구조로 설계되어 있다.  
즉,  
서버(Server): 모든 상태의 진실(Truth)을 결정하는 권위자  
클라이언트(Client): 서버의 판단을 시각적으로 반영하는 피어  
이 구조는 Unreal Engine의 Replication(복제) 시스템과  
Remote Procedure Call(RPC)을 기반으로 한다.  
    
실제 네트워크 흐름 예시  
A. 사격 (Fire)  
Client 입력 감지  
InputAction Fire -> CombatComponent->FireButtonPressed(true)  
Server RPC 요청  
ServerFire()실행 -> 서버만 피격 판정 수행  
서버 판정  
라인트레이스(LineTrace) -> 타격 성공 시 대상 캐릭터에 데미지 적용  
Multicast 전파  
MulticastFire()호출 -> 모든 클라이언트에서 동일한 Muzzle Flash, 사운드 재생  
결과 반영  
피격 클라이언트에서 OnRep_Health() -> HUD 자동 갱신  
  
B. 와이어 시스템 (WireComponent)  
Client Q 입력  
WireComponent->FireWire() -> ServerFireWire()RPC 송신  
Server 처리  
라인트레이스로 충돌 대상 계산 -> 성공 시 bIsAttached = true  
이동 모드: CharacterMovement->SetMovementMode(MOVE_Flying)  
Multicast 이펙트 전파  
MulticastWireSuccess() -> 모든 클라에 Niagara 이펙트, 사운드 재생  
  
5.5. 데이터 동기화 및 외부 연동 (Firebase)   
Firebase 연동 구조  
게임 종료 시 UTFGameInstance::SendKillToFirebase()호출  
FJsonObject생성 → FHttpModule::CreateRequest()로 REST POST  
URL 예: https://<DB_URL>/leaderboard/<PlayerName>.json  
데이터 형식:  
{ "name": "플레이어 스팀 닉네임", "kills": 15, "updatedAt": 123456.78 }   
  
6. 결과(Result)    
6.1. 테스트 결과 요약    
첫 테스트 3인 기준 서버 완결 OK / 두 번째 테스트 10인 기준 호스트 게임 주최후 참가자들이 Join부분에서 문제가 생김. ( 한명씩 들어오는걸로 넘어감) -> 서버 최적화가 필요하다는걸 확인  
  
6.2. 테스트 후 유저 피드백  
개발 미완성시에 기능 및 버그 확인을 위해 첫 테스트 (3인)  
  
https://youtu.be/WeMqVnDT7_Y?si=g0drKmB83UI-wmF-  
게임성은 괜찮지만 맵의 구조 / 게임의 스피드/ 자신이 맞았는지 안맞았는지 등 UI편의성 개선 요구  
다음 테스트 (3주뒤) 10인 테스트에서는 모두 만족스러운 테스트를 함  

