
const char _PAGE_HEAD[] PROGMEM = R"=====(
<HTML>
	<HEAD>
			<TITLE>Follower Counter</TITLE>
            <style>

                html {
                font-size: 62.5%;
                box-sizing: border-box;
                }

                html  {
                    padding: 1em;
                }

                *, *::before, *::after {
                margin: 0;
                padding: 0;
                box-sizing: inherit;
                font-family: sans-serif;
                }


                label {
                    display: block;
                    font-size: 18px;
                    border-bottom: 1px solid #eee; 
                    padding: 5px 0;
                    margin: 10px 0 0 0; 
                }

                label.inline {

                    display: block;
                    font-size: 13px;
                    border-bottom: 0 none;
                    padding: 2px 0;
                    margin: 0;
                }

                button, a { 
                    display: inline-block; 
                    padding: 5px 12px 3px 12px; 
                    margin: 2px; 
                    background: #eee;
                    color: #000;
                    border: 0;
                    text-decoration: none;
                }
                
                h1 { font-size: 24px; margin: 5px 0;}
         
            </style>
            <meta name = "viewport" content = "width = device-width">
            <meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>

	</HEAD>


<BODY>
)=====";

const char _PAGE_START[] PROGMEM  = "<h1>Instagram & YouTube follower counter</h1><form method=get action=/config>";

const char _PAGE_CONFIG_HUMI[] PROGMEM  = "<label>Funktionen</label> <label class=inline><input type=checkbox name=humidity value=checked %HUMISTAT% > Luftfeuchtigkeit anzeigen (nur mit DHT22 Sensor verf&uuml;gbar!)</label>"; 
const char _PAGE_CONFIG_TEMP[] PROGMEM  = "<label class=inline><input type=checkbox name=temperature value=checked %TEMPSTAT% > Temperatur anzeigen (nur mit DHT22 Sensor verf&uuml;gbar!)</label>";
const char _PAGE_CONFIG_INSTA[] PROGMEM  = "<label class=inline><input type=checkbox name=insta value=checked %INSTASTAT% > Instagram Follwer Z&auml;hler</label>";
const char _PAGE_CONFIG_YOUTU[] PROGMEM  = "<label class=inline><input type=checkbox name=youtu value=checked %YOUTUSTAT% > YoutTube Abonnenten</label>";
const char _PAGE_CONFIG_CLOCK[] PROGMEM  = "<label class=inline><input type=checkbox name=clock value=checked %CLOCKSTAT% > Uhrzeit anzeigen</label>";
const char _PAGE_CONFIG_GHOST[] PROGMEM  = "<label class=inline><input type=checkbox name=ghost value=checked %GHOSTSTAT% > Animation</label>";
const char _PAGE_CONFIG_FADE[] PROGMEM  = "<label class=inline><input type=checkbox name=fade value=checked %FADESTAT% > Fade</label>";
const char _PAGE_FOOTER[] PROGMEM = "<button type=submit>Konfiguration speichern!</button></form></BODY></HTML>";

const char _PAGE_CONFIG_INTENSITY[] PROGMEM  = "<label>Helligkeit</label> <input type=range min=0 max=15 name=intensity value=%INTENSITY% ><br> </br>" ;

const char _PAGE_CONFIG_NAME[] PROGMEM  = "<label>Instagram-Name:</label> <input type=text size=20 name=instagramname value=%INSTAGRAM% >";
const char _PAGE_CONFIG_YOUKEY[] PROGMEM  = "<label>YouTube API-Key:</label> <input type=text size=40 name=youtubeapi value=%YOUTUBEAPI% >";
const char _PAGE_CONFIG_YOUCHA[] PROGMEM  = "<label>YouTube CHANNEL-KEY:</label> <input type=text size=40 name=youtubeCha value=%YOUTUBECHA% >";

const char _PAGE_ACTIONS[] PROGMEM = "<label>Erweiterte Funktionen</label>    <a href='/reset'>Reboot</a>   <a href='/update'>Firmware-Update-Online</a> <a href='/format'>Werkseinstellungen</a>";
const char _PAGE_SOURCE[] PROGMEM = "<p><label>Infos</label>    <a href='http://www.kidbuild.de'>KidBuild</a>";
