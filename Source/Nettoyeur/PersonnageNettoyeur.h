// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InteractionComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "PersonnageNettoyeur.generated.h"

UCLASS()
class NETTOYEUR_API APersonnageNettoyeur : public ACharacter
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UInteractionComponent> InteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UPhysicsHandleComponent> PhysicsHandle;

	// Sets default values for this character's properties
	APersonnageNettoyeur();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual bool CanJumpInternal_Implementation() const override;


public:	
	UFUNCTION(BlueprintCallable, Category = "Nettoyage")
	void TenterNettoyage();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
