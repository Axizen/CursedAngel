// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "CurseWeaponComponent.h"
#include "CursedAngelComponent.h"
#include "Components/ActionComponent.h"
#include "Components/CursedAngelCameraComponent.h"
#include "Animation/CursedAngelAnimInstance.h"
#include "CursedAngelCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UCurseWeaponComponent;
class UCursedAngelComponent;
class UStyleComponent;
class AFrankAI;
class UCharacterDataAsset;
class UHealthComponent;
class UAirDashComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 * ACursedAngelCharacter - Base character class for Cursed Angel game
 * 
 * This is the parent class for both ARipley and AFrankAI, providing shared functionality:
 * - Camera system (CameraBoom, FollowCamera)
 * - Input system (MappingContext, basic actions)
 * - Combat components (CurseWeaponComponent, CursedAngelComponent, StyleComponent)
 * - Combat functions (FireCurseWeapon, SwitchCurseWeapon, ActivateCursedAngel)
 * - Style system integration
 * 
 * Subclasses (ARipley, AFrankAI) should override virtual functions to customize behavior.
 */


UCLASS(config=Game)
class ACursedAngelCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCursedAngelCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Curse Weapon Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UCurseWeaponComponent* CurseWeaponComponent;

	/** Cursed Angel Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UCursedAngelComponent* CursedAngelComponent;

	/** Style Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UStyleComponent* StyleComponent;

	/** Health Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UHealthComponent* HealthComponent;

	/** Action Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Actions, meta = (AllowPrivateAccess = "true"))
	UActionComponent* ActionComponent;

public:
	/** Character Configuration Data Asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Configuration")
	UCharacterDataAsset* CharacterConfig;

public:
	ACursedAngelCharacter();

	/** Frank Companion Reference */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion")
	class AFrankAI* FrankCompanion;
	

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			

protected:
	virtual void BeginPlay() override;

	virtual void NotifyControllerChanged() override;


	/** Virtual functions for subclass customization */

	/** Called when any combat action is performed (firing, switching weapons, etc.) */
	UFUNCTION(BlueprintNativeEvent, Category = "Combat")
	void OnCombatAction();
	virtual void OnCombatAction_Implementation();

	/** Called when style rank changes */
	UFUNCTION(BlueprintNativeEvent, Category = "Combat")
	void OnStyleRankChanged(EStyleRank NewRank);
	virtual void OnStyleRankChanged_Implementation(EStyleRank NewRank);

	/** Called when cursed angel is activated */
	UFUNCTION(BlueprintNativeEvent, Category = "Combat")
	void OnCursedAngelActivated();
	virtual void OnCursedAngelActivated_Implementation();

	/** Called when cursed angel is deactivated */
	UFUNCTION(BlueprintNativeEvent, Category = "Combat")
	void OnCursedAngelDeactivated();
	virtual void OnCursedAngelDeactivated_Implementation();

	/** Protected helper functions for shared logic */

	/** Apply damage to target actor */
	void ApplyDamageToTarget(AActor* Target, float Damage);

	/** Check if can perform combat action */
	bool CanPerformCombatAction() const;

	/** Update style meter based on combat performance */
	void UpdateStyleMeter(float DeltaTime);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/** Combat Functions */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void FireCurseWeapon(ECurseWeaponType WeaponType);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SwitchCurseWeapon(ECurseWeaponType NewWeapon);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ActivateCursedAngel();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void RequestFrankUtility();

	/** Get current curse weapon */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	ECurseWeaponType GetCurrentCurseWeapon() const;

	/** Check if cursed angel can be activated */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool CanActivateCursedAngel() const;

	/** Called when an enemy is killed */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnEnemyKilled(AActor* Enemy, bool bEnvironmentalKill);

	/** Get the style component */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	UStyleComponent* GetStyleComponent() const;

	/** Get current style rank */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	EStyleRank GetCurrentStyleRank() const;

	/** Get style damage multiplier */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetStyleDamageMultiplier() const;

	/** Load stats from CharacterDataAsset */
	UFUNCTION(BlueprintCallable, Category = "Character Configuration")
	virtual void LoadStatsFromDataAsset();

	/** Initialize character from CharacterDataAsset */
	UFUNCTION(BlueprintCallable, Category = "Character Configuration")
	virtual void InitializeFromDataAsset();

	/** Execute action by name */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool ExecuteAction(FName ActionName);

	/** Queue action for combo chain */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void QueueAction(FName ActionName);

	/** Cancel current action */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void CancelCurrentAction();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	/** Returns CurseWeaponComponent subobject **/
	FORCEINLINE class UCurseWeaponComponent* GetCurseWeaponComponent() const { return CurseWeaponComponent; }
	/** Returns CursedAngelComponent subobject **/
	FORCEINLINE class UCursedAngelComponent* GetCursedAngelComponent() const { return CursedAngelComponent; }
	/** Returns StyleComponent subobject **/
	FORCEINLINE class UStyleComponent* GetStyleComponentInline() const { return StyleComponent; }
	/** Returns HealthComponent subobject **/
	FORCEINLINE class UHealthComponent* GetHealthComponent() const { return HealthComponent; }
	/** Returns ActionComponent subobject **/
	FORCEINLINE class UActionComponent* GetActionComponent() const { return ActionComponent; }

	/**
	 * Helper functions for new camera and animation instance integration.
	 * 
	 * NOTE: UCursedAngelCameraComponent should be set as the camera component class in Blueprint.
	 * NOTE: UCursedAngelAnimInstance (or a child Blueprint class) should be set as the Anim Class in the mesh component in Blueprint.
	 */

	/** Returns the CursedAngelCameraComponent if it exists */
	UFUNCTION(BlueprintPure, Category = "Camera")
	class UCursedAngelCameraComponent* GetCursedAngelCamera() const;

	/** Returns the CursedAngelAnimInstance if it exists */
	UFUNCTION(BlueprintPure, Category = "Animation")
	class UCursedAngelAnimInstance* GetCursedAngelAnimInstance() const;
};