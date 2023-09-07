// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "EnemyCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameplayTagContainer.h"
#include "Weapon.h"
#include "ArmBarrier.h"
#include "Kismet/KismetMathLibrary.h"
#include "CollisionShape.h"
#include "Kismet/GameplayStatics.h"


APlayerCharacter::APlayerCharacter()
{
	// SpringArm ����
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // 
	CameraBoom->bUsePawnControlRotation = true; 

	//Camera ����
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); 
	FollowCamera->bUsePawnControlRotation = false; 

	PlayerSetup();

	DefaultMappingContext = nullptr;
	MoveAction = nullptr;
	LookAction = nullptr;
	DashAction = nullptr;
	DodgeAction = nullptr;
	ComboAction = nullptr;
	Sword = nullptr;
	CurrentWeapon = nullptr;
	BlockAction = nullptr;
	bUseLockOn = false;
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsValid(LockOnEnemy)) //���� ���� ������, ī�޶� ������ ��ü�� ����, ���� ���� ���游 ���� �ϵ��� ����
	{
		FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), LockOnEnemy->GetActorLocation());
		FRotator LockOnRotation = FRotator (GetController()->GetControlRotation().Pitch, LookAt.Yaw, LookAt.Roll);
		GetController()->SetControlRotation(LockOnRotation);
	}
}

void APlayerCharacter::PlayerSetup()
{
	// �÷��̾� ĸ�� ������Ʈ ũ�� ����
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// ��Ʈ�ѷ� ȸ����, ĳ���� ȸ�� �ȵǵ��� ����
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// ĳ���� ȸ�� ����
	GetCharacterMovement()->bOrientRotationToMovement = true; // �̵��������� ȸ��
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ȸ�� �ӵ�

	//ĳ���� ������ ���� ����
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
}

void APlayerCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
		ArmBarrier = GetWorld()->SpawnActor<AArmBarrier>(ArmBarrierClass);
		ArmBarrier->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, ArmBarrier->GetEquipSocketName());

		Sword = GetWorld()->SpawnActor<AWeapon>(SwordClass);
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) 
	{
		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &APlayerCharacter::MoveEnd);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);

		//�޸���
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Dash);
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Completed, this, &APlayerCharacter::DashEnd);

		//ȸ��(������ & ����)
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Dodge);

		//���� ����
		EnhancedInputComponent->BindAction(EquipActions[0], ETriggerEvent::Triggered, this, &APlayerCharacter::SwordSummons);

		//�Ϲݰ���(�޺�)
		EnhancedInputComponent->BindAction(ComboAction, ETriggerEvent::Started, this, &APlayerCharacter::ComboAttack);

		//��ų ���
		EnhancedInputComponent->BindAction(QSkillAction, ETriggerEvent::Triggered, this, &APlayerCharacter::UseQSkill); //Q Skill
		EnhancedInputComponent->BindAction(ESkillAction, ETriggerEvent::Triggered, this, &APlayerCharacter::UseESkill); // E Skill
		EnhancedInputComponent->BindAction(EFSkillAction, ETriggerEvent::Triggered, this, &APlayerCharacter::UseEFSkill); //Ư�� ��ų

		//��� ���
		EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Triggered, this, &APlayerCharacter::BlockStart);
		EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Completed, this, &APlayerCharacter::BlockEnd);

		//Lock On ���
		EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Started, this, &APlayerCharacter::LockOn);
	}
}
void APlayerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		if (GetAbilitySystemComponent()->GetTagCount(FGameplayTag::RequestGameplayTag(FName("Player.State.DoNotMove"))) <= 0)
		{
			// add movement 
			AddMovementInput(ForwardDirection, MovementVector.Y);
			AddMovementInput(RightDirection, MovementVector.X);
		}

		//�÷��̾�� Tag ����, ���� �±װ� ���Ǹ� �������� ����.
		if (GetAbilitySystemComponent()->GetTagCount(FGameplayTag::RequestGameplayTag(FName("Player.Move"))) <= 0)
		{
			GetAbilitySystemComponent()->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.Move"))); //�̵� ���϶� ����
		}

		// �̵� ���⿡ ���� ���� �±� ����, -> �Ŀ� Ÿ�� ���� ���¿����� ȸ�ǿ� ����� ����
		if (MovementVector.Y > 0)
		{
			if (GetAbilitySystemComponent()->GetTagCount(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveFwd"))) <= 0)
			{
				// �ٸ� ���� �±� ����
				GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveBwd")));
				GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveLeft")));
				GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveRight")));

				//W Ű�� ���� -> ������ �̵� -> MoveFwd Tag ����
				GetAbilitySystemComponent()->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveFwd")));
			}
		}
		else if (MovementVector.Y < 0)
		{
			if (GetAbilitySystemComponent()->GetTagCount(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveBwd"))) <= 0)
			{
				// �ٸ� ���� �±� ����
				GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveFwd")));
				GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveLeft")));
				GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveRight")));

				//S Ű�� ���� -> �ڷ� �̵� -> MoveBwd Tag ����
				GetAbilitySystemComponent()->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveBwd")));
			}
		}
		else if (MovementVector.X < 0)
		{
			if (GetAbilitySystemComponent()->GetTagCount(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveLeft"))) <= 0)
			{
				// �ٸ� ���� �±� ����
				GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveFwd")));
				GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveBwd")));
				GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveRight")));

				//A Ű�� ���� -> �·� �̵� -> MoveLeft Tag ����
				GetAbilitySystemComponent()->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveLeft")));
			}
		}
		else if (MovementVector.X > 0)
		{
			if (GetAbilitySystemComponent()->GetTagCount(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveRight"))) <= 0)
			{
				// �ٸ� ���� �±� ����
				GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveFwd")));
				GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveBwd")));
				GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveLeft")));

				//D Ű�� ���� -> ��� �̵� -> MoveRight Tag ����
				GetAbilitySystemComponent()->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveRight")));
			}
		}
	}
}

void APlayerCharacter::MoveEnd()
{
	//�̵��� ���߸� �̵� �� ���� Tag ����
	GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.Move")));
	GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveFwd")));
	GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveBwd")));
	GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveLeft")));
	GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveRight")));
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X * LookRate);
		AddControllerPitchInput(LookAxisVector.Y * LookRate);
	}
}

// �޸��� ��� -> �̵� �ӵ� ���� �� Dash Tag ����
void APlayerCharacter::Dash()
{
	if (Controller != nullptr)
	{
		if (!(GetCharacterMovement()->IsFalling()))
		{
			if (GetAbilitySystemComponent()->GetTagCount(FGameplayTag::RequestGameplayTag(FName("Player.State.UseBlock"))) <= 0)
			{
				GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
				if (GetAbilitySystemComponent()->GetTagCount(FGameplayTag::RequestGameplayTag(FName("Player.Dash"))) <= 0)
				{
					GetAbilitySystemComponent()->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.Dash")));
				}
			}
		}
	}
}

// �޸��� Ű���� ���� �� -> Tag ���� �� �ӵ� ����
void APlayerCharacter::DashEnd()
{
	if (Controller != nullptr)
	{
		if (!(GetCharacterMovement()->IsFalling()))
		{
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
			GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.Dash")));
		}
	}
}

// �±� ���� �� �ش� Tag�� ���� GamepalyAbility �۵�
void APlayerCharacter::MakeTagAndActive(FString TagName)
{
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName(TagName)));
	GetAbilitySystemComponent()->TryActivateAbilitiesByTag(TagContainer);
}

//ȸ��
void APlayerCharacter::Dodge()
{
	if (Controller != nullptr)
	{
		if (!(GetCharacterMovement()->IsFalling()))
		{
			if (bUseLockOn)
			{
				if (GetAbilitySystemComponent()->GetTagCount(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveFwd"))) > 0)
				{
					MakeTagAndActive("Player.Dodge.Rolling");
				}
				else if (GetAbilitySystemComponent()->GetTagCount(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveBwd"))) > 0)
				{
					MakeTagAndActive("Player.Dodge.Rolling.Bwd");
				}
				else if (GetAbilitySystemComponent()->GetTagCount(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveLeft"))) > 0)
				{
					MakeTagAndActive("Player.Dodge.Rolling.Left");
				}
				else if (GetAbilitySystemComponent()->GetTagCount(FGameplayTag::RequestGameplayTag(FName("Player.State.MoveRight"))) > 0)
				{
					MakeTagAndActive("Player.Dodge.Rolling.Right");
				}
			}
			else if (!bUseLockOn)
			{
				MakeTagAndActive("Player.Dodge.Rolling");
			}
		}
	}
}

//�޺� ����
void APlayerCharacter::ComboAttack()
{
	if (!(GetCharacterMovement()->IsFalling()))
	{
		MakeTagAndActive("Player.Attack");
	}
}

//��ų ���(Q)
void APlayerCharacter::UseQSkill()
{
	if (!(GetCharacterMovement()->IsFalling()))
	{
		MakeTagAndActive("Player.Skill.QSkill");
	}
}

//��ų ���(E)
void APlayerCharacter::UseESkill()
{
	if (!(GetCharacterMovement()->IsFalling()))
	{
		MakeTagAndActive("Player.Skill.ESkill");
	}
}

// Ư�� ��ų ���
void APlayerCharacter::UseEFSkill()
{
	if (!(GetCharacterMovement()->IsFalling()))
	{
		MakeTagAndActive("Player.Skill.EFSkill");
	}
}

//��� ����
void APlayerCharacter::BlockStart()
{
	if (!(GetCharacterMovement()->IsFalling()))
	{
		if (!CanUseParrying)
		{
			if (GetAbilitySystemComponent()->GetTagCount(FGameplayTag::RequestGameplayTag(FName("Player.State.UseParrying"))) <= 0)
			{
				GetAbilitySystemComponent()->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.State.UseParrying"))); 
				CanUseParrying = true;

				FTimerHandle ParryingEndHandle;
				GetWorld()->GetTimerManager().SetTimer(ParryingEndHandle, FTimerDelegate::CreateLambda([&]()
					{
						GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.State.UseParrying")));

						// Ÿ�̸� �ʱ�ȭ
						GetWorld()->GetTimerManager().ClearTimer(ParryingEndHandle);
					}), InDelayTime, false); 
			}
		}
		if (GetAbilitySystemComponent()->GetTagCount(FGameplayTag::RequestGameplayTag(FName("Player.State.UseBlock"))) <= 0)
		{
			MakeTagAndActive("Player.Block");
		}
	}
}

// ��� ����
void APlayerCharacter::BlockEnd()
{
	if (GetAbilitySystemComponent()->GetTagCount(FGameplayTag::RequestGameplayTag(FName("Player.State.UseBlock"))) > 0)
	{
		CanUseParrying = false;
		GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.State.UseBlock")));
		ArmBarrier->BarrierOff();
	}
}

//�� ��ȯ
void APlayerCharacter::SwordSummons()
{
	if (GetAbilitySystemComponent()->GetTagCount(FGameplayTag::RequestGameplayTag(FName("Player.State.Weapon.Sword"))) <= 0)
	{
		if (!(GetCharacterMovement()->IsFalling()))
		{
			WeaponUnequip();
			MakeTagAndActive("Player.EquipWeapon.Sword");
		}
	}
}

// ���� �÷��̾� ���Ͽ� ����
void APlayerCharacter::WeaponEquip(FName EquipSocketName, AWeapon* Weapon)
{
	if (Weapon != nullptr)
	{
		CurrentWeapon = Weapon;
		if (CurrentWeapon->GetSoummonsParticle() == nullptr) return;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), CurrentWeapon->GetSoummonsParticle(), CurrentWeapon->GetActorTransform());

		if (CurrentWeapon->GetSoummonSound() == nullptr) return;
		UGameplayStatics::PlaySound2D(this, CurrentWeapon->GetSoummonSound());

		Weapon->SetOwner(this);
		Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, EquipSocketName);
	}
}

// �̹� ������ ���⸦ ���� �� �Ⱥ��̴� ������ �̵�
void APlayerCharacter::WeaponUnequip()
{
	if (CurrentWeapon != nullptr)
	{
		CurrentWeapon->SetOwner(CurrentWeapon);
		CurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
		CurrentWeapon->SetActorLocation(FVector(0.f));
	}
}

void APlayerCharacter::TakeDamageFromEnemy()
{
	FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponent()->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	FGameplayEffectSpecHandle SpecHandle;

	if (GetAbilitySystemComponent()->GetTagCount(FGameplayTag::RequestGameplayTag(FName("Player.State.UseBlock"))) > 0)
	{
		if (GetInstigator())
		{
			float BlockAbleRot = 360.f - FMath::Abs(GetActorRotation().Yaw - GetInstigator()->GetActorRotation().Yaw);
			bool CheakBlock = UKismetMathLibrary::BooleanOR(BlockAbleRot > 150.f, BlockAbleRot < -150.f);
			AEnemyCharacter* AttackedEnemy = Cast<AEnemyCharacter>(GetInstigator());

			if (!(BlockAbleRot < 130.f || BlockAbleRot > 230.f))
			{
				if (GetAbilitySystemComponent()->GetTagCount(FGameplayTag::RequestGameplayTag(FName("Player.State.UseParrying"))) > 0)
				{
					UE_LOG(LogTemp, Warning, TEXT("Parrying"));
					SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(HitEffects[2], 1, EffectContext);
					AttackedEnemy->TakeParrying();
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Block"));
					SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(HitEffects[1], 1, EffectContext);
				}
				
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Hit"));
				SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(HitEffects[0], 1, EffectContext);
			}
		}
		
	}
	else
	{
		GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.Attack.Combo1")));
		GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.Attack.Combo2")));
		GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.Attack.Combo3")));
		GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.Attack.Combo4")));
		GetAbilitySystemComponent()->CancelAbilities();

		SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(HitEffects[0], 1, EffectContext);
	}

	if (SpecHandle.IsValid())
	{
		FActiveGameplayEffectHandle GEHandle = GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void APlayerCharacter::EFCharge()
{
	FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponent()->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	FGameplayEffectSpecHandle SpecHandle;
	SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(HitEffects[3], 1, EffectContext);

	if (SpecHandle.IsValid())
	{
		FActiveGameplayEffectHandle GEHandle = GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void APlayerCharacter::LockOn()
{
	if (LockOnEnemy != nullptr) //���� ����
	{
		LockOnEnemy = nullptr;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		bUseLockOn = false;
	}
	else if (LockOnEnemy == nullptr) //���� �۵�
	{
		FVector Start = GetActorLocation();
		FVector End = GetActorLocation() + (UKismetMathLibrary::GetForwardVector(GetControlRotation()) * 500.f);
		TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
		TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(GetOwner());
		ActorsToIgnore.Add(ArmBarrier);
		ActorsToIgnore.Add(Sword);
		FHitResult OutHit;
		bool bResult;

		bResult = UKismetSystemLibrary::SphereTraceSingle
		(
			GetWorld(),
			Start,
			End,
			125.f,
			UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_GameTraceChannel4),
			false,
			ActorsToIgnore,
			EDrawDebugTrace::ForDuration,
			OutHit,
			true
		);

		if (bResult)
		{
			LockOnEnemy = Cast<AEnemyCharacter>(OutHit.GetActor());
			if (LockOnEnemy)
			{
				GetCharacterMovement()->bOrientRotationToMovement = false;
				GetCharacterMovement()->bUseControllerDesiredRotation = true;
				bUseLockOn = true;
			}
		}
	}
}