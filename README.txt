Installation:

1- Rendez vous à http://aisandbox.com/download/ et choissisez votre plateforme

2- Télécharger et installer AI Sandbox 

3- Télécharger le dépôt

Compilation

1- Rendez vous dans le dossier MyCommander/src 

2- Ouvrez CaptureTheFlag.sln avec Visual Studio 2010 ou 2012
	1.1 - Si vous l'ouvrez avec Visual Studio 2012, ne pas mettre à jour la version du compilateur utilisé.
		  Le compilateur prévilégié pour la compétition était VC10 et c'est celui que j'utilise.
		  
3- Compilez

4- Une nouvelle version de l'exécutable est maintenant dans le dossier bin

Exécution

1- Rendez vous dans le dossier MyCommander/bin

2- Pour exécuter l'application, lancer Play.bat
	1.1- Pour changer l'adversaire, vous pouvez modifier examples.BalancedCommander dans Play.bat par: examples.DefenderCommander
																									   examples.GreedyCommander
																									   examples.RandomCommander
	1.2- NE PAS modifier game.NetworkCommander sinon plus rien ne marchera

3- Pour exécuter les tests, lancer Test.exe 
	(Note: L'application n'étant pas complète, les tests vont planter)
