## Librairies utilisées

Nous avons utilisé __``jsmn``__ pour l’analyse du JSON et __``sha1``__ pour la création de hash sur 40 caractères.

## Lancement

Au lancement du programme, plusieurs actions sont effectuées :  
- Lecture du fichier de configuration beesy.inc 
- Affectation des options à la structure Settings 
- Vérification des fichiers/dossiers nécessaires au bon fonctionnement du programme, si première connexion : 
- Création du fichier __``/marker``__ et __``/xxx/refs``__  
- Création des dossiers __``/xxx/current/``__ et __``/xxx/tags/``__  
- Initialisation du mot de passe root  
- Création du fichier root avec le hash du mot de passe précédent  

Attention, lors de la première connexion, l’utilisateur est __root__.

## Interpréteur de commandes

Lorsqu’une commande est saisie, elle passe par différentes étapes d’analyse avant d’être reconnue puis exécutée :  
- Suppression des caractères d’espacements et de fin de ligne en début et fin de chaîne 
- Séparation en tableau de sous-chaines à partir de chaque espace 
- Détection d’une commande reconnue par le programme 
- Analyse des flags et des paramètres (taille et caractères interdits) 
- Exécution de la fonction associée 

## Gestion des privilèges

Les privilèges sont stockés dans la variable permission (structure Settings). Elle est initialisée à __UNINITIATED__. 

Aperçu binaire de _permission_ / signification  
00000000000000000000000000000001 1er bit utilisé pour le mode UNINITIATED*  
00000000000000000000000000000010 2ème bit utilisé pour le mode WAIT*  
00000000000000000000000000000100 3ème bit utilisé pour le mode ADVANCED*  
00000000000000000000000000001000 4ème bit utilisé pour le mode STAGE*  
01000000000000000000000000000000 31ème bit utilisé pour le mode SECURITY*  
10000000000000000000000000000000 bit de poids fort réservé pour le privilège ROOT  

Les privilèges __ROOT__ et __SECURITY__ sont indépendants des autres privilèges. 

_* uninitiated mode : en cours de validation des fichiers nécessaires au lancement de l’application_  
_* wait mode : en attente d’une connexion avec une base de données_  
_* advanced mode : connecter à une base de données_  
_* stage mode : des éléments ont été ajoutés à la pile et sont prêts à être « poussés » vers une collection_  
_* security mode : commit à chaque modification de la base de données_  

## Fonctionnement du moteur SGBD

Notre projet étant un système de gestion de base de données orientée documents, l’architecture change par rapport à une base de données relationnelle.

Premièrement on parle de collections pour définir grossièrement des tables et de documents pour les colonnes dans ces tables.

Dans _Beesy_, les bases de données seront représentées par des dossiers, les collections par des fichiers et les documents par des lignes au format JSON dans ces fichiers.

Le dossier __``/xxx/current/``__ contient toutes les bases de données actuelles alors que les sauvegardes (commit*) sont stockées dans __``/xxx/tags/``__ (le fichier refs liste simplement les sauvegardes). 

Dernièrement, lors de la création d’une base de données, un fichier passwd est créé dedans et contient le mot de passe chiffré de la base de données.

_* commit : copie à un instant T de toutes les bases de données._ 

Remarque :   
Le hash résultant d’un commit est généré grâce à sha1, tandis que l’ID de chaque document en bsd.

## Commandes et droits associés

Pour avoir de l’aide sur l’utilisation des autres commandes, taper help.

Liste des commandes connues dans Beesy (droit à l’entrée, droit à la sortie) :  
- Connexion à une base de données (__WAIT__/__ADVANCED__) 
```batch
$ connect database password
```
- Déconnexion à la base de données (__ADVANCED__, __STAGE__/__WAIT__) 
```batch
$ disconnect
```
- Insertion d’un document dans une collection  
- Ajout des éléments à la pile* (__ADVANCED__/__STAGE__) 
```batch
$ insert --push --type key value
```
- Transfère de la pile vers la collection (__STAGE__/__ADVANCED__) 
```batch
$ insert --pull collection
```
- Recherche de documents dans une collection* (__ADVANCED__, __STAGE__/__ADVANCED__, __STAGE__) 
```batch
$ search --type collection key symbol value
```
- Suppression d’une base de données (__WAIT__/__WAIT__) 
```batch
$ drop -db/--database database password
```
- Suppression d’une collection (__ADVANCED__, __STAGE__/__ADVANCED__, __STAGE__) 
```batch
$ drop -c/--collection --type collection
```
- Suppression d’un document (__ADVANCED__, __STAGE__/__ADVANCED__, __STAGE__) 
```batch
$ drop -doc/--document --type collection key symbol value
```
- Créer une sauvegarde (__ROOT__/__ROOT__) 
```batch
# commit
```
- Charger une sauvegarde (__ROOT__/__ROOT__) 
```batch
# reset hash
```
- Afficher les logs des sauvegardes (__WAIT__, __ADVANCED__, __STAGE__, __ROOT__/__WAIT__, __ADVANCED__, __STAGE__, __ROOT__) 

```batch
$ log
```
- Connexion au compte root (__WAIT__, __ADVANCED__, __STAGE__/__WAIT__, __ADVANCED__, __STAGE__, __ROOT__) 
```batch
$ sudo password
```
- Quitter l’application / revenir en mode user (__WAIT__, __ADVANCED__, __STAGE__, __ROOT__/__WAIT__, __ADVANCED__, __STAGE__) 
```batch
$ exit
```

_* pile : limitée à 50 emplacements_ 
_* recherche : limitée à 100 résultats_ 

Les types reconnus sont les entiers (-i, --integer), les réels (-r, --real) et les chaines de caractères (-s, -string). 

Les opérateurs reconnus par type sont :  
- Les entiers :  =, <, >, <=, >=, ! 
- Les réel : =, <, >, <=, >=, ! 
- Les chaines de caractères : =, !, | 

Remarque :  
a ! b : a différent de b  
a | b : b contenu dans a  

## Exemple

Nous allons créer une collection, initial, dans la base de données esgi (mot de passe test). Cette collection contiendra un élève (Bob, 19 ans avec une moyenne de 14.2). On fera ensuite une recherche dessus avant de supprimer respectivement la collection puis la base de données. 

```shell
$ connect esgi test
esgi$ insert --push --string name Bob
esgi$ insert --push --integer age 19
esgi$ insert --push --real average 14.2
esgi$ insert --pull initial
esgi$ search --string initial name = Bob
esgi$ drop --collection initial
esgi$ disconnect
$ drop --database esgi test
$ exit
```