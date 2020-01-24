// Fill out your copyright notice in the Description page of Project Settings.
//Corey Veitch-McAllister 2019

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPSHCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "PickUpActor.generated.h"

UENUM(BlueprintType)
enum class ItemType : uint8{
KeyItem, Weapon, FirstAidSpray, 
GreenHerb, RedHerb, DoubleGreenHerb, GreenRedHerb, MaxGreenHerb,
Ammo, InkRibbon };


UENUM(BlueprintType)
enum class ETypeOfWeapon : uint8 { None, Handgun, Shotgun, Rifle, RocketLauncher };

UENUM(BlueprintType)
enum class EAmmoType : uint8 { None, Handgun, Shotgun, Magnum, Rifle, Rockets };

UCLASS()
class APickUpActor : public AActor
{
	GENERATED_BODY()
public:	
	APickUpActor();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float FireRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
		bool pickedUp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool canUse;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ZoomInMin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	    float ZoomInMax;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
		float Damage;

    #pragma region Ammo Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
		int AmmoInMag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
		int AmmoInReserve;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
		int SizeOfMagazine;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
		int AmmoDifference = 0;
#pragma endregion

    #pragma region Stacking & Combining Variables
	//Stacking Varaibles
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool stackChecked;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool combineChecked;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool canStack;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	    bool canDiscard;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool canCombine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int MaxStackSize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int currentStackSize = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int amountToAdd = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool stackFull = false;
#pragma endregion

    #pragma region Item Description Variables
	//Item Information Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString itemDesc;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* Image;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName SocketName;
#pragma endregion

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    #pragma region Get Ammo Variables
	UFUNCTION()
		int GetAmmoInMag() const { return AmmoInMag; };
	void SetAmmoInMag(int value) { AmmoInMag = value; }
	UFUNCTION()
		int GetAmmoInReserve()const { return AmmoInReserve; }
	    void SetAmmoInReserve(int value) { AmmoInReserve = value; }
	UFUNCTION() 
		int GetSizeOfMagazine() const { return SizeOfMagazine; }
	UFUNCTION()
		int GetAmmoDifference() const { return AmmoDifference; }
	UFUNCTION()
		void SetAmmoDifference(int value) { AmmoDifference = value; }
#pragma endregion

	UFUNCTION()
		int GetMaxStackSize() { return MaxStackSize; }
	    bool GetCanDiscard() { return canDiscard; }
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
		bool buttonPressed;
	UFUNCTION()
		bool GetCanCombine() { return canCombine; }

	UFUNCTION()
	bool GetStackChecked() { return stackChecked; }
	void SetStackChecked(bool value) { stackChecked = value; }

	bool GetCanStack() { return canStack; }
	//int GetMaxStackSize()  { return MaxStackSize; }
	int GetAmountToAdd()  { return amountToAdd; }

	int GetCurrentStackSize()  { return currentStackSize; }
	void SetCurrentStackSize(int value) { currentStackSize = value; }

	bool GetStackFull()  { return stackFull; }
	void SetStackFull(bool value) { stackFull = value; }

	FString GetName() { return Name; }

	//Component Varaibles
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USceneComponent* SceneComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* ItemMesh;
	UPROPERTY(EditAnywhere)
	UBoxComponent* boxCollider;

	//Enums 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Enum) 
		ItemType itemType;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Enum)
		ETypeOfWeapon weaponType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Enum)
		EAmmoType AmmoType;
	UPROPERTY()
		bool AmmoAdded = false;

	virtual void Show(bool visible);
	virtual void OnInteract();

	void AddItemToPlayerInventory(APickUpActor * item);

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


};