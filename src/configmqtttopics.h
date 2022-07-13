
String BASE_TOPIC = me_lokalizacja; //jest niepelna -brakuje kondygnacji


const String LOG_TOPIC = BASE_TOPIC + "/log";
const String WILL_TOPIC = BASE_TOPIC + "/Will";
const String IP_TOPIC = BASE_TOPIC + "/IP";
const String STATS_TOPIC = BASE_TOPIC + "/stats";


const String BASE_HA_TOPIC = "homeassistant";
const String OUTSIDE = "outside";
const String BOILERROOM = "boilerroom";
const String TEMPERATURE = "_temperature";



const String SET_LAST = "/set";
const String SENSOR = "/sensor/";
const String SWITCH = "/switch/";

const String OUTSIDE_TEMPERATURE_N = OUTSIDE + TEMPERATURE + "_North";
const String OUTSIDE_TEMPERATURE_E = OUTSIDE + TEMPERATURE + "_East";
const String OUTSIDE_TEMPERATURE_W = OUTSIDE + TEMPERATURE + "_West";
const String OUTSIDE_TEMPERATURE_S = OUTSIDE + TEMPERATURE + "_South";
const String OUTSIDE_TEMPERATURE_A = OUTSIDE + TEMPERATURE + "_Averange";
#ifdef newSensorT1
const String BOILERROOM_TEMPERATURE_T1 = BOILERROOM + TEMPERATURE + "_Spare";
#endif
const String HEATERCO_TEMPERATURE = BOILERROOM + TEMPERATURE + "_CO";
const String WATER_TEMPERATURE = BOILERROOM + TEMPERATURE + "_Water";
const String BOILERROOM_TEMPERATURE = BOILERROOM + TEMPERATURE;
const String BOILERROOM_PRESSURE = BOILERROOM + "_pressure";
const String BOILERROOM_HIGH = BOILERROOM + "_high";
const String BOILERROOM_HIGHREAL = BOILERROOM + "_highreal";
const String BOILERROOM_COVAL = BOILERROOM + "_gasCO";
const String BOILERROOM_PUMP1WA = BOILERROOM + "_pump1Water";
const String BOILERROOM_PUMP1WA_E = BOILERROOM + "_pump1Water_Energy";
const String BOILERROOM_PUMP2CO = BOILERROOM + "_pump2CO";
const String BOILERROOM_PUMP2CO_E = BOILERROOM + "_pump2CO_Energy";

const String BOILERROOM_SWITCH_TOPIC = BASE_TOPIC + SWITCH + BOILERROOM + "/attributes";
const String BOILERROOM_SWITCH_TOPIC_SET = BASE_TOPIC + SWITCH + BOILERROOM + "/set";
const String BOILERROOM_HA_SWITCH_TOPIC = BASE_HA_TOPIC + SWITCH + BASE_TOPIC + "/" + BOILERROOM;     //+"/state"
const String BOILERROOM_SENSOR_TOPIC = BASE_TOPIC + SENSOR + BOILERROOM + "/attributes";
const String BOILERROOM_HA_SENSOR_TOPIC = BASE_HA_TOPIC + SENSOR + BASE_TOPIC + "/" + BOILERROOM;     //+"/state"
//Subscribe
String SUPLA_VOLT_TOPIC = "electricmain/supla/devices/zamel-mew-01-99a200/channels/0/state/phases/3/voltage";
String SUPLA_FREQ_TOPIC = "electricmain/supla/devices/zamel-mew-01-99a200/channels/0/state/phases/3/frequency";


// const String ROOM_TEMPERATURE = ROOM_TEMP + TEMPERATURE;
// const String ROOM_TEMPERATURE_SETPOINT = ROOM_TEMPERATURE + "_setpoint";
// const String ROOM_TEMPERATURE_SETPOINT_SET_TOPIC = BASE_TOPIC + "/SET/" + ROOM_TEMPERATURE_SETPOINT; //+SET_LAST + "/set"; // t


// const String ROOMS_HACLI_TOPIC = BASE_HA_TOPIC + "/climate/" + BASE_TOPIC + "/" + ROOM_TEMP; //+"/state"





// const String BOILER = "boiler";
// const String HOT_WATER = "domestic_hot_water";
// const String ROOM_OTHERS = "room_other";

// const String BOILER_TEMPERATURE = BOILER + TEMPERATURE;
// // const String BOILER_MOD = BOILER+"-mode";   //tryb pracy
// const String BOILER_TEMPERATURE_RET = BOILER + TEMPERATURE + "_return";
// const String BOILER_TEMPERATURE_SETPOINT = BOILER + TEMPERATURE + "_setpoint";
// // const String BOILER_CH_STATE = BOILER + "_ch_state";
// const String BOILER_SOFTWARE_CH_STATE_MODE = BOILER + "_software_ch_state_and_mode";
// // const String FLAME_STATE = "flame_state";
// // const String FLAME_LEVEL = "flame_level";
// const String TEMP_CUTOFF = "temp_cutoff";
// // const String FLAME_W = "flame_used_energy";
// // const String FLAME_W_TOTAL = "flame_used_energy_total";

// // const String HOT_WATER_TEMPERATURE = HOT_WATER + TEMPERATURE;
// const String HOT_WATER_TEMPERATURE_SETPOINT = HOT_WATER + TEMPERATURE + "_setpoint";
// // const String HOT_WATER_CH_STATE = HOT_WATER + "_dhw_state";
// const String HOT_WATER_SOFTWARE_CH_STATE = HOT_WATER + "_software_dhw_state";


// // const String ROOM_OTHERS_PRESSURE = ROOM_OTHERS + "_pressure";

// const String BOILER_TOPIC = BASE_TOPIC + "/" + BOILER + "/attributes";
// // const String HOT_WATER_TOPIC = BASE_TOPIC + "/" + HOT_WATER + "/attributes";



// const String TEMP_SETPOINT_SET_TOPIC = BASE_TOPIC + "/SET/" + BOILER_TEMPERATURE_SETPOINT + "/set";  // sp
// const String TEMP_CUTOFF_SET_TOPIC = BASE_TOPIC + "/SET/" + TEMP_CUTOFF + "/set";                    // cutOffTemp
// const String STATE_DHW_SET_TOPIC = BASE_TOPIC + "/SET/" + HOT_WATER_SOFTWARE_CH_STATE + "/set";      // enableHotWater
// const String MODE_SET_TOPIC = BASE_TOPIC + "/SET/" + BOILER_SOFTWARE_CH_STATE_MODE + "/set";         // 012 auto, heat, off ch
// const String TEMP_DHW_SET_TOPIC = BASE_TOPIC + "/SET/" + HOT_WATER_TEMPERATURE_SETPOINT + "/set";    // dhwTarget
// String COPUMP_GET_TOPIC = "COWoda_mqqt_MARM/switch/bcddc2b2c08e/pump2CO/state";                      // temperatura outside avg NEWS
// String NEWS_GET_TOPIC = "COWoda_mqqt_MARM/sensor/bcddc2b2c08e/WENS_Outside_Temp_AVG/state";          // pompa CO status
// String BOILER_FLAME_STATUS_TOPIC = "opentherm-thermostat/boiler/attributes";                              //flme status of co gaz boiler
// String BOILER_FLAME_STATUS_ATTRIBUTE = "ot_flame_state";                              //boiler flame status of co gaz boiler
// String BOILER_COPUMP_STATUS_ATTRIBUTE = "ot_boiler_ch_state";                          //boiler pump status

// // logs topic
// const String DIAGS = "diag";
// const String DIAG_TOPIC = BASE_TOPIC + "/" + DIAGS + "/attributes";
// const String DIAG_HA_TOPIC = BASE_HA_TOPIC + "/sensor/" + BASE_TOPIC + "/";
// const String DIAG_HABS_TOPIC = BASE_HA_TOPIC + "/binary_sensor/" + BASE_TOPIC + "/";

// const String LOGS = "log";
// const String LOG_GET_TOPIC = BASE_TOPIC + "/" + DIAGS + "/" + LOGS;
// const String INTEGRAL_ERROR_GET_TOPIC = DIAGS + "_" + "interr";
// const String DIAGS_OTHERS_FAULT = DIAGS + "_" + "fault";
// const String DIAGS_OTHERS_DIAG = DIAGS + "_" + "diagnostic";

// //Homeassistant Autodiscovery topics
// const String BOILER_HA_TOPIC = BASE_HA_TOPIC + "/sensor/" + BASE_TOPIC + "/";              //+"/state"
// const String BOILER_HABS_TOPIC = BASE_HA_TOPIC + "/binary_sensor/" + BASE_TOPIC + "/";     //+"/state"
// const String BOILER_HACLI_TOPIC = BASE_HA_TOPIC + "/climate/" + BASE_TOPIC + "/" + BOILER; //+"/state"

// const String HOT_WATER_HA_TOPIC = BASE_HA_TOPIC + "/sensor/" + BASE_TOPIC + "/";                 //+"/state"
// const String HOT_WATER_HABS_TOPIC = BASE_HA_TOPIC + "/binary_sensor/" + BASE_TOPIC + "/";        //+"/state"
// const String HOT_WATER_HACLI_TOPIC = BASE_HA_TOPIC + "/climate/" + BASE_TOPIC + "/" + HOT_WATER; //+"/state"





// // setpoint topic
// const String SETPOINT_OVERRIDE = "setpoint-override";
// const String SETPOINT_OVERRIDE_SET_TOPIC = BASE_TOPIC + "/" + SETPOINT_OVERRIDE + "/set";     // op_override
// const String SETPOINT_OVERRIDE_RESET_TOPIC = BASE_TOPIC + "/" + SETPOINT_OVERRIDE + "/reset"; //
