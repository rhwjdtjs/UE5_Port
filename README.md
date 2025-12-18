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
- 전투(Combat): 무기 장착/교체, 사격/재장전, 수류탄, 탄약 `TMap` 관리
- 버프(Buff): 체력/실드/이속/점프 버프, 시간 기반 리셋 및 HUD 반영
- 와이어(Wire): 서버 판정, 성공 시 이동/연출 동기화, 실패 시 Client 피드백
- HUD/UI: `TFPlayerController` 생성, `ATFHUD`에서 위젯 중앙 관리, OnRep 기반 동기화

### 1.2 게임 흐름
- 로비 → 호스트 방 생성/참가자 조인 → 카운트다운 동기화 → 매치 진행 → 종료/재시작

---

## 2. 시스템 구조 (System Architecture)
### 2.1 전체 아키텍처
- Rules/State: `ATFGameMode`, `ATFGameState`
- Player: `ATFPlayerController`, `ATFPlayerState`
- Gameplay: `ATimeFractureCharacter` + `UCBComponent` + `UWireComponent` + `UBuffComponent`
- UI: `ATFHUD`, `UCharacterOverlay`, `UChatWidget`, `ULobbyWidget`, `UAlert` 등
- Objects: 무기/투사체/픽업 클래스 계층

### 2.2 네트워크 구조 (Listen Server + Steam OSS)
- Client Input → Server RPC → (서버 판정/상태 갱신) → Multicast(연출) + OnRep(UI/수치)


</details>

---

<details>
  <summary><b>🖼️ PPT / Slide Gallery (많은 이미지 접기)</b></summary>



</details>
