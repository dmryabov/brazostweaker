# Anleitung BrazosTweaker #
## Installation ##
Um das Tool und den Service zu nutzen, müßt ihr erst einmal installieren. Dabei kann es möglich sein, daß Microsofts .NET Framework 4 zusätzlich installiert werden muß, falls es nicht schon vorhanden ist.
Bei einer schon vorhandenen Version vom BrazosTweaker empfehle ich die vorherige De-Installation.

## Reiter ##
Sobald es gestartet ist, sollte man ein kleines Fenster mit 6 Reitern sehen. Die ersten 3 Reiter stehen für einen Zustand, indem sich die CPU befinden kann, die beiden Reiter mit "NB" zeigen die Einstellungen für die Northbridge/Grafikeinheit (GPU) und der 6te Reiter ist eher für Debug und weitere Informationen nützlich.

## CPU PStates ##
Hier geht es erst einmal nur um die ersten 3 Reiter.
Beim E-350 werden alle drei Zustände benutzt, wobei die C-50 Version nur zwei verwendet. Der jeweils aktive ist mit einem Sternchen gekennzeichnet. Sie werden P0 / P1 / P2 genannt. P0 ist dabei der schnellste und P2 der langsamste. Je nach Last vom System, wird dynamisch zwischen den Zuständen hin und hergeschaltet, wobei natürlich P2 weniger Strom verbraucht, als P0.

### Spannung ###
Mit dem Tool kann man jetzt die Spannung (CPU VID) für jeden dieser Zustände einzeln verstellen. Weiterhin läßt sich auch der Divider verändern, womit die jeweilige Frequenz pro Zustand einstellbar ist.

Nun können als erstes folgende Einstellungen vorgenommen werden (diese funktionieren bei ziemlich vielen Usern)

  * P0 - 1.225V

  * P1 - 1.05V

  * P2 - 0.85V

Dann drück man auf "Apply" rechts unten. Damit werden diese Einstellungen erst einmal temporär übernommen. Überprüft werden kann das mit dem sehr guten Programm CPU-Z.

Wenn das alles klappt, sollte man Orthos downloaden. Damit kann man einen Stresstest durchführen, damit man weiß, ob die Einstellungen stabil sind. Ansonsten können zum Besipiel Blue Screens oder Einfrieren des Rechners vorkommen.

Jeder Zustand sollte einzeln mit dem Stresstest überprüft werden.
Um einen speziellen Zustand (PState)zu halten, kann man folgendermaßen vorgehen:
  1. Windows Taste drücken und unten links "Power Options" eingeben
  1. Dann sollte "Energieoptionen" aufgehen
  1. Bei "Ausbalanciert" -> "Energiesparplaneinstellungen ändern" klicken
  1. Unten "Erweiterte Energieeinstellungen ändern" klicken
  1. Runter scrollen bis Prozessorenergieverwaltung und dort kann Minimum und Maximum für die Frequenz in Prozent eingestellt werden. Einfach ein bisschen mit den Werten herumprobieren (Minimum = Maximum setzen), bis er im gewünschten Zustand bleibt.

Sollte das Notebook bei den Tests einfrieren, ist das nicht schlimm. Dann war die Spannung etwas zu niedrig. Einfach lange den Power Knopf drücken und neu mit etwas höherer Spannung probieren (die oben angegebenen Spannungen sollten aber schon relativ sicher sein). Die eingestellten Spannungen im Tool sind noch nirgendwo fest eingestellt.

Sobald alles stabil läuft, **aber auch erst dann**, kann man auf "Service..." klicken. Bitte vorher folgenden Beitrag beachten: ServiceDeaktivieren. Damit kann ein Dienst gestartet werden, der dann beim Windows Start schon dafür sorgt, daß diese modifizierten Spannungen verwendet werden. Dazu einfach auf Update klicken und die Checkbox links aktivieren.
**Bitte noch nicht Custom Cool&Quiet aktivieren. Dann friert der Rechner nach einer Weile ein! Das wird in der nächsten Version repariert. -> Mit Version V1.0.3 sollte es jetzt funktionieren!**

### Frequenz/Divider ###
Wer möchte, kann auch an den Dividern jedes PStates herumstellen, womit sich noch wesentlich geringere Taktfrequenzen erreichen lassen. Zum Beispiel läuft das Notebook auch locker mit 400MHz und einer Spannung von nahe 0.7V. Allerdings ist eine Erhöhung über den Maximaltakt nicht möglich, weil die CPU intern keine niedrigeren Divider übernimmt.
  * Beispiel:
  * E-350 - 1.6GHz
  * FSB 100MHz
  * Multiplier 32x
  * Divider 2
  * Maximaltakt 32/2\*100MHz = 1600MHz
Auch wenn der Divider kleiner als 2 einzustellen geht, wird das von der CPU intern geblockt und nicht verwendet. Die CPU bleibt dann einfach bei 1.6GHz!

## NB PStates ##

### Spannung ###
Da die NB (Northbridge) und die GPU (Grafik) sich zusammen eine Spannungsversorgung teilen, macht es durchaus Sinn auch diese Spannung abzusenken, um eine längere Batterielaufzeit zu erreichen.
Im Grunde sind die Einstellmöglichkeiten sehr ähnlich denen, wie auf der CPU, nur daß man maximal zwei PowerStates zur Verfügung stehen hat.
Bei mir funktionieren folgende Einstellungen:
  * NB P0 - 0.85V
  * NB P1 - 0.8V

Etwas weiter darunter friert bei mir der Bildschirm ein und nur ein langes Drücken des Powerknopfes erweckt das Notebook wieder zum Leben.

### Frequenz/Divider ###
Die Einstellungen des Dividers haben auf der NB bisher keine Funktion.

Wenn dir diese Anwendung gefällt und du willst meine Arbeit unterstützen, kannst du hier spenden.<br>
<a href='https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=MXVAJLM9ABXC4'><img src='https://www.paypal.com/en_US/i/btn/btn_donateCC_LG.gif' /></a>