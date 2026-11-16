#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ComposantNettoyage.generated.h"

class UInteractionComponent;
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnProgressionNettoyageChanged,
	float,
	Progression
);

/**
 * Nettoyage des preuves visées. Ne vise pas lui-même : il lit la cible de
 * l'UInteractionComponent du même acteur au moment de l'appui.
 * Une cible est nettoyable si son profil d'interactions autorise Wipe.
 * Le prompt n'est plus piloté ici : UWidgetInteraction écoute directement la visée.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NETTOYEUR_API UComposantNettoyage : public UActorComponent
{
	GENERATED_BODY()

public:
	UComposantNettoyage();

	/**
	 * Widget du prompt, donné par le Blueprint via DefinirWidgetInteraction.
	 * Simple référence conservée : le C++ ne l'utilise plus (voir UWidgetInteraction).
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Interface")
	TObjectPtr<UUserWidget> WidgetInteraction;

	UPROPERTY(BlueprintAssignable, Category = "Nettoyage")
	FOnProgressionNettoyageChanged OnProgressionNettoyageChanged;

	/** Écrit la progression dans le journal (Output Log) à chaque essuyage. Faux par défaut. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nettoyage|Debug")
	bool bAfficherDebug = false;

	/** Essuie la cible visée si son profil autorise Wipe (touche E). */
	UFUNCTION(BlueprintCallable, Category = "Nettoyage")
	void TenterNettoyage();

	/**
	 * Mémorise le widget du prompt. Gardée pour ne pas casser le nœud du Blueprint
	 * du personnage ; le widget se met désormais à jour tout seul.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interface")
	void DefinirWidgetInteraction(UUserWidget* Widget);

protected:
	virtual void BeginPlay() override;

private:
	/** Composant de visée du même acteur, trouvé une seule fois au BeginPlay. */
	UPROPERTY(Transient)
	TObjectPtr<UInteractionComponent> ComposantInteraction;

	/** Vrai si une cible est visée et que son profil autorise Wipe. */
	bool PeutNettoyerCible() const;
};
