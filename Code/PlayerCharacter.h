// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "InputActionValue.h"
#include "PlayerCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PF_MAGICKNIGHT_API APlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

private:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	// 카메라 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	//MappingContext 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	// W,A,S,D Move Input Action 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	//시점 변경 Look Input Action 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	//달리기 Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* DashAction;

	//회피 Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* DodgeAction;

	// 무기 장착 Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TArray<class UInputAction*>EquipActions;

	// 일반공격 Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ComboAction;

	// 스킬 공격(Q) Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* QSkillAction;

	// 스킬공격(E) Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ESkillAction;

	// 특수 스킬(EF)공격 Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* EFSkillAction;

	// 방어 입력 Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* BlockAction;

	// 락온 입력 Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LockOnAction;

	// 힐링 입력 Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* HealingAction;

	//시점 회전 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	float LookRate = 0.35f;

	//걷기 이동 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float WalkSpeed = 380.0f;
	
	//달리기 이동 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float RunSpeed = 650.0f;

	//----------------------------------------------------------------------무기-----------------------------------------------------------------------\\

	//검 블루프린트 지정 -> 에디터에서 셋팅
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AWeapon> SwordClass;

	//class에서 사용할 검 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	class AWeapon* Sword;

	// 장착될 장소 배열 0  = 검, 1 = 도끼  ....(무기가 늘어날때 마다 추가 할 것)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	TArray<FName>EquipSockets;

	//현재 장착한 무기
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	class AWeapon* CurrentWeapon;

	//방패  블루프린트 지정 -> 에디터에서 셋팅
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AArmBarrier> ArmBarrierClass;

	//class에서 사용할 방패 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	class AArmBarrier* ArmBarrier;

	//----------------------------------------------------------------------무기-----------------------------------------------------------------------\\

	//튕겨내기 사용 여부
	bool UseParrying = false;

	//튕겨내기 종료 시간 -> 테스트를 위해 블루프린트에서 설정 가능 하도록 구현
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Parrying", meta = (AllowPrivateAccess = "true"))
	float InDelayTime_Parry = 0.5f;

	//체간 감소 타이머 작동 시간 -> 테스트를 위해 블루프린트에서 설정 가능 하도록 구현
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Parrying", meta = (AllowPrivateAccess = "true"))
	//float InDelayTime_Posture = 1.f;

	//-------------- Lock On ---------------//

	//바라볼 적 객체
	class AEnemyCharacter* LockOnEnemy = nullptr;

	//LockOn 사용 여부
	bool bUseLockOn = false;

	//-------------- Lock On ---------------//

	//체간 감소용 타이머
	//FTimerHandle PostureHandle;

public:
	// 생성자
	APlayerCharacter();

	void Tick(float DeltaTime) override;

	void PlayerSetup();

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// To add mapping context
	virtual void BeginPlay();

	//------------------------ 카메라 및 SpringArm 반환 ------------------------//
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool GetUseLockOn() const { return bUseLockOn; }
	//--------------------------------------------------------------------------------//
	
	//현재 사용 중인 무기 반환
	UFUNCTION(BlueprintCallable)
	FORCEINLINE class AWeapon* GetCurrentWeapon() const { return CurrentWeapon; }

	//태그 생성 및 해당 태그를 가진 GameplayAbility 작동
	void MakeTagAndActive(FString TagName);

	//W, A, S, D 움직임 
	void Move(const FInputActionValue& Value);

	// 움직임 종료 -> Tag 제거
	void MoveEnd();

	// 시점 변경
	void Look(const FInputActionValue& Value);

	// 달리기 사용 및 종료
 	void Dash();
	void DashEnd();

	//회피 사용
	void Dodge();

	// 일반 공격 사용(콤보)
	void ComboAttack();

	//Q Skill 사용
	void UseQSkill();

	// E Skill 사용
	void UseESkill();

	// EF Skill 사용
	void UseEFSkill();

	//방어 시작 및 종료
	void BlockStart();
	void BlockEnd();

	//무기 소환 -> 장착해제(다른 무기 장착 중 일때) -> 장착
	void SwordSummons();
	void WeaponUnequip();

	// 무기 장착은 블루프린트에서 호출 -> 무기 이동 완료후 장착 됨
	UFUNCTION(BlueprintCallable)
	void WeaponEquip(FName EquipSocketName, AWeapon* Weapon);

	//적이 공격하여 피격 될시 GameplayEffect 호출
	void TakeDamageFromEnemy(EDamageEffectType DamgeType); 
	// EF 충전 함수
	void EFCharge();

	//락온 기능 사용
	void LockOn();
	void LockOnReset();

	//사망 함수
	void Die() override;

	//회복 스킬 사용
	void Healing();

	//void DecreasePosture();

	//UFUNCTION(BlueprintCallable)
	//FORCEINLINE void ReStartPostureTimer() const { GetWorldTimerManager().UnPauseTimer(PostureHandle); }
};
