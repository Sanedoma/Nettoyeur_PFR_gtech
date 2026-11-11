# NETTOYEUR
## Game Design Document complet & plan de développement modulaire
**Titre provisoire — Sandbox systémique de nettoyage et de dissimulation de traces**  
**Moteur cible : Unreal Engine 5.7.4**  
**Format : projet solo, PC, vue à la première personne**  
**Version du document : 1.2 — 23 octobre 2026**

---

# 0. Comment utiliser ce document

Ce document fusionne les éléments de la Note d’intention, du GDD léger, du TDD d’architecture, du planning de neuf semaines et de la Boucle de Gameplay fournie. Il distingue explicitement :

- **Socle initial** : les éléments nécessaires pour produire un prototype cohérent et présentable.
- **Extension prioritaire** : les fonctionnalités qui enrichissent la boucle une fois le socle stable.
- **Vision long terme** : les systèmes plus coûteux à développer, à ne pas commencer avant d’avoir validé le jeu de base.

Les durées proposées ci-dessous sont des estimations de planification, pas des garanties. Le planning d’origine de neuf semaines est conservé comme cible académique compacte ; le parcours débutant Unreal présenté ensuite est plus progressif et peut prendre plusieurs mois selon le temps disponible.

**Règle d’or :** ne pas commencer un module tant que le précédent n’est pas jouable, testable et sauvegardé dans Git.

# 1. Résumé exécutif

## 1.1 Pitch

Le crime a déjà eu lieu. Le joueur est un nettoyeur professionnel appelé pour remettre une scène en état avant l’inspection. Il doit observer, manipuler, nettoyer, masquer ou déplacer des éléments, puis quitter les lieux ou se cacher avant que les enquêteurs ne découvrent trop de traces.

## 1.2 Genre et expérience

- **Genre :** sandbox systémique, simulation immersive, puzzle environnemental sous pression.
- **Caméra :** première personne.
- **Joueur :** nettoyeur, et non auteur du crime.
- **Promesse :** chaque action modifie la scène ; une action utile peut aussi créer une nouvelle trace.
- **Tension principale :** « Ai-je vraiment tout repéré ? Qu’est-ce que mon intervention a laissé derrière moi ? »
- **Expérience visée :** observation, expérimentation, planification, gestion du risque et satisfaction de voir une scène redevenir cohérente.

Le projet doit privilégier la lisibilité des systèmes et la qualité des réactions du monde plutôt que le nombre de niveaux ou la quantité de contenu.

## 1.3 Piliers de design

1. **Tout est trace.** Les éléments pertinents sont mesurables : sang, empreintes, objet déplacé, porte ouverte, corps visible, vitre altérée, etc.
2. **Action = réaction.** Nettoyer, déplacer ou cacher peut produire des conséquences secondaires.
3. **Lisibilité.** Le joueur dispose d’indices visuels et d’un mode de détection pour comprendre ce qui reste.
4. **Plusieurs solutions.** Une trace peut être nettoyée, masquée, déplacée ou rendue inaccessible à l’inspection, avec des coûts et des risques différents.
5. **Le monde garde mémoire.** La scène compare son état courant à son état de référence et conserve l’historique des actions pertinentes.
6. **La pression est graduelle.** Le risque augmente selon le temps, le bruit, la visibilité, le poids transporté et les traces restantes.
7. **Architecture modulaire.** Un nouvel objet ou type de trace doit pouvoir être ajouté sans réécrire le personnage ou le calcul du score.

## 1.4 Objectif de la première version

Livrer un niveau dense et entièrement jouable : un petit appartement, une scène préparée, un corps représenté par un proxy physique simple, cinq types de traces, trois outils, un mode de détection et une phase d’inspection avec score final.

La conduite libre, la police aléatoire, les inspecteurs autonomes, les pièges complexes et une grande boutique ne sont pas requis pour valider cette première version.

# 2. Vision du produit et périmètre

## 2.1 Version cible par étapes

### Version A — Prototype fondamental
- Déplacement FPS, regard et accroupissement.
- Interaction générique.
- Saisie et déplacement d’objets physiques.
- Traces mesurables et score de détection.
- Nettoyage simple.
- Un contenant permettant de cacher un objet ou le proxy du corps.
- Un niveau test et un écran de résultat.

### Version B — Démo verticale
- Appartement complet et décor narratif.
- Sang statique et traînée générée par déplacement du corps.
- Lampe UV / mode focus.
- VFX et sons de nettoyage.
- Inspection présentée par une séquence cinématique.
- Écran final expliquant les traces détectées.

### Version C — Sandbox étendu
- Van comme base de mission et inventaire.
- Choix des outils avant la mission.
- Déplacement de meubles, gestion de vitres et textiles.
- Temps, bruit, visibilité et risque de rencontre.
- Plusieurs solutions de dissimulation et objectifs secondaires.

### Version D — Jeu avancé
- Conduite du van.
- Quartier extérieur et zone de repli.
- Contrôles de police et événements aléatoires.
- Caméras de surveillance.
- Inspecteurs avec perception et navigation.
- Pièges, distractions et système de poursuite.
- Progression, boutique, tenues et missions supplémentaires.

## 2.2 Hors-scope du premier prototype

Ne pas développer au départ : multijoueur, sauvegarde de campagne, monde ouvert, IA d’enquête en temps réel, conduite réaliste, système de police complet, ragdoll complexe, économie détaillée, grande variété de matériaux et plusieurs niveaux.

Ces éléments sont des extensions possibles, pas des prérequis.

## 2.3 Plateforme et cible technique

- Unreal Engine 5.7.4, version utilisée actuellement pour le projet.
- PC Windows comme cible initiale.
- Contrôleur clavier/souris.
- Blueprints pour prototyper et assembler le contenu ; C++ pour les bases réutilisables lorsque les notions sont comprises et que le système le justifie.
- Git + Git LFS pour le versionnage des assets binaires.
- Priorité à une scène stable et à une fréquence d’image régulière plutôt qu’à un rendu spectaculaire coûteux.

La version exacte d’Unreal doit être installée et conservée pendant le projet. Éviter de migrer de version en plein développement sans raison.

# 3. Boucle de jeu

## 3.1 Boucle principale d’une mission

1. **Préparer.** Lire le briefing, les pièces concernées, les outils disponibles et les risques connus.
2. **Observer.** Faire un premier tour, repérer les traces évidentes et les éléments anormaux.
3. **Planifier.** Décider de l’ordre des actions et des objets à déplacer ou à traiter.
4. **Intervenir.** Nettoyer, saisir, déplacer, cacher, refermer ou remplacer des éléments.
5. **Générer des conséquences.** Un objet déplacé peut laisser une empreinte ; traîner le corps peut créer une traînée ; casser un vase peut ajouter des débris et du bruit.
6. **Vérifier.** Utiliser la lampe UV ou le mode focus pour rechercher les traces restantes.
7. **Décider.** Déclencher l’inspection ou continuer à prendre des risques.
8. **Résoudre.** L’inspection révèle les traces visibles, calcule le score et produit un rapport.
9. **S’échapper ou se cacher.** Dans le socle initial, cette phase est abstraite ou cinématique ; dans les versions avancées, elle devient une séquence jouable.

## 3.2 Boucle de 30 secondes

- Repérer une trace.
- Choisir l’outil ou l’action.
- Exécuter l’action en visant correctement.
- Lire le retour visuel/sonore.
- Vérifier si la trace a été neutralisée et si une nouvelle trace est apparue.

Chaque action doit communiquer clairement son résultat. Une action sans feedback ne doit pas être la norme.

## 3.3 Exemple de décision systémique

Le joueur remarque une flaque de sang. Il peut :
- la nettoyer avec le bon outil : efficace, mais prend du temps ;
- déplacer un tapis dessus : masque la trace, mais le tapis devient suspect ;
- déplacer le corps : peut résoudre un problème de visibilité, mais génère une traînée ;
- quitter la pièce : rapide, mais la trace reste comptabilisée.

Aucune solution n’est automatiquement parfaite : chacune implique un compromis entre temps, risque et score.

# 4. Structure d’une mission

## 4.1 Avant la mission

Le menu de démarrage comprend, à terme :
- Nouvelle mission / continuer.
- Carte des niveaux.
- Boutique d’équipement.
- Tenues cosmétiques.
- Options.
- Tutoriel / commandes.
- Crédits et quitter.

Pour le prototype, un simple menu « Jouer / Options / Quitter » suffit. La boutique et la carte seront ajoutées seulement quand plusieurs missions et des coûts d’équipement existeront.

## 4.2 Briefing

Le briefing indique :
- le lieu et la zone à traiter ;
- les pièces incluses dans l’inspection ;
- le matériel disponible dans le van ;
- les contraintes éventuelles : temps, bruit, accès limité ou surveillance ;
- les objectifs principaux et facultatifs.

Le briefing ne doit pas donner automatiquement la liste exacte de toutes les preuves. Le plaisir d’observation dépend du fait que le joueur découvre lui-même une partie des problèmes.

## 4.3 Préparation du matériel

Inventaire de la vision complète :
- serpillière et produit de sol ;
- aspirateur ;
- gants ;
- sacs ;
- lampe UV ;
- produits pour vitres ;
- peinture ;
- nettoyant textile ;
- déshodorisant ;
- outils spécifiques à des missions.

Pour le prototype, retenir trois outils clairement distincts :
1. **Outil de nettoyage** : réduit l’intensité des traces nettoyables.
2. **Lampe UV / focus** : révèle les traces, sans les supprimer.
3. **Sac ou contenant** : permet de neutraliser ou masquer certains objets selon les règles du jeu.

Les outils doivent avoir des usages lisibles. Ne pas ajouter un outil si son fonctionnement est identique à celui d’un outil déjà présent.

## 4.4 Déroulement sur place

Le joueur entre dans les pièces, observe, interagit avec les objets, puis vérifie le résultat. Les actions sont physiques lorsque cela apporte quelque chose au gameplay ; les tâches répétitives peuvent utiliser une interaction contextualisée plutôt qu’une simulation détaillée.

## 4.5 Fin de mission

Deux issues fondamentales sont définies dans la boucle fournie :
- **Réussite :** atteindre la zone de repli sans être intercepté, ou rester caché jusqu’à la fin de l’inspection.
- **Échec :** être repéré par un policier ou un inspecteur, laisser trop d’indices, ou être attrapé à l’extérieur.

Dans le prototype, la sortie peut être un déclencheur de fin et l’inspection une phase calculée. Les poursuites et contrôles routiers attendent une version ultérieure.

# 5. Système central : les traces et les preuves

## 5.1 Modèle de donnée

Chaque trace possède au minimum :
- **Type** : sang, empreinte, objet déplacé, corps, ouverture, textile, vitre, bruit, etc.
- **Gravité** : importance de la trace dans le score.
- **Intensité** : valeur de 0.0 à 1.0.
- **État** : active, neutralisée ou masquée.
- **Visibilité** : visible directement, visible seulement avec outil, ou non visible à l’inspection.
- **Objet ou surface d’origine** : l’acteur ou la zone à laquelle la trace est associée.
- **Règles d’action** : quelles actions réduisent, déplacent ou masquent cette trace.
- **Feedback** : son, effet visuel et message éventuel.

Les données communes sont définies dans un Data Asset de type de trace. L’état propre à chaque occurrence reste dans un composant attaché à l’acteur.

## 5.2 États d’une trace

- **Active :** contribue au score selon son intensité.
- **Neutralisée :** n’est plus comptée, par exemple après nettoyage complet.
- **Masquée :** l’inspection standard ne la voit pas, mais une autre conséquence peut exister. Le masquage n’équivaut pas toujours à une suppression.
- **Révélée :** état temporaire d’affichage en mode UV ; il s’agit d’une propriété de présentation plutôt que d’une preuve supplémentaire.
- **Invalide :** réservée au debug si un acteur ou une donnée n’est plus cohérent.

Il faut séparer l’état de la preuve de son apparence à l’écran. Une trace ne doit pas être supprimée uniquement parce que son effet visuel a disparu.

## 5.3 Types de traces du prototype

| Type | Exemple | Action principale | Risque de réaction |
|---|---|---|---|
| Sang statique | Flaque sur carrelage | Nettoyage | Nettoyage partiel détectable |
| Sang dynamique | Traînée créée en déplaçant le corps | Nettoyage | Plusieurs traces peuvent apparaître |
| Objet déplacé | Chaise ou objet hors de sa position initiale | Remettre à la position de référence | Un déplacement supplémentaire peut empirer l’écart |
| Corps / objet compromettant | Proxy du corps | Cacher dans un contenant | Le contenant peut ne pas fermer |
| Ouverture | Porte ou fenêtre laissée ouverte | Refermer | La ligne de vue peut avoir déjà exposé un objet |
| Empreinte (extension proche) | Trace de pas ou de doigts | Outil adapté | Une mauvaise action peut étaler ou déplacer la trace |

Pour une démo verticale, commencer avec cinq familles : sang statique, objet déplacé, corps/objet compromettant, ouverture et empreinte simple. La traînée dynamique peut être ajoutée une fois le déplacement physique fiable.

## 5.4 Gravité et intensité

La gravité représente l’importance de la preuve ; l’intensité représente la quantité ou la visibilité résiduelle. Ces notions ne doivent pas être confondues.

Exemple de données de test :
- sang : gravité 2 ;
- objet compromettant : gravité 4 ;
- empreinte : gravité 7 ;
- porte ouverte : gravité 1.

Ces nombres sont des exemples inspirés de la boucle fournie, pas des valeurs d’équilibrage définitives. Le couteau taché et l’empreinte associée peuvent être modélisés comme deux traces distinctes si le design veut que leur cumul soit significatif.

## 5.5 Calcul du score

Formule proposée :

`Score de détection (%) = 100 × somme(gravité × intensité restante × facteur de visibilité) / somme(gravité × intensité maximale × facteur maximal)`

La formule doit être normalisée afin que le score reste entre 0 et 100 %. Le facteur de visibilité est facultatif pour la première version ; on peut commencer avec la formule simple du TDD :

`score = somme(gravité × intensité) / total maximal`

Puis convertir en pourcentage. Le total maximal doit être calculé de manière cohérente à partir des traces attendues dans la mission. Une trace neutralisée contribue zéro. Une trace masquée utilise une règle explicitement définie, et non une exception cachée dans le code.

**Seuil de réussite initial suggéré :** score de détection inférieur à 25 %, à tester et équilibrer. Ce seuil est une proposition de design, pas une exigence des documents sources.

## 5.6 Réactions en chaîne

Le système devient un sandbox quand les actions modifient plusieurs états :
- Traîner le corps sur une surface peut créer une ou plusieurs traces de sang.
- Déplacer un meuble change son transform par rapport à la position de référence.
- Nettoyer partiellement une surface diminue l’intensité, sans nécessairement la mettre à zéro.
- Cacher un objet le rend invisible à l’inspection visuelle, mais son volume peut empêcher de fermer le contenant.
- Casser un objet fragile ajoute bruit, débris et éventuellement une nouvelle preuve.
- Ouvrir une fenêtre peut modifier les lignes de vue ou les conditions d’entrée, selon les systèmes futurs.

La première implémentation doit être simple : chaque réaction appelle une fonction explicite et crée une trace enregistrée. Les simulations complexes ne sont pas nécessaires pour obtenir une boucle intéressante.

# 6. Interactions, objets et outils

## 6.1 Interaction générique

Le joueur vise un acteur. Si celui-ci implémente l’interface `Interactable`, un texte d’action apparaît et l’entrée d’interaction appelle sa fonction. Le personnage ne doit pas contenir une liste codée en dur de tous les objets du jeu.

Exemples :
- « Prendre »
- « Déplacer »
- « Nettoyer »
- « Ouvrir / Fermer »
- « Cacher »
- « Utiliser la lampe UV »

Le prompt doit afficher l’action réellement disponible et disparaître lorsque l’objet n’est plus ciblé.

### Principe confirmé : interactions indépendantes et assignables

Les interactions sont conçues comme des comportements indépendants de la preuve elle-même. Une preuve ne doit pas avoir à réécrire le système général du joueur pour proposer une action. Le joueur détecte la cible et demande une interaction générique ; l’objet ou le composant concerné décide ensuite de ce que cette interaction fait.

Exemples d’interactions réutilisables :
- nettoyer une trace ;
- examiner ou révéler une trace ;
- prendre, déplacer ou relâcher un objet ;
- ouvrir ou fermer un contenant ;
- cacher un objet dans un contenant ;
- déclencher une action contextuelle propre à un objet.

Chaque preuve peut déclarer quelles interactions elle accepte, quelles conditions doivent être remplies et quel résultat appliquer. Cette association doit être réglable autant que possible dans les propriétés de l’acteur ou dans des données, plutôt que par une longue liste de conditions codées dans le personnage. Une preuve donnée n’est pas obligée de proposer toutes les interactions.

Il faut distinguer trois responsabilités :
1. **Détection et demande** : le personnage ou `UInteractionComponent` vise l’acteur, affiche le texte approprié et transmet la demande.
2. **Règles de l’action** : l’interaction vérifie les conditions nécessaires (portée, outil compatible, état de l’objet, etc.) et exécute son comportement.
3. **État de la preuve et score** : `UEvidenceComponent` conserve l’état de la preuve et `UEvidenceSubsystem` met à jour le registre et le score. L’interface utilisateur affiche le résultat, sans devenir la source de vérité.

Une interaction spécifique, comme le nettoyage, peut être utilisée par plusieurs preuves. Inversement, une preuve peut accepter plusieurs interactions différentes, avec des conséquences distinctes. Le personnage ne doit pas contenir de logique particulière du type « si la cible est une tache, nettoyer ; si c’est une porte, ouvrir ».

Pour le prototype actuel, la mécanique de nettoyage déjà fonctionnelle reste en place. La généralisation se fera progressivement, après inspection du code existant, sans dupliquer ni remplacer prématurément `APreuveNettoyable` ou `UComposantNettoyage`.

## 6.2 Portage physique

- Objet léger : saisie via Physics Handle.
- Objet lourd : déplacement contraint ou drag.
- Lancer : impulsion limitée et prévisible.
- Corps : proxy simple dans la première version ; ragdoll seulement après stabilisation du système de saisie.
- Contenant : vérifier volume, collision, état d’ouverture et compatibilité avec l’objet.

Il faut tester les collisions, les angles et les objets coincés. La physique ne doit jamais bloquer définitivement une mission.

## 6.3 Nettoyage

L’action de nettoyage vérifie :
1. que l’outil est compatible avec la trace ;
2. que le joueur est à portée ;
3. que la cible est bien touchée ;
4. que la durée ou l’énergie d’action est suffisante ;
5. que l’intensité diminue ;
6. que l’effet visuel suit l’état de donnée ;
7. que le score se met à jour.

Un nettoyage partiel doit être possible. Il est plus intéressant de laisser une faible intensité résiduelle que de transformer chaque action en simple interrupteur marche/arrêt.

## 6.4 Objets fragiles

Dans une extension, casser un vase ou une assiette doit :
- déclencher un son ;
- produire des débris ou un effet visuel ;
- éventuellement attirer l’attention ;
- créer une trace d’objet brisé si la mission l’utilise ;
- permettre un nettoyage adapté.

Commencer avec un seul objet fragile et un effet simple, pas un système de destruction généralisé.

## 6.5 Meubles, murs, peinture et décoration

Le déplacement de meubles doit être suivi par comparaison entre la position initiale et la position actuelle. Peindre un mur ou le sol, remplacer une vitre, déplacer un tableau ou changer la disposition des plantes sont des systèmes coûteux : ils nécessitent un état de référence, des assets alternatifs, des règles de collision et des conséquences d’inspection.

Les repousser après la démo verticale. Si le système est ajouté, le design doit expliquer pourquoi l’action aide et quelles traces elle peut créer.

## 6.6 Textiles et vitres

Extension possible :
- textiles : tache, intensité, matériau, outil compatible, risque de décoloration ;
- vitres : salissure, bris, remplacement ou nettoyage ;
- odeur : système abstrait au départ, jamais une simulation chimique complète.

Ne pas développer plusieurs paramètres de textile avant que le nettoyage générique fonctionne sur une surface simple.

# 7. Détection, inspection et IA

## 7.1 Mode détection

La lampe UV ou le mode focus révèle les traces qui restent actives. Il peut utiliser un post-process, un matériau de surbrillance ou une interface simple au prototype.

Règles :
- ne pas révéler les objets neutres comme s’ils étaient suspects ;
- distinguer une trace faible d’une trace importante ;
- donner un feedback cohérent à l’activation et à la désactivation ;
- ne pas remplacer l’observation du joueur par un marqueur omniprésent.

## 7.2 Phase d’inspection — version initiale

La première version n’a pas besoin d’une IA autonome. L’Inspection Manager :
1. verrouille les interactions ;
2. récupère les traces enregistrées ;
3. détermine celles qui seraient détectées selon les règles de visibilité ;
4. calcule le score ;
5. présente la liste des traces détectées et manquées ;
6. affiche réussite ou échec.

La mise en scène peut être une caméra fixe, une suite de plans ou une séquence Sequencer. Le score doit rester calculé par le système de données, pas être décidé arbitrairement par la cinématique.

## 7.3 Inspecteur cinématique — version démo

Le bloc 3 prévoit un inspecteur mis en scène via Sequencer. L’inspecteur se déplace entre quelques points et la caméra révèle les traces pertinentes. Ce parcours est une présentation, pas une IA complète.

## 7.4 Inspecteurs autonomes — vision avancée

Plus tard, chaque inspecteur pourra utiliser :
- champ de vision ;
- distance et éclairage ;
- perception du bruit ;
- état d’alerte ;
- investigation d’un point suspect ;
- mémoire limitée des événements ;
- règles d’échec en cas de repérage.

L’IA ne doit pas être commencée avant que les traces et la phase d’inspection fonctionnent sans elle. Une IA qui détecte des objets mais ne comprend pas leur état produira des résultats incohérents.

## 7.5 Pièges, gaz et élimination d’inspecteurs

La boucle fournie mentionne des pièges glissants, du gaz somnifère et l’élimination d’inspecteurs. Ces mécaniques augmentent fortement le périmètre, le nombre d’états et les risques de bugs. Elles ne font pas partie du socle recommandé.

Si elles sont conservées dans une version avancée, elles doivent être traitées comme des mécaniques de jeu abstraites, équilibrées et lisibles : durée, portée, effet, contre-mesure, conséquences et coût. Ne pas les utiliser comme raccourci pour éviter le cœur du gameplay de nettoyage.

# 8. Van, inventaire et progression

## 8.1 Van — version de base

Le van est d’abord une zone de préparation et un hub narratif. Il contient les outils et un ordinateur qui affiche :
- résumé de mission ;
- pièces à traiter ;
- matériel choisi ;
- progression du briefing ;
- rapport après mission.

La conduite n’est pas nécessaire pour que le van remplisse son rôle.

## 8.2 Inventaire et poids

La barre d’inventaire peut être limitée par le poids :
- poids total = somme des poids des outils transportés ;
- vitesse et accélération peuvent diminuer au-delà de certains seuils ;
- les outils volumineux peuvent occuper des emplacements ;
- le joueur doit arbitrer entre polyvalence et mobilité.

Commencer par un inventaire à trois emplacements fixes. Ajouter ensuite un poids simple. N’ajouter les poursuites qu’une fois que les effets sur la vitesse sont testables et compréhensibles.

## 8.3 Boutique et tenues

La boutique n’a de sens que si les missions et l’économie existent. Les tenues peuvent être cosmétiques ; les outils doivent changer le style de jeu sans rendre une mission impossible si le joueur n’a pas acheté le bon équipement.

## 8.4 Progression

Une progression future peut débloquer :
- nouveaux outils ;
- capacités d’inventaire ;
- nouvelles missions ;
- options cosmétiques ;
- informations de briefing plus détaillées.

Ne pas ajouter de sauvegarde persistante avant d’avoir une boucle de mission stable.

# 9. Risque, échec et réussite

## 9.1 Variables de risque futures

- Score de détection.
- Temps passé sur place.
- Bruit produit.
- Visibilité du joueur.
- Traces générées pendant les manipulations.
- Poids transporté.
- Présence d’un témoin ou d’un inspecteur.
- Indices laissés dans la zone extérieure.

Le prototype ne doit pas utiliser toutes ces variables. Le score de détection et une limite de temps optionnelle suffisent.

## 9.2 Conditions d’échec

Selon la boucle fournie :
- être repéré par un policier ou un inspecteur ;
- laisser trop d’indices sur la scène ;
- se faire attraper à l’extérieur.

Dans la version initiale, seul le seuil du score provoque l’échec. Les autres conditions sont ajoutées quand les systèmes correspondants existent.

## 9.3 Conditions de réussite

- atteindre la zone de repli sans être attrapé ;
- ou rester caché jusqu’à la fin de l’inspection.

Pour le prototype : obtenir un score sous le seuil et terminer la phase d’inspection. Pour la version sandbox étendue, la réussite exige également de rejoindre le point d’extraction ou d’attendre la fin de l’inspection sans être repéré.

## 9.4 Écran de résultat

Afficher :
- score de détection ;
- score de discrétion = 100 − score de détection ;
- résultat réussi / échoué ;
- traces détectées ;
- traces neutralisées ;
- nouvelles traces créées par le joueur ;
- temps de mission ;
- conseils de compréhension, sans imposer une unique solution.

# 10. Architecture Unreal Engine modulaire

## 10.1 Principe

Architecture data-driven :
- composants pour les comportements réutilisables ;
- Data Assets pour les réglages ;
- Subsystem pour le registre global des traces ;
- interface pour les interactions ;
- delegates/events pour notifier les changements.

Le contenu doit pouvoir être réglé dans l’éditeur sans modifier le code. La logique centrale doit être testable indépendamment de l’apparence.

### Règle d’architecture : système commun, comportements séparés

Le système d’interaction est commun à tous les objets, mais les actions disponibles et leurs effets sont séparés. Le personnage ne gère ni la liste des preuves ni leurs règles particulières. Une interaction est appelée par son contrat commun ; elle vérifie si la cible accepte cette action, puis modifie l’état pertinent. Les preuves fournissent leurs données et leur état, tandis que le système global suit les preuves et le score.

Le lien entre preuve et interaction doit être explicite et configurable. À terme, les données pourront indiquer les actions autorisées, les outils compatibles, le texte de prompt, les conditions, l’effet sur l’intensité ou l’état de la preuve et le feedback à jouer. Les éléments réellement spécifiques à un objet restent dans sa classe ou son composant, pas dans le personnage.

### Flux nominal

1. `UInteractionComponent` effectue le Line Trace et identifie la cible.
2. La cible expose les interactions disponibles via `IInteractable` ou un mécanisme équivalent validé pour le projet.
3. Le joueur choisit ou déclenche l’action annoncée.
4. L’action vérifie ses préconditions et demande à la preuve ou à l’objet de changer d’état.
5. `UEvidenceSubsystem` reçoit la notification de changement et recalcule le score si nécessaire.
6. L’UI reçoit les événements et affiche le feedback.

Ce flux décrit l’architecture cible, pas l’état déjà garanti du prototype. Le code actuel doit être examiné avant de choisir les signatures définitives des interfaces et composants.

## 10.2 Modules proposés

### `ACleanerCharacter`
Responsable du déplacement, de la caméra et des entrées. Ne doit pas connaître les détails de chaque trace.

### `UInteractionComponent`
Effectue un Line Trace depuis la caméra, détecte un objet interactif, affiche le prompt et déclenche l’action.

### `IInteractable`
Contrat commun aux objets interactifs. Chaque acteur décide de ses propres actions. Il expose au minimum les informations nécessaires pour afficher l’action disponible et transmettre la demande d’interaction.

### Actions / comportements d’interaction
Les comportements comme nettoyer, examiner, ouvrir ou cacher sont conçus séparément de la preuve. Selon les besoins confirmés lors de l’implémentation, ils pourront être portés par des composants, des objets d’action ou des fonctions d’interface. Ne pas créer toutes ces classes d’un coup : commencer par l’interaction de nettoyage et extraire une abstraction seulement quand un second cas concret le justifie.

### `UCarryComponent`
Gère la saisie, le déplacement, le drag et le lancer d’objets physiques.

### `UEvidenceComponent`
Attaché aux acteurs pertinents. Stocke le type de trace, son intensité, son état et ses références.

### `UEvidenceTypeDataAsset`
Définit le nom, la gravité, la compatibilité avec les outils, la présentation visuelle et les règles de la famille de trace.

### `UEvidenceSubsystem`
Registre des traces de la mission et calcul du score. Il s’enregistre et se désenregistre proprement avec le cycle de vie des acteurs.

### `ACleaningTool`
Outil qui applique une action compatible et réduit l’intensité d’une trace.

### `AContainer`
Contenant qui accepte ou refuse un objet selon sa capacité et ses règles.

### `AInspectionManager`
Clôture la phase d’action, calcule le résultat et lance l’affichage ou la séquence.

### `UMissionDataAsset` (extension recommandée)
Contient les paramètres propres à une mission : seuil, liste des zones, traces de référence, briefing et conditions de sortie.

## 10.3 Répartition C++ / Blueprint pour un débutant

| Domaine | Départ conseillé | Évolution |
|---|---|---|
| Déplacement et caméra | Template First Person + réglages | C++ si nécessaire |
| Interaction générique | Blueprint de prototype, puis interface claire | Interface C++ si le projet l’exige |
| Physique | Components et réglages de l’éditeur | C++ pour comportement réutilisable |
| Trace et score | Blueprint simple pour apprendre | Component / Subsystem C++ après validation |
| Outils et objets | Blueprints enfants | Réutilisation des classes de base |
| UI | UMG Blueprint | Logique séparée des widgets |
| Inspection | Blueprint / Sequencer | Manager dédié |
| Données de mission | Data Assets lorsque les bases sont comprises | Réglages data-driven |

Le TDD initial cible du C++ pour les classes centrales et Blueprint pour l’itération. Pour quelqu’un qui part de zéro, il est plus sûr de comprendre d’abord le fonctionnement avec un prototype Blueprint, puis de migrer les systèmes stables. Ne pas recopier du C++ sans pouvoir expliquer ses fonctions.

## 10.4 Structure de dossiers

```text
Content/
  _Project/
    Blueprints/
      Player/
      Interaction/
      Evidence/
      Tools/
      Containers/
      Mission/
      UI/
    Data/
      EvidenceTypes/
      Missions/
      Tools/
    Levels/
    Materials/
    VFX/
    Audio/
    Props/
    Cinematics/
Source/
  Nettoyeur/
    Interaction/
    Evidence/
    Carry/
    Inspection/
    Mission/
Docs/
  GDD/
  Technical/
  Tests/
```

Les noms exacts peuvent varier, mais la séparation par système doit rester cohérente.

## 10.5 Événements et dépendances

- Le personnage appelle l’interface d’interaction ; il ne modifie pas directement un objet par son nom.
- Un outil appelle une fonction générique de nettoyage ; il ne connaît pas tous les types de trace.
- Un composant de trace notifie le Subsystem lorsqu’il change.
- Le Subsystem calcule le score à partir du registre.
- L’interface utilisateur lit le résultat et l’affiche ; elle ne calcule pas le score indépendamment.
- L’Inspection Manager orchestre la fin de mission ; il ne stocke pas une deuxième copie divergente des traces.

## 10.6 Git et versions

Dès le début :
- créer un dépôt Git ;
- ajouter le `.gitignore` Unreal ;
- utiliser Git LFS pour les assets binaires ;
- faire un commit après chaque jalon jouable ;
- ne pas versionner `Binaries`, `Intermediate`, `DerivedDataCache` et `Saved` ;
- garder un build de référence avant les changements risqués.

# 11. Plan de développement détaillé pour débutant Unreal

## Étape 0 — Préparer son environnement

**Objectif :** ouvrir un projet Unreal et savoir le lancer.

1. Conserver Unreal Engine 5.7.4 comme version de référence du projet et éviter toute migration sans raison explicite.
2. Créer un projet First Person de test.
3. Lancer Play, se déplacer et arrêter le jeu.
4. Identifier Content Browser, World Outliner, Details, Viewport et Output Log.
5. Créer un dossier de projet propre.
6. Mettre le projet sous Git et faire un premier commit.

**Validation :** le projet s’ouvre après fermeture de l’éditeur et le premier commit est disponible.  
**Ne pas faire encore :** importer des dizaines d’assets, écrire un système d’inventaire ou créer un menu complexe.

## Étape 1 — Apprendre les notions minimales

Apprendre progressivement :
- Actor : objet placé dans le monde ;
- Component : fonctionnalité ajoutée à un acteur ;
- Blueprint : logique visuelle ;
- variable : valeur stockée ;
- fonction : action réutilisable ;
- événement : réaction à un événement ;
- collision : géométrie utilisée pour bloquer ou détecter ;
- trace de rayon (Line Trace) : rayon de détection ;
- interface : contrat commun entre objets ;
- Data Asset : données configurables ;
- Widget / UMG : interface à l’écran.

**Exercice :** créer un cube qui change de couleur quand le joueur l’interagit.  
**Validation :** pouvoir expliquer ce qui déclenche le changement.

## Étape 2 — Joueur FPS et interactions

1. Partir du template First Person.
2. Vérifier déplacement, caméra et collisions.
3. Ajouter l’accroupissement si le template ne le fournit pas déjà.
4. Ajouter une entrée « Interagir » avec Enhanced Input.
5. Lancer un Line Trace depuis la caméra.
6. Afficher un prompt quand le rayon touche un objet interactif.
7. Créer un premier objet qui implémente la règle d’interaction.
8. Tester la portée, les angles, les objets occultés et l’absence de cible.

**Livrable :** le joueur vise un objet et le manipule via une interaction générique.

## Étape 3 — Manipulation physique

1. Créer plusieurs cubes avec physique activée.
2. Ajouter la saisie d’un objet léger via Physics Handle.
3. Définir une distance de maintien.
4. Gérer la libération de l’objet.
5. Ajouter le lancer avec une impulsion limitée.
6. Créer un objet lourd que le joueur traîne plutôt que de le porter.
7. Tester les collisions contre les portes, murs et autres objets.
8. Ajouter un proxy du corps, sans ragdoll complexe.

**Livrable :** saisir, déplacer, relâcher et lancer un objet sans casser la scène.

## Étape 4 — Première trace et état de référence

1. Créer un acteur représentant une trace.
2. Lui donner un type, une gravité et une intensité.
3. Afficher son intensité dans un texte de debug.
4. Créer une trace « objet déplacé » à partir de la transform initiale.
5. Comparer la transform initiale à la transform actuelle.
6. Définir un seuil de tolérance pour éviter les micro-écarts de physique.
7. Réduire l’intensité quand l’objet revient près de sa position de référence.
8. Tester déplacement, retour, rotation et petites perturbations physiques.

**Livrable :** un objet peut devenir suspect parce qu’il a été déplacé, et redevenir moins suspect lorsqu’il revient à sa place.

## Étape 5 — Registre global et score

1. Créer une structure de donnée de trace.
2. Enregistrer les traces actives au démarrage de la mission.
3. Les désenregistrer à la destruction ou à la fin de leur cycle de vie.
4. Calculer la somme gravité × intensité.
5. Déterminer le maximum théorique de la mission.
6. Convertir le résultat en pourcentage.
7. Afficher le score en debug.
8. Tester les cas zéro trace, trace complète, trace partielle et trace neutralisée.
9. Vérifier que le score ne dépasse jamais 100 % et n’est jamais négatif.

**Livrable :** le score change automatiquement quand une trace change.

## Étape 6 — Nettoyage générique

1. Créer un outil de nettoyage.
2. Définir portée et durée d’action.
3. Détecter une trace compatible.
4. Diminuer l’intensité progressivement.
5. Ajouter un feedback sonore et visuel.
6. Neutraliser la trace à intensité zéro.
7. Ajouter un cas de nettoyage partiel.
8. Tester une cible incompatible et une cible hors portée.

**Livrable :** le joueur peut réduire le score en nettoyant réellement les traces.

## Étape 7 — Contenants et dissimulation

1. Créer un contenant interactif avec état ouvert/fermé.
2. Définir les objets acceptés.
3. Vérifier la capacité et la collision.
4. Autoriser le placement d’un objet.
5. Si le contenant ferme correctement, appliquer la règle de masquage prévue.
6. Si l’objet bloque la fermeture, refuser l’action et afficher un feedback clair.
7. Vérifier que l’inspection utilise l’état réel du contenant.

**Livrable :** cacher un objet devient une alternative au nettoyage, avec une limite physique.

## Étape 8 — Mode détection

1. Ajouter une entrée pour activer la lampe UV / focus.
2. Déterminer les traces actives à proximité ou dans le champ de vision.
3. Ajouter un matériau de surbrillance ou une indication de couleur.
4. Distinguer les traces neutralisées des traces actives.
5. Ajouter un son d’activation discret.
6. Tester la lisibilité en lumière claire et sombre.
7. Vérifier que le mode n’altère pas le score.

**Livrable :** le joueur peut rechercher les traces qu’il aurait autrement manquées.

## Étape 9 — Première mission complète

1. Construire un appartement gris simple de trois pièces.
2. Placer cinq familles de traces.
3. Ajouter un corps proxy et un contenant.
4. Placer les outils de départ.
5. Créer un point de sortie.
6. Ajouter un bouton ou terminal « Terminer le nettoyage ».
7. Lancer l’inspection et calculer le score.
8. Afficher le résultat et la liste des traces.
9. Permettre de recommencer rapidement la mission.

**Livrable :** une boucle complète, du départ au résultat, même avec des graphismes temporaires.

## Étape 10 — Rendu et feedback

1. Ajouter le matériau de sang.
2. Utiliser des decals pour les traces statiques.
3. Ajouter la traînée de sang lorsque le corps est déplacé, d’abord avec des points échantillonnés.
4. Ajouter un VFX simple de nettoyage.
5. Régler éclairage, contraste et matériaux.
6. Tester le mode détection avec le rendu final.
7. Mesurer les performances avec `stat Unit`, `stat GPU` et Unreal Insights lorsque nécessaire.

**Livrable :** le joueur comprend l’état des traces sans devoir lire du debug.

## Étape 11 — Inspection mise en scène

1. Créer un `AInspectionManager` ou équivalent Blueprint.
2. Bloquer les actions du joueur pendant l’inspection.
3. Déclencher une séquence Sequencer.
4. Définir quelques points de caméra et de parcours.
5. Mettre en évidence les traces détectées.
6. Afficher le score final.
7. Prévoir une fin immédiate si la cinématique échoue ou si le niveau est relancé.

**Livrable :** l’inspection est une phase claire et dramatique, mais le score provient toujours du système de traces.

## Étape 12 — Étendre le jeu sans casser le socle

Ajouter un seul module à la fois :
1. inventaire à trois emplacements ;
2. van comme hub ;
3. briefing sur ordinateur ;
4. poids des outils ;
5. meuble déplacé ;
6. objet fragile ;
7. textile ;
8. vitre ;
9. zone extérieure et extraction ;
10. événement de contrôle ;
11. caméra de surveillance ;
12. IA d’inspecteur.

Chaque module doit avoir ses données, son feedback, ses tests et une condition de désactivation en debug si nécessaire.

# 12. Planning de neuf semaines — cible académique

Ce planning reprend le document source. Pour un débutant complet, il est ambitieux : il doit être considéré comme un calendrier de livrables prioritaires, pas comme une promesse d’apprentissage complet du moteur.

## Bloc 1 — Gameplay, C++/Blueprint et physique

### Semaine 1 : fondations et interaction
- Projet propre, versionnement, conventions.
- Personnage FPS, Enhanced Input, accroupissement.
- Interaction générique et prompt.
- Un objet test interactif.

**Rendu :** sandbox physique jouable avec interaction.

### Semaine 2 : portage et Chaos
- Grab via Physics Handle.
- Drag d’objet lourd et lancer.
- Objets physiques, collisions et triggers.
- Proxy du corps et premier objet destructible ou contrainte.

**Rendu :** manipulation physique complète.

### Semaine 3 : système de traces v1
- Evidence Component, registre global, Data Asset de type de trace.
- Score de détection en debug.
- Sang statique, objet déplacé.
- Contenant de dissimulation.
- Préparation et oral du bloc.

**Rendu :** prototype physique avec système de traces et score.

## Bloc 2 — Rendu temps réel

### Semaine 4 : matériaux et sang
- Matériau de sang paramétrable.
- Decals et fonctions de matériau si maîtrisées.
- Traînée dynamique le long du trajet du corps.

**Rendu :** rendu des traces amélioré.

### Semaine 5 : détection et VFX
- Mode UV / focus.
- Niagara pour feedback de nettoyage.
- Éclairage et lisibilité.

**Rendu :** feedback visuel cohérent.

### Semaine 6 : optimisation
- Profiling avec Unreal Insights, `stat GPU`, `stat Unit`.
- Mesures avant/après.
- Passe de post-process et oral.

**Rendu :** expérience visuelle interactive et documentée.

## Bloc 3 — Mise en scène et expérience

### Semaine 7 : inspecteur et Sequencer
- Inspection mise en scène.
- Caméra de révélation.
- Présentation des traces détectées.

**Rendu :** phase d’inspection cinématique.

### Semaine 8 : direction et son
- Pacing de la tension.
- Sons d’outils, révélations, réussite et échec.
- Éclairage et narration environnementale.

**Rendu :** expérience dirigée.

### Semaine 9 : stabilisation et soutenance
- Tests des cas limites.
- Documentation technique.
- Build jouable et répétition de soutenance.

**Rendu :** prototype final stable.

## Rituels hebdomadaires
- Commit réguliers.
- Build jouable à la fin de chaque semaine.
- Mise à jour de la documentation.
- Test de non-régression.
- Déclaration d’intention IA requise par le cadre du projet source.
- Être capable d’expliquer chaque système montré.

# 13. Plan de tests et critères d’acceptation

## 13.1 Joueur et interaction
- Le joueur peut se déplacer sans traverser les murs.
- Le prompt apparaît uniquement sur une cible valide.
- L’interaction ne fonctionne pas à travers un mur.
- La perte de cible masque le prompt.
- Une interaction invalide ne fait pas planter le jeu.

## 13.2 Physique
- Un objet léger peut être pris et relâché.
- Un objet lancé réagit de façon cohérente.
- Les objets ne restent pas attachés après une fin de mission.
- Un objet coincé peut être récupéré ou la mission redémarrée.
- Le proxy du corps reste manipulable sans ragdoll avancé.

## 13.3 Traces et score
- Une trace neuve a une intensité comprise entre 0 et 1.
- Le nettoyage diminue l’intensité.
- Une trace neutralisée ne contribue plus au score.
- Le score est stable et borné entre 0 et 100 %.
- Une trace supprimée ou un acteur détruit est correctement retiré du registre.
- Une trace masquée suit une règle explicite et reproductible.

## 13.4 Mission
- La mission démarre toujours dans un état connu.
- Le joueur peut déclencher l’inspection.
- L’inspection utilise les données courantes, pas une copie ancienne.
- Le résultat liste les traces pertinentes.
- Le bouton recommencer réinitialise toutes les traces et positions.
- Il existe une issue de réussite et une issue d’échec.

## 13.5 Rendu et performance
- Le mode détection reste lisible.
- Les decals ne clignotent pas de façon gênante.
- Les effets visuels ne masquent pas les interactions.
- Une mesure de performance est prise avant la passe d’optimisation.
- Les options visuelles sont testées sur la machine cible.

# 14. Risques et stratégies de réduction

| Risque | Pourquoi | Réponse |
|---|---|---|
| Trop de fonctionnalités | Le scope initial est déjà dense | Une seule mission et cinq types de traces au départ |
| Débutant submergé par le C++ | Plusieurs notions techniques sont nouvelles | Prototype Blueprint, puis migration progressive |
| Physique instable du corps | Ragdoll et contraintes sont difficiles à régler | Proxy simple avant ragdoll |
| Score incohérent | Traces créées/détruites sans registre fiable | Tests sur le Subsystem et règles explicites |
| Jeu perçu comme une checklist | Les actions n’ont pas de conséquences | Ajouter des réactions secondaires et des solutions alternatives |
| Traces illisibles | L’état visuel ne correspond pas à l’état de donnée | Une source de vérité et un feedback par état |
| IA trop ambitieuse | Perception et navigation multiplient les cas limites | Inspection calculée puis Sequencer avant IA temps réel |
| Perte du projet | Assets et changements risqués | Git + LFS, commits et builds réguliers |
| Perf faible | Decals, transparences et post-process | Mesurer avant d’optimiser |

# 15. Décisions de design à figer tôt

Les documents ne définissent pas encore ces points ; voici les décisions provisoires recommandées :

1. **Durée d’une mission :** viser 10 à 20 minutes pour la démo verticale.
2. **Point de vue :** FPS.
3. **Échec de base :** seuil de score dépassé.
4. **Seuil de départ :** 25 %, à équilibrer avec des tests.
5. **Outils initiaux :** nettoyant, lampe UV, sac/contenant.
6. **Nombre de traces initiales :** cinq familles.
7. **Inspection :** calcul de données, puis Sequencer en présentation.
8. **Niveau :** appartement de trois pièces.
9. **Conduite :** reportée après validation de la mission à pied.
10. **IA en temps réel :** reportée après la phase d’inspection cinématique.
11. **C++ :** classes réutilisables une fois le prototype compris.
12. **Progression :** pas de boutique tant qu’il n’y a pas plusieurs missions et un besoin d’économie.

Ces choix peuvent changer, mais toute modification doit être motivée par un test jouable.

# 16. Glossaire Unreal pour débutant

- **Actor :** objet pouvant être placé dans le niveau.
- **Component :** module fonctionnel attaché à un Actor.
- **Blueprint :** programmation visuelle d’Unreal.
- **C++ :** langage de programmation utilisé pour les systèmes plus structurés.
- **Pawn / Character :** entité contrôlée par le joueur ou l’IA ; Character ajoute des fonctions de déplacement de personnage.
- **Enhanced Input :** système moderne de gestion des entrées.
- **Line Trace :** rayon invisible utilisé pour détecter un objet.
- **Interface :** contrat qui permet à des objets différents de répondre à une même action.
- **Physics Handle :** outil permettant de saisir et déplacer un composant physique.
- **Chaos :** système de physique et de destruction d’Unreal.
- **Actor Component :** composant réutilisable qui porte une logique.
- **Subsystem :** objet de service géré par Unreal pour centraliser une responsabilité.
- **Data Asset :** ressource de données réglable dans l’éditeur.
- **Delegate / Event :** mécanisme permettant de notifier d’autres systèmes qu’un événement s’est produit.
- **Decal :** projection de matériau sur une surface, utile pour une flaque ou une trace.
- **Material :** définition visuelle de la surface d’un objet.
- **Niagara :** système d’effets de particules.
- **UMG / Widget :** système d’interface utilisateur.
- **Sequencer :** outil de mise en scène et de cinématiques.
- **Profiling :** mesure des performances pour identifier les coûts réels.

# 17. Checklist de livraison finale

- [ ] Le projet s’ouvre et se lance depuis un état propre.
- [ ] Les commandes sont indiquées.
- [ ] Une mission complète peut être terminée.
- [ ] Les traces changent réellement d’état.
- [ ] Le score est calculé à partir des données de trace.
- [ ] Le mode détection montre les traces pertinentes.
- [ ] Le joueur peut nettoyer, déplacer et cacher au moins un objet.
- [ ] L’inspection explique le résultat.
- [ ] Une réussite et un échec sont possibles.
- [ ] Le jeu peut recommencer sans état résiduel.
- [ ] Les cas limites connus sont documentés.
- [ ] Le dépôt Git est propre et les assets sont gérés.
- [ ] Un build jouable est livré.
- [ ] La documentation technique est à jour.
- [ ] Chaque système présenté peut être expliqué et justifié.

# 18. Références de conception utilisées

Ce document est construit à partir des éléments fournis :
- `01-note-intention.md` : fantasy du joueur, piliers systémiques, périmètre contraint et blocs de rendu.
- `02-gdd-leger.md` : mécaniques, types de traces, score, scope minimal et risques.
- `03-tdd-architecture.md` : classes, architecture data-driven, séparation C++/Blueprint et ordre de construction.
- `04-planning-9-semaines.md` : planning en trois blocs, livrables et rituels.
- `Boucle-de-Gameplay.txt` : menu, van, outils, actions possibles, inspection, défaite et victoire.

Les fonctionnalités supplémentaires de la Boucle de Gameplay sont conservées comme vision d’extension. Les seuils, durées et priorités non définis dans les fichiers sources sont identifiés comme des propositions de design à valider par le prototype.

---

## Conclusion

La réussite de *Nettoyeur* ne dépend pas du nombre d’outils, de pièges ou de niveaux. Elle dépend de la qualité d’une boucle simple : le joueur observe, agit, provoque des conséquences, vérifie, puis découvre si ses décisions ont été suffisantes.

Construis d’abord une seule pièce qui fonctionne. Puis une mission. Puis une inspection. Quand cette petite version est amusante et fiable, chaque nouveau système pourra l’enrichir sans remplacer le cœur du jeu.
