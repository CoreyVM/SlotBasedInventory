// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
//Corey Veitch-McAllister 2019

#include "TPSHCharacter.h"
#include "PickUpActor.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine.h"


ATPSHCharacter::ATPSHCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

    this->isSprinting = false;
	this->currentHealth = maxHealth;
	this->currentHealth = FMath::Clamp<float>(currentHealth, 0, maxHealth);
	this->isAiming = false;
	this->knifeAiming = false;
	this->weaponEquipped = false;
	this->currentWeapon = EWeaponType::None;
	this->InvWeaponEq = nullptr;
	this->ItemBoxOpen = false;
	this->weaponName = ""; //Gives defualt value for weapon
	this->itemIndex = 0;
}

void ATPSHCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ATPSHCharacter::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ATPSHCharacter::Sprint);
	PlayerInputComponent->BindAction("Aiming", IE_Pressed, this, &ATPSHCharacter::Aiming);
	PlayerInputComponent->BindAction("Aiming", IE_Released, this, &ATPSHCharacter::Aiming);

	PlayerInputComponent->BindAxis("MoveForward", this, &ATPSHCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATPSHCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput); 
	PlayerInputComponent->BindAxis("TurnRate", this, &ATPSHCharacter::TurnAtRate); 
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ATPSHCharacter::LookUpAtRate);
}

void ATPSHCharacter::TurnAtRate(float Rate) {
	if(!ItemBoxOpen)
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ATPSHCharacter::LookUpAtRate(float Rate){
	if (!ItemBoxOpen)
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ATPSHCharacter::MoveForward(float Value) {
	if ((Controller != NULL) && (Value != 0.0f) && !ItemBoxOpen)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
		forwardValue = Value;
	}	
}

void ATPSHCharacter::MoveRight(float Value) {
	if ( (Controller != NULL) && (Value != 0.0f) && !ItemBoxOpen)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}

}

void ATPSHCharacter::Sprint() {
	if (!isSprinting && !isAiming && forwardValue >= 1) //Checks to see if the player is not sprintng and aiming
	{
		isSprinting = true;
		GetCharacterMovement()->MaxWalkSpeed = 350;
	}
	else if (isSprinting) {
		GetCharacterMovement()->MaxWalkSpeed = 200;
		isSprinting = false;
	}
}

void ATPSHCharacter::Aiming() {
	if (!ItemBoxOpen && !isSprinting) {
		if (currentWeapon == EWeaponType::None) {if (!knifeAiming) knifeAiming = true;
		else if (knifeAiming) knifeAiming = false; }
	}
	if (!isAiming) { isAiming = true; }
	else isAiming = false;
}
	
void ATPSHCharacter::CheckForItem(APickUpActor* item) {
	for (int i = 0; i < _inventory.Num(); i++)
	{
		if (_inventory[i]->GetName() == item->GetName() && !_inventory[i]->GetStackFull() && item->itemType == _inventory[i]->itemType) {
			ObjectFound = true;
			AvailableStackFound = true;
			break;
		}
		else {
			ObjectFound = false;
			AvailableStackFound = false;
		}
	}
}

void ATPSHCharacter::GetAmmoInInventory() {
	if (InvWeaponEq != nullptr)
	{
		if (InvWeaponEq->GetName() == weaponName) //Checks the ammo of the current weapon equipped
		{
			int nonFullStackAmount = 0;
			int fullStackAmount = 0;
			StackAmountToAdd = 0; //Reset these varaibles so it wont count any additional ammo value from the inventroy
			FullStackSize = 0;

			for (int i = 0; i < _inventory.Num(); i++) {
				if (_inventory[i]->itemType == ItemType::Ammo && InvWeaponEq->AmmoType == _inventory[i]->AmmoType && !_inventory[i]->GetStackFull())//Looks for non full stacks
				{
					nonFullStackAmount += _inventory[i]->GetCurrentStackSize();
					StackAmountToAdd = nonFullStackAmount;
				}
			}
			for (int x = 0; x < _inventory.Num(); x++) {
				if (_inventory[x]->itemType == ItemType::Ammo &&  _inventory[x]->AmmoType == InvWeaponEq->AmmoType && _inventory[x]->GetStackFull())
				{
					fullStackAmount += _inventory[x]->GetMaxStackSize();
					FullStackSize = fullStackAmount;
				}
			}
		  ammoToAddInReserve = nonFullStackAmount + FullStackSize;
		}
		else if (InvWeaponEq->GetName() != weaponName)
		ammoToAddInReserve = 0;
	}
}

void ATPSHCharacter::AddStackableItemToInventory(APickUpActor* item) {
	if (itemInInvetory < sizeOfInventory)
	{
		if (item->GetCurrentStackSize() < item->GetMaxStackSize()) //Adds a new stack into the inventory 
		{
			int ammoInReserve = item->GetAmmoInReserve();
			_inventory.Add(item);
			itemInInvetory++;
			item->AddItemToPlayerInventory(item);
			item->SetCurrentStackSize(item->GetCurrentStackSize() + item->GetAmountToAdd());
			item->SetAmmoInReserve(ammoInReserve += amountToAdd);
			item->SetStackFull(false);
			AvailableStackFound = true;
			CantFindCurrentStack = false;
			GetAmmoInInventory();
			UpdateInventory();
			return;
		}
		else if (item->GetCurrentStackSize() >= item->GetMaxStackSize()) //Makes a new stack if the curent item is over the limit
		{
			_inventory.Add(item);
			item->AddItemToPlayerInventory(item);
			itemInInvetory++;
			item->SetCurrentStackSize(item->GetCurrentStackSize() + amountToAdd);
			item->SetStackFull(false); 
			AvailableStackFound = true; 
			UpdateInventory();
			return;
		}
		GEngine->AddOnScreenDebugMessage(1, 3, FColor::White, "Add a new stack of this object");
	}
}

void ATPSHCharacter::AddItemToStack(APickUpActor* item) {
	for (int i = 0; i < _inventory.Num(); i++) {
		if (_inventory[i]->AmmoType == item->AmmoType) {
			if (_inventory[i]->GetCurrentStackSize() < _inventory[i]->GetMaxStackSize()) { //Add more stacks to this item if there is space
				int AmountOfSpaceLeft = _inventory[i]->GetMaxStackSize() - _inventory[i]->GetCurrentStackSize();
				int amountToAdd = item->GetAmountToAdd();
				int OverFlowAmount = amountToAdd - AmountOfSpaceLeft;
				if (_inventory[i]->GetCurrentStackSize() + amountToAdd > _inventory[i]->GetMaxStackSize()) { //If there is an overspill create a new item with a reduced amount of stacks
					_inventory[i]->SetCurrentStackSize(_inventory[i]->GetMaxStackSize());
					_inventory[i]->SetStackFull(true);
					item->SetCurrentStackSize(OverFlowAmount);
					item->SetStackFull(false);
					item->AddItemToPlayerInventory(item);
					_inventory.Add(item);
			//		GEngine->AddOnScreenDebugMessage(1, 3, FColor::White, FString::FromInt(OverFlowAmount));
					GetAmmoInInventory();
					UpdateInventory();
					break;
				}
				else if (_inventory[i]->GetCurrentStackSize() + item->GetAmountToAdd() <= _inventory[i]->GetMaxStackSize()) {
					_inventory[i]->SetCurrentStackSize(_inventory[i]->GetCurrentStackSize() + item->GetAmountToAdd());
					item->Destroy();
			//		GEngine->AddOnScreenDebugMessage(1, 3, FColor::White, "Add some more stacks to this item");

					if (_inventory[i]->GetCurrentStackSize() == _inventory[i]->GetMaxStackSize()) {
						_inventory[i]->SetStackFull(true);
			//			GEngine->AddOnScreenDebugMessage(1, 3, FColor::White, "Destroy this item after adding it to the available stack");
						item->Destroy();
						break;
					}
					break;
				}
			}

		}
	}
	GetAmmoInInventory();
	UpdateInventory();
}

void ATPSHCharacter::AddToInventory(APickUpActor* actor) {
	if (actor->GetCanStack())
	{
		if(!actor->GetStackFull())
		CheckForItem(actor);
		if (ObjectFound)
		{
			if (AvailableStackFound)
				AddItemToStack(actor);
			
			else if (!AvailableStackFound)
			{
				if (itemInInvetory < sizeOfInventory) 
				AddStackableItemToInventory(actor);
				
			}
		}
		else if (!ObjectFound) //If the object you are adding to the array is not currently inside the array then add a new object to the array
		AddStackableItemToInventory(actor);
		
	}
	else if (!actor->GetCanStack()) //If the object is not stackble run this
	{
		if (itemInInvetory < sizeOfInventory)
		{
			_inventory.Add(actor);
			actor->AddItemToPlayerInventory(actor);
			UpdateInventory();
			itemInInvetory++;
			GEngine->AddOnScreenDebugMessage(1, 3, FColor::White, "Added a non stackbable item to the inventory");
			return;
		}
	}
}

void ATPSHCharacter::UpdateInventory() {
	OnUpdateInventory.Broadcast(_inventory);
}

void ATPSHCharacter::UpdateItemBox() {
	OnUpdateItemBox.Broadcast(itemBoxInventory);
}

void ATPSHCharacter::PrintInventory(APickUpActor* actor) {
	FString sInventory = "";
	for (APickUpActor* actor : _inventory)
	{
		sInventory.Append(actor->GetName());
		sInventory.Append(" | ");	
	}
}

void ATPSHCharacter::UseItem(APickUpActor* item) {
	if (item != nullptr)
	{
		switch (item->itemType)
		{
		case ItemType::KeyItem:
			GEngine->AddOnScreenDebugMessage(1, 3, FColor::White, "This is a key item");
			break;
		case ItemType::Weapon:
		 if (!weaponEquipped){ EquipWeapon(item); GetAmmoInInventory(); break; }
		 else if (weaponEquipped) {
			 UnEquipWeapon(item);
		     GEngine->AddOnScreenDebugMessage(1, 3, FColor::White, "Un Equip The Weapon");
			 break;
		 }
	 	case ItemType::FirstAidSpray:
			this->currentHealth = this->maxHealth;
			this->currentHealth = FMath::Clamp<float>(this->currentHealth, 0, this->maxHealth);
			DiscardItem(item);
			break;
		
		case ItemType::GreenHerb:
			this->currentHealth = this->currentHealth + 25;
			this->currentHealth = FMath::Clamp<float>(this->currentHealth, 0, this->maxHealth);
			DiscardItem(item);
			break;
		case ItemType::GreenRedHerb:
			this->currentHealth = this->maxHealth;
			this->currentHealth = FMath::Clamp<float>(this->currentHealth, 0, this->maxHealth);
			DiscardItem(item);
			break;
		case ItemType::DoubleGreenHerb:
			this->currentHealth = this->currentHealth + 40;
			this->currentHealth = FMath::Clamp<float>(this->currentHealth, 0, this->maxHealth);
			DiscardItem(item);
			break;
		case ItemType::MaxGreenHerb:
			this->currentHealth = this->maxHealth;
			this->currentHealth = FMath::Clamp<float>(this->currentHealth, 0, this->maxHealth);
			DiscardItem(item);
			break;
		default:
			break;
		}
	}
}

void ATPSHCharacter::DiscardItem(APickUpActor* item) {
	itemInInvetory--;
	_inventory.RemoveSingle(item);
	item->Destroy();
	UpdateInventory();
	GetAmmoInInventory();
}

void ATPSHCharacter::RemoveItemFromInventory(APickUpActor* item) {
	for (int i = 0; i < _inventory.Num(); i++)
	{
		if (item->GetCanStack())
		{
			if (_inventory.Contains(item) && item->GetCurrentStackSize() <= 1)
			{
				_inventory.RemoveSingle(item); //Removes the first element found in the array search 
				item->Destroy();
				itemInInvetory--;
				GEngine->AddOnScreenDebugMessage(1, 3, FColor::White, "Removed the inventory item at the current index");
				break;
			}
		}

		else if (!item->GetCanStack()) {
			if (_inventory.Contains(item)) {
				_inventory.RemoveSingle(item); //Removes the first element found in the array search 
				item->Destroy();
				itemInInvetory--;
				GEngine->AddOnScreenDebugMessage(1, 3, FColor::White, "Removed the inventory item at the current index");
				break;
			}
		}
	}
	UpdateInventory();
	GetAmmoInInventory();
}

void ATPSHCharacter::EquipWeapon(APickUpActor* item) {
	if (item != NULL)
	{
		AmmoReloaded = false;
		weaponEquipped = true;
		item->ItemMesh->SetVisibility(true);
		item->ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		switch (item->weaponType)
		{
		case ETypeOfWeapon::None:
			GEngine->AddOnScreenDebugMessage(1, 3, FColor::White, "This is not a weapon");
			break;
		case ETypeOfWeapon::Handgun:
			GEngine->AddOnScreenDebugMessage(1, 3, FColor::White, "Equip The Handgun");
			weaponName = item->GetName();
			currentWeapon = EWeaponType::Handgun;
			InvWeaponEq = item;
			GEngine->AddOnScreenDebugMessage(1, 3, FColor::White, weaponName);
			GetAmmoInInventory();
			break;
		case ETypeOfWeapon::Rifle:
			GEngine->AddOnScreenDebugMessage(1, 3, FColor::White, "Equip The Rifle");
			InvWeaponEq = item;
			break;
		case ETypeOfWeapon::RocketLauncher:
			GEngine->AddOnScreenDebugMessage(1, 3, FColor::White, "Equip The Rocket Launcher");
			break;
		case ETypeOfWeapon::Shotgun:
			GEngine->AddOnScreenDebugMessage(1, 3, FColor::White, "Equip The Shotgun");
			currentWeapon = EWeaponType::Shotgun;
			weaponName = item->GetName();
			InvWeaponEq = item;
			GetAmmoInInventory();
			break;
		}
	}
}

void ATPSHCharacter::UnEquipWeapon(APickUpActor* item)
{
	AmmoReloaded = false;
	ammoToAddInReserve = 0;
	weaponEquipped = false;
	currentWeapon = EWeaponType::None;
	item->ItemMesh->SetVisibility(false);
	item->ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UpdateInventory();
}

bool ATPSHCharacter::LookForNonFullStack() {
	for (int i = 0; i < _inventory.Num(); i++) { //Use a backwards for loop 
		if (_inventory[i]->itemType == ItemType::Ammo && _inventory[i]->AmmoType == InvWeaponEq->AmmoType && !_inventory[i]->GetStackFull()) {
			RemoveAmmoFromStack(_inventory[i]);
			return true;
			break;
		}
	}
	return false;
}

void ATPSHCharacter::LookForFullStack() {
	for (int i = 0; i < _inventory.Num(); i++) { //Use a backwards for loop 
		if (_inventory[i]->itemType == ItemType::Ammo && _inventory[i]->AmmoType == InvWeaponEq->AmmoType && _inventory[i]->GetStackFull()) {
			RemoveAmmoFromStack(_inventory[i]);
			break;
		}
	}
}

void ATPSHCharacter::ReloadWeapon() {
	if (InvWeaponEq != nullptr && InvWeaponEq->GetAmmoInMag() < InvWeaponEq->GetSizeOfMagazine()) {
		if (LookForNonFullStack()) {return; }
		else LookForFullStack();
	}
}

void ATPSHCharacter::RemoveAmmoFromStack(APickUpActor* item) {
	int ammoToRemove = InvWeaponEq->GetSizeOfMagazine() - InvWeaponEq->GetAmmoInMag();
	
	if (ammoToRemove <= item->GetCurrentStackSize()) { //If the ammo to be added is less than the total amount of the stack
		item->SetCurrentStackSize(item->GetCurrentStackSize() - ammoToRemove);
		InvWeaponEq->SetAmmoInMag(InvWeaponEq->GetSizeOfMagazine());
		item->SetStackFull(false);
		if (item->GetCurrentStackSize() <= 0) { _inventory.RemoveSingle(item); item->Destroy(); UpdateInventory();}
		GetAmmoInInventory();
		return;
	}
	else if (ammoToRemove > item->GetCurrentStackSize()) {  //Destroy the ammo from the inventory
		InvWeaponEq->SetAmmoInMag(InvWeaponEq->GetAmmoInMag() + item->GetCurrentStackSize());
	//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, FString::FromInt(ammoToRemove));
		_inventory.RemoveSingle(item);
		item->Destroy();
		itemInInvetory--;
		GetAmmoInInventory();
		UpdateInventory();
		if (ammoToAddInReserve > 0 && InvWeaponEq->GetAmmoInMag() != InvWeaponEq->GetSizeOfMagazine()) { ReloadWeapon(); }
	//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "Add destroyed ammo  to mag");
		return;
	}
}

FString ATPSHCharacter::CombineDuplicateItems(APickUpActor* item1, APickUpActor* item2) {
	FString result = "";
	if (item1 != nullptr && item2 != nullptr) {
		if (item1->itemType == item2->itemType) {
			switch (item1->itemType) {
			case ItemType::GreenHerb:
				RemoveCombinedItems(item1, item2);
				return "Double Green";
				break;
			case ItemType::RedHerb:
				return "Red Herb";
				break;
			case ItemType::Ammo:
				if (item1->GetCurrentStackSize() >= item1->GetMaxStackSize() || item2->GetCurrentStackSize() >= item2->GetMaxStackSize()) { return "nothing"; }
				else {
					AddItemToStack(item2);
					return "nothing";
				}
			}
		}
		else if (item1->itemType != item2->itemType) {
			result = CombineItems(item1, item2); //If the two items are not duplicates then run this function
			return result;
		}
	}
	return "nothing";
}

FString ATPSHCharacter::CombineItems(APickUpActor* item1, APickUpActor* item2) {
	switch (item1->itemType) {
	case ItemType::GreenHerb:
		if (item2->itemType == ItemType::RedHerb) {
			RemoveCombinedItems(item1, item2);
			return "Green+Red";
			break;
		}
		if (item2->itemType == ItemType::DoubleGreenHerb) {
			RemoveCombinedItems(item1, item2);
			return "MaxGreenHerb";
			break;
		} 
		break;
	case ItemType::RedHerb:
		if (item2->itemType == ItemType::GreenHerb) { RemoveCombinedItems(item1, item2);  return "Green+Red"; break; }
		//Implement Blue Herb Here
	case ItemType::DoubleGreenHerb:
		if (item2->itemType == ItemType::GreenHerb) { RemoveCombinedItems(item1, item2); return "MaxGreenHerb"; break; }
	default:
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "You cannot combine these two items");
		return "nothing";
		break;
	}
	return "nothing";
}

void ATPSHCharacter::RemoveCombinedItems(APickUpActor* item1, APickUpActor* item2) {
	_inventory.RemoveSingle(item1);
	_inventory.RemoveSingle(item2);
	itemInInvetory--;
	itemInInvetory--;
	item1->Destroy();
	item2->Destroy();
	UpdateInventory();
}

void ATPSHCharacter::PlaceInventoryItemToItemBox(APickUpActor* item) {
	if (itemInItemBox < sizeOfItemBox) {
		itemBoxInventory.Add(item);
		itemInItemBox++;
		itemInInvetory--;
		_inventory.RemoveSingle(item);
	}
	UpdateItemBox();
	UpdateInventory();
	GetAmmoInInventory();
}

void ATPSHCharacter::PlaceItemFromBoxToInventory(APickUpActor* item) {
	if (_inventory.Num() < sizeOfInventory) {
		_inventory.Add(item);
		itemBoxInventory.Remove(item);
		itemInItemBox--;
		itemInInvetory++;
		UpdateItemBox();
		UpdateInventory();
		GetAmmoInInventory();
	}
}