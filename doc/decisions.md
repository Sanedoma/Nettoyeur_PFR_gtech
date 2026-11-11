# Décisions d'architecture

> Tenu par l'orchestrateur. Une décision n'est modifiée que par une nouvelle entrée qui la remplace (jamais effacée).

## D001 — Version du moteur figée : UE 5.7.4 — 2026-11-11
**Décision :** le projet reste sur Unreal Engine 5.7.4 jusqu'à la livraison.
**Raison :** GDD §2.3 ; une migration en cours de projet risque de casser assets et code.

## D002 — Interactions : composant de profil + enum, à la place d'une interface `IInteractable` — 2026-11-11 (constat, à confirmer)
**Constat :** le code existant utilise `UInteractionProfileComponent` (liste `AllowedInteractions` d'`EInteractionType`) attaché à chaque acteur interactif.
**Décision :** on conserve ce mécanisme comme « mécanisme équivalent validé » au sens du GDD §10.1. Les objets déclarent les interactions qu'ils acceptent dans leur profil ; le personnage ne contient aucune logique propre à un type d'objet.
**À revoir si :** une interaction a besoin d'un comportement propre à chaque objet que le profil ne peut pas exprimer — on ajoutera alors une interface en complément, pas en remplacement.

## D003 — Logique en C++, configuration en Blueprint — 2026-11-11
**Décision :** les systèmes (interaction, preuves, score, inspection) sont écrits en C++. Les Blueprints sont des classes enfants qui règlent des valeurs, des meshes et des références.
**Raison :** les agents écrivent et compilent le C++ de façon fiable ; les graphes Blueprint ne sont pas scriptables proprement. Quand un graphe est indispensable, il est décrit nœud par nœud et monté par le développeur.

## D004 — Organisation en agents et documentation systématique — 2026-11-11
**Décision :** orchestrateur + agents `gameplay-cpp`, `editeur-contenu`, `qa`, `rendu` (voir `CLAUDE.md`). Toute action est documentée selon `doc/CONVENTION.md`.

## D005 — Une branche Git par tâche, validation humaine avant merge — 2026-11-11
**Décision :** `tache/T00X-nom` → QA → validation du développeur → merge dans `main`.
