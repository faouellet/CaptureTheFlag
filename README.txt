Installation:

1- Rendez vous � http://aisandbox.com/download/ et choissisez votre plateforme

2- T�l�charger et installer AI Sandbox 

3- T�l�charger le d�p�t

Compilation

1- Rendez vous dans le dossier MyCommander/src 

2- Ouvrez CaptureTheFlag.sln avec Visual Studio 2010 ou 2012
	1.1 - Si vous l'ouvrez avec Visual Studio 2012, ne pas mettre � jour la version du compilateur utilis�.
		  Le compilateur pr�vil�gi� pour la comp�tition �tait VC10 et c'est celui que j'utilise.
		  
3- Compilez

4- Une nouvelle version de l'ex�cutable est maintenant dans le dossier bin

Ex�cution

1- Rendez vous dans le dossier MyCommander/bin

2- Pour ex�cuter l'application, lancer Play.bat
	1.1- Pour changer l'adversaire, vous pouvez modifier examples.BalancedCommander dans Play.bat par: examples.DefenderCommander
																									   examples.GreedyCommander
																									   examples.RandomCommander
	1.2- NE PAS modifier game.NetworkCommander sinon plus rien ne marchera

3- Pour ex�cuter les tests, lancer Test.exe 
	(Note: L'application n'�tant pas compl�te, les tests vont planter)
