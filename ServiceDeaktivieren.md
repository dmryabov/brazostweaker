# Wozu diese Seite? #

Hier findet ihr die Beschreibung, wie man den Service deaktiviert, falls er beim normalen Booten, Windows zum Absturz bringt.

# Details #

  1. Um den Service zu deaktivieren (falls Windows nicht mehr normal lädt und zwischendrin abstürzt), muß man Windows im **abgesicherten** Modus starten. Dabei wird der BrazosTweaker Service **nicht** gestartet.
  1. Sobald Windows erfolgreich gestartet ist, auf Windows-Start klicken, danach in der untersten Zeile "Dienste" eingeben und Enter drücken.
  1. Nun sieht man eine Liste aller installierten Dienste, bei der man einfach BrazosTweaker sucht.
  1. Als nächstes in der Zeile BrazosTweaker mit der rechten Maustaste klicken und auf Eigenschaften gehen.
  1. Dann einfach auf dem Reiter "Allgemein" in der Zeile "Starttyp" schauen und "Automatisch" in "Manuell" ändern.
  1. Jetzt kann der Rechner neugestartet werden und sollte problemlos starten.
  1. Nach dem nächsten Windows Boot, BrazosTweaker starten und die Ursache für die Systemstabilität finden (zu niedrige Spannung bei einem PState, Bug im Programm, etc.)
  1. Falls das Problem behoben wurde, kann der Service wie oben beschrieben wieder auf "Automatisch" gestellt werden, damit er wieder aktiv ist.