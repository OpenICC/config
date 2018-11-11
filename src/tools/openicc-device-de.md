# openicc\-device v0.1.1 {#openiccdevicede}
*"openicc\-device"* *1* *""* "User Commands"
## NAME
openicc\-device v0.1.1 \- OpenICC Device
## ÜBERSICHT
**openicc\-device** \-l [\-d *NUMBER*] [\-j] [\-n] [\-b *FILENAME*] [\-v]
<br />
**openicc\-device** \-a \-f *FILENAME* [\-b *FILENAME*] [\-v]
<br />
**openicc\-device** \-e \-d *NUMBER* [\-b *FILENAME*] [\-v]
<br />
**openicc\-device** \-p [\-s] [\-v]
<br />
## BESCHREIBUNG
Manipulation of OpenICC color management data base device entries.
## OPTIONEN
### Print the Devices in the DB
**openicc\-device** \-l [\-d *NUMBER*] [\-j] [\-n] [\-b *FILENAME*] [\-v]

* \-l|\-\-list\-devices	List Devices
* \-d|\-\-device *NUMBER*	Device position
* \-j|\-\-dump\-json	Dump JSON
* \-n|\-\-long	List all key/values pairs

### Add a Devices to the DB
**openicc\-device** \-a \-f *FILENAME* [\-b *FILENAME*] [\-v]

* \-a|\-\-add	Add Device to DB
* \-f|\-\-file\-name *FILENAME*	File Name
   * \-f device\-file\-name.json		# Device File

### Erase a Devices from the DB
**openicc\-device** \-e \-d *NUMBER* [\-b *FILENAME*] [\-v]

* \-e|\-\-erase\-device	Erase Devices
* \-d|\-\-device *NUMBER*	Device position

### Show Filepath to the DB
**openicc\-device** \-p [\-s] [\-v]

* \-p|\-\-show\-path	Show Path
* \-s|\-\-scope	System

### General options

* \-b|\-\-db\-file *FILENAME*	DB File Name
   * \-b DB\-file\-name.json		# DB File
* \-X|\-\-export *json|json+command|man|markdown*	Export formated text: Get UI converted into text formats
   * \-X json		# Json
   * \-X json+command		# Json + Command
   * \-X man		# Man
   * \-X markdown		# Markdown
* \-v|\-\-verbose	plaudernd
* \-h|\-\-help	Help

## AUTOR
Kai\-Uwe Behrmann http://www.openicc.org
## KOPIERRECHT
*Copyright 2018 Kai\-Uwe Behrmann*


### Lizenz
newBSD
## FEHLER
[https://www.github.com/OpenICC/config/issues](https://www.github.com/OpenICC/config/issues)

