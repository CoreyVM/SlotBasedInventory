// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
//Corey Veitch-McAllister 2019

#pragma once

#include "CoreMinimal.h"
#include "PickUpActor.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "TPSHCharacter.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateInventoryDelegate, const TArray<class APickUpActor*>&, InventoryItems);

UENUM(BlueprintType)
enum class EWeaponType : uint8 { None, Handgun, Shotgun, Rifle, RocketLauncher };

UCLASS(config = Game)
class ATPSHCharacter : public ACharacter
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	ATPSHCharacter();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		void UseItem(APickUpActor* item);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		void RemoveItemFromInventory(APickUpActor* item);

protected:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void DiscardItem(APickUpActor* item);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	bool weaponEquipped;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	bool Attacking = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Health")
	float currentHealth;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Health")
	float maxHealth = 100;

    #pragma region Inventory Variables
	//Inventory Arrays
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
		TArray<class APickUpActor*> _inventory; //The standard size of the inventory is 12
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
		TArray<class APickUpActor*> itemBoxInventory; //The standard size of the inventory is 12
	UPROPERTY()
		int sizeOfInventory = 12;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
		int itemInInvetory = 0;
	UPROPERTY()
		int sizeOfItemBox = 16;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
		int itemInItemBox = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
		int itemIndex;
#pragma endregion

    #pragma region Combine Item Functions
	//Combine Items Functions
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		FString CombineDuplicateItems(APickUpActor* item1, APickUpActor* item2);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		FString CombineItems(APickUpActor* item1, APickUpActor* item2);
	UFUNCTION()
		void RemoveCombinedItems(APickUpActor* item1, APickUpActor* item2);
#pragma endregion

    #pragma region ItemBox Functions
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		void PlaceInventoryItemToItemBox(APickUpActor* item);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		void PlaceItemFromBoxToInventory(APickUpActor* item);
#pragma endregion

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement")
	bool isSprinting;
	void Sprint();

	UFUNCTION(BlueprintCallable, Category = "Aiming")
		void Aiming();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
		bool isAiming;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
		bool ItemBoxOpen;


	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

	//Enums
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Enum)
		EWeaponType currentWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
		FString weaponName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		bool knifeAiming;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float forwardValue;

    #pragma region Inventory Stacking Variables/Functions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		int StackAmountToAdd = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		int FullStackSize = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		int ammoIndex = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reloading")
		bool CantFindCurrentStack = false;

	void CheckForItem(APickUpActor* item);
	void AddStackableItemToInventory(APickUpActor* item);
#pragma endregion

private:
	UFUNCTION()
	void AddItemToStack(APickUpActor* item);
	void RemoveAmmoFromStack(APickUpActor* item);

	bool ObjectFound = false;
	bool AvailableStackFound = false;
	bool hasChecked = false;
	bool AlreadyAdded = false;
	bool AmmoReloaded = false;

public:
	//Inventory Functions
	UFUNCTION(BlueprintCallable)
		void AddToInventory(APickUpActor* actor);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debugging")
		int amountToAdd;
	UFUNCTION(BlueprintCallable)
		void UpdateInventory();
	UFUNCTION(BlueprintCallable)
		void UpdateItemBox();

	UPROPERTY(BlueprintAssignable, Category = "Pickup")
		FUpdateInventoryDelegate OnUpdateInventory;
	UPROPERTY(BlueprintAssignable, Category = "Pickup")
		FUpdateInventoryDelegate OnUpdateItemBox;

	UFUNCTION(BlueprintCallable)
		void PrintInventory(APickUpActor* actor);
	UFUNCTION(BlueprintCallable)
		void EquipWeapon(APickUpActor* item);
	UFUNCTION(BlueprintCallable)
		void UnEquipWeapon(APickUpActor* item);

	UFUNCTION()
	bool LookForNonFullStack();
	void LookForFullStack();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PickUp")
		bool InRangeOfItem = false;
	UFUNCTION(BlueprintCallable, Category = "Ammo")
		void GetAmmoInInventory();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")//Used to add the amount of ammo needed in reserve from the inventory
		int ammoToAddInReserve;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
		APickUpActor* InvWeaponEq;
	UFUNCTION(BlueprintCallable, Category = "Combat")
		void ReloadWeapon();

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

