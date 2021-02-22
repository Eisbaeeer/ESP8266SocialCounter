![Logo](pics/logo.jpg)
# -DEPRECATED- ESP8266 Social Counter

## Beschreibung
http://www.kidbuild.de   

This repo will not be updated, because the used lib was unusable after facebook changes the instagram API. The request for counter of instagram will be blocked. There is a new version on the website of kidbuild.
 
## Verzeichnisstruktur
- pics = Bilder
- source = Arduino Sketch

## Projektbilder
![Logo](pics/counter1.jpg)
![Logo](pics/counter2.jpg)
![Logo](pics/counter3.jpg)
![Logo](pics/counter4.jpg)
![Logo](pics/counter5.jpg)

## Changelog

 ### Version 2.73
 - (Eisbaeeer 20210124)
 + fix display 5 > modules (max 10)
  
 ### Version 2.72
 - (Eisbaeeer 20210122)
 + Updload firmware locally via browser

 ### Version 2.71   
 - (Eisbaeeer 20210120)
 + Print "---" by blocking by Instagram

### Version 2.7   
- (Eisbaeeer 20210114)   
+ added scrolltext for IP-Address in info
 
### Verison 2.6   
- (Eisbaeeer 20210113)   
+ Change SPIFFS to LitteFS because SPIFFS is deprecated
+ Move clock to center if 4 modules used

### Version 2.5
- (Eisbaeeer 20200103)
+ Extended info WifiManager on display

### Version 2.4
- (Eisbaeeer 20210101)
+ Bugfix redirect 'captive portal' first config not working
+ Cleaned code

### Version 2.3
- (Eisbaeeer 20201230)
+ Bugfix read temperature and humitdity only if checked

### Version 2.2
- (Eisbaeeer 20201214)
+ Bugfix MAX_DEVICES not read from JSON

### Version 2.1
-(Eisbaeeer 20201119)
+ Fade-in fade-out effect added
 
### Version 2.0
- (Eisbaeeer 20201117)
+ changed libs from u8g2 to MD_MAX72xx
+ added animations

### Version 1.5
- (Eisbaeeer 20201102)
+ webpage config select mode (completely new)
+ changed font type
+ Symbols during display the time
+ New settings menu (short-press / long press)
+ Every information are separate selectable
+ New symbols
+ Font full hight
+ Temperature available (DHT11 or DHT22 sensor requiered)
+ Humidity available (DHT11 or DHT22 sensor required )

### Version 1.4
-(Eisbaeeer 20201026
+ display correct counter with logo if more than 4 modules used

### 1.3
- (Eisbaeeer 20201015)   
+ push button mode on display

### 1.2
- (Eisbaeeer 20201012)   
+ WebPage Config full support
+ Changed binary to default export of Arduino IDE
+ Changed startup mode to 7

### 1.0
- (Eisbaeeer)
Initial Version

## License
GNU Affero General Public License v3.0

## Based on
https://github.com/jegade/followercounter
