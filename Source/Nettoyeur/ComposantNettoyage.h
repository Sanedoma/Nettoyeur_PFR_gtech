// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ComposantNettoyage.generated.h"

class ACharacter;
class UCameraComponent;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NETTOYEUR_API UComposantNettoyage : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<ACharacter> Personnage;

	UPROPERTY()
	TObjectPtr<UCameraComponent> CameraJoueur;
	// Sets default values for this component's properties
	UComposantNettoyage();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	UFUNCTION(BlueprintCallable, Category = "Nettoyage")
	void TenterNettoyage();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
