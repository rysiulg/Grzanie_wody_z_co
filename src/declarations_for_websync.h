//defined in common_functions 0,1,2
#define ASS_uptimedana 0
#define ASS_uptimedanaStr "uptimedana"
#define ASS_Statusy 1
#define ASS_StatusyStr "Statusy"
#define ASS_MemStats 2
#define ASS_MemStatsStr "MemStats"
#define ASS_temp_NEWS  3      // avg temperatura outside -getting this from mqtt topic as averange from 4 sensors North West East South
#define ASS_temp_NEWSStr "temp_NEWS"
#define ASS_lastNEWSSet 4      //last time NEWS get updated from mqtt
#define ASS_lastNEWSSetStr "lastNEWSSet"
#define ASS_bmTemp 5           //hot water temp
#define ASS_bmTempStr "bmTemp"
#define ASS_coTherm 6         //hot water temp set
#define ASS_coThermStr "coTherm"
#define ASS_forceCObelow 7        // outside temp setpoint to cutoff heating co. CO heating is disabled if outside temp (temp_NEWS) is above this value
#define ASS_forceCObelowStr "sliderValue1"
#define ASS_waterTherm 8
#define ASS_waterThermStr "waterTherm"
#define ASS_NTherm 9
#define ASS_NThermStr "NTherm"
#define ASS_ETherm 10
#define ASS_EThermStr "ETherm"
#define ASS_WTherm 11        //boiler heat for co heat/not
#define ASS_WThermStr "WTherm"
#define ASS_STherm 12      //hotWater heat active/not
#define ASS_SThermStr "STherm"
#define ASS_coConstTempCutOff 13
#define ASS_coConstTempCutOffStr "sliderValue2"
#define ASS_dcoval 14
#define ASS_dcovalStr "dcoval"
#define ASS_forceCO 15
#define ASS_forceCOStr "forceCO"
#define ASS_forceWater 16
#define ASS_forceWaterStr "forceWater"
#define ASS_prgstatusrelay1WO  17        //Return temperature
#define ASS_prgstatusrelay1WOStr "prgstatusrelay1WOStr"
#define ASS_prgstatusrelay2CO 18
#define ASS_prgstatusrelay2COStr "prgstatusrelay2CO"
#define ASS_najpierwCO 19           //Statistical data of used media
#define ASS_najpierwCOStr "najpierwCO"
#define ASS_dbmpressval 20     // boiler tempset on heat modetemp boiler set -mainly used in auto mode and for display on www actual temp
#define ASS_dbmpressvalStr "dbmpressval"
#define ASS_bm_high 21          //tryb pracy kondensacyjny -eco -temp grzania CO max 40st
#define ASS_bm_highStr "bm_high"
#define ASS_opcohi 22
#define ASS_opcohiStr "opcohi"
#define ASS_bm_high_real 23
#define ASS_bm_high_realStr "bm_high_real"
#define ASS_pump1wo 24
#define ASS_pump1woStr "pump1wo"
#define ASS_pump2co 25
#define ASS_pump2coStr "pump2co"
#define ASS_waterThermBG 26         //hot water temp set
#define ASS_waterThermBGStr "waterTherm_boilerGAZ"
#define ASS_waterThermBGtime 27         //hot water temp set
#define ASS_waterThermBGtimeStr "waterTherm_boilerGAZ_time"
#define ASS_UsedMedia 28         //hot water temp set
#define ASS_UsedMediaStr "UsedMedia"






#define ASS_Num (28 + 1)              //number of ASS definitions
#define numDecimalsWWW 1
#define cutoff_histereza 0.7        //histereza cutoff and roomtemp/roomtempset
#define dhwTargetStart 51           // domyslna temperatura docelowa wody uzytkowej
#define cutoffstartval 2
#define roomtempinitial 21
#define ecoModeMaxTemp 39           //max temp pracy kondensacyjnej
#define ecoModeDisabledMaxTemp 60
#define wwwstatus_on "on"
#define wwwstatus_off "off"
#define logStandard 0               //standard log send
#define logCommandResponse 1        //send response to websocket after received command
#define remoteHelperMenu 0          //werbserial -remote commands helper: get menu help
#define remoteHelperMenuCommand 1   //werbserial -remote commands helper: get menucommand line
#define remoteHelperCommand 2       //werbserial -remote commands helper: run command
