# Journal — orchestrateur

> Entrées selon `doc/CONVENTION.md`, la plus récente en haut.

## T000 — Vérifier la config éditeur et documenter l'existant — 2026-11-12

### 1. Fichiers créés / modifiés
- `doc/taches/T000-verification-et-doc-existant.md` : statut « En cours » puis « Terminé » ; trois derniers critères cochés après la validation du développeur.
- `doc/00_etat-projet.md` : réécrit avec les constats de T000 (lignes ❓ / 🟡 levées, dette n° 1 à 16 + B1/B2 répartie par tâche, points en attente du développeur).
- `doc/taches/T001-unifier-visee.md`, `T002-lancer-objet.md`, `T003-preuves-et-score.md` : section « Hérité de T000 » ajoutée en fin de fiche (quelle dette chaque tâche reprend).
- Ce journal.

### 2. Implémentation du code
Aucune par l'orchestrateur. Seule modification de code de la tâche : `InteractionTypes.h` réenregistré en UTF-8 par `gameplay-cpp` (voir son journal §3).

### 3. Configuration
Aucune.

### 4. Étapes dans l'éditeur
Aucune par l'orchestrateur. Le développeur a fait les tests A, B, C de la procédure QA (`doc/agents/qa.md`) : affichage de « Traîner », nettoyage 4 × E, Pull avec R → tous OK.

### 5. Compilation et tests
- Compilation faite par `qa` éditeur fermé : succès, 0 erreur, 0 warning.
- Validation humaine : OK le 2026-11-12. Grab non testable (aucun objet [Grab]) → reporté en T002.

### 6. Git
- Branche `tache/T000-verification-existant` créée depuis `master`.
- Commit `T000: documenter l'existant et passer InteractionTypes.h en UTF-8`, puis merge dans `master` et push.
- **Volontairement exclus du commit** (décision du développeur attendue) : la modification de `Nettoyeur.uproject` (version `"5.7"` + plugin EngineWire) et le fichier non suivi `Content/__ExternalActors__/.../I6/M8RK6YKWGV6E3Q8C5UHZX3.uasset` (PointLight). Ils restent dans la copie de travail.
- Équivalent manuel : `git checkout -b tache/T000-verification-existant`, `git add doc/ Source/Nettoyeur/InteractionTypes.h`, `git commit`, `git checkout master`, `git merge --no-ff tache/T000-verification-existant`, `git push`.

### 7. Résultat et points ouverts
- T000 terminée. Prochaine tâche : T001.
- En attente du développeur : `Nettoyeur.uproject`, le PointLight non suivi, `BP_FirstPersonCharacter_Backup`.
- Remarques mineures de la QA laissées en l'état (non bloquantes) : 4 notions sans lien vers la doc officielle, §5 de `gameplay-cpp.md` qui dit « compilation non réalisée » (faite ensuite par QA).
- Pour la QA : depuis Git Bash, Build.bat échoue à cause de l'espace dans « Program Files » ; le lancer depuis PowerShell (`& "C:/Program Files/.../Build.bat" ...`).
