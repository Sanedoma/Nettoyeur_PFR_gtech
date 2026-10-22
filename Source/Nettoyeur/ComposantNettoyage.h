// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Blueprint/UserWidget.h"
#include "ComposantNettoyage.generated.h"

class ACharacter;
class UCameraComponent;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnProgressionNettoyageChanged,
	float,
	Progression
);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NETTOYEUR_API UComposantNettoyage : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<ACharacter> Personnage;

	UPROPERTY(BlueprintReadWrite, Category = "Interface")
	TObjectPtr<UUserWidget> WidgetInteraction;


	UPROPERTY(BlueprintAssignable, Category = "Nettoyage")
	FOnProgressionNettoyageChanged OnProgressionNettoyageChanged;


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

	UFUNCTION(BlueprintCallable, Category = "Interface")
	void DefinirWidgetInteraction(UUserWidget* Widget);

	UFUNCTION()
	void MettreAJourTexteInteraction(bool Afficher);

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
