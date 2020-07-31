/*
 * main.h
 * 		MarcDuino Master (Dome Panel Control)
 *      Author: Marc Verdiell
 *      v1.7
 *
 *      v2.0
 *      Author: Neil Hutchison
 *
 *      See description in implementation file
 *
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h> 		// uint8_t and companions

/********* BUILD SETTINGS FOR ALL VARIANTS, COMMENT OUT TO CHANGE OPTION *******************/
#define _RELEASEVERSION_		// comment out for private release version
#define _MP3TRIGGER_			// uncomment to use MP3 Trigger sounds instead of CF-III
#define _MARCDUINOV2_			// for the V2 boards
/*****************************************************************************************/

// turn to 1 for debug only
#define _ERROR_MSG_ 0
#define _FEEDBACK_MSG_ 0

// defaults to private version settings when _RELEASEVERSION_ is undefined
#ifndef _RELEASEVERSION_
#define _PRIVATEVERSION_		// settings for my own droid
#endif


// settings for the public release version are here
#ifdef _RELEASEVERSION_
	//#define _9600BAUDSJEDI_				// for future compatibility with new version of JEDI display
#endif

// settings for my own droid version are here
#ifdef _PRIVATEVERSION_
	#define _DIGITALJEDI_					// my JEDI setting is non-standard
//#define _9600BAUDSJEDI_				// for future compatibility with new version of JEDI display
#endif

#define CMD_MAX_LENGTH   64		//Max length of the command string

// all commands must start with one of these characters
#define PANEL_START_CHAR 	':'
#define HP_START_CHAR		'*'
#define DISPLAY_START_CHAR 	'@'
#define SOUND_START_CHAR 	'$'
#define ALT1_START_CHAR		'!'	// for custom extensions, messages get forwarded on SUART2 (PC4) pin of master DomePanel Marcduino
#define ALT2_START_CHAR		'%'	// for custom extensions, messages get forwarded on SUART2 (PC4) pin of slave HPPanel Marcduino
#define I2C_START_CHAR		'&' // for outputting an i2c commands on all the boards
#define SETUP_START_CHAR    '#' // For MarcDuino Setup commands.

#define CMD_END_CHAR 	'\r'// all command must end with one of these characters

// I2C control characters (not used, hard coded for now)
#define CMD_SEP_CHAR	','		// separator for I2C arguments
#define CMD_HEX_CHAR	'x' 	// for hex numbers
#define CMD_CHAR_CHAR	'\'' 	// char delimiter
#define CMD_STR_CHAR	'"'		// string delimiter

// Panel command vocabulary
#define CMD_SEQUENCE 	"SE"		// launches a sequence
#define CMD_OPEN 		"OP"		// opens a panel (00=all panels)
#define CMD_CLOSE 		"CL"		// closes a panel, removes from RC, kill servos (00=all panels)
#define CMD_RC			"RC"		// put the panels under RC control (0=remove all, >=11 put them all)
#define CMD_STOP		"ST"		// buzz kill/soft hold: remove a panel from RC control, turn servo off (0=all)
#define CMD_HOLD		"HD"		// hard hold: remove panel from RC and hold in last position (0=all that where on RC)

// Setup command vocabulary
#define SETUP_SERVO_DIR "SD"		// Servo direction.  0 forward, 1 reversed
#define SETUP_SERVO_REVERSE "SR"	// Reverse Individual Servo.  #SRxxy where xx is the servo y is 0 for forward, 1 for reverse.
#define SETUP_LAST_SERVO "SL"		// Servo Last // not used yet
#define SETUP_START_SOUND "SS"		// Startup Sound. 0 = none, 1 = sound in 255, 2 = sound in 254, 3 = sound in 253
#define SETUP_RANDOM_SOUND_DISABLED "SQ"		// Random Sounds Disabled.  0 = Random Sounds on, 1=Random Sounds disabled, volume 0, 2=Random Sounds disabled R2 Quiet
#define SETUP_SLAVE_DELAY_TIME "ST"	// Slave commanding delay.  Allow you to tune the time between sending the Slave panel command and starting master panel command execution.

void echo(char ch);
uint8_t build_command(char ch, char* output_str);
void dispatch_command(char* command_str);
void parse_panel_command(char* command, uint8_t length);
void parse_hp_command(char* command,uint8_t length);
void parse_display_command(char* command,uint8_t length);
void parse_sound_command(char* command,uint8_t length);
void parse_alt1_command(char* command, uint8_t length);
void parse_alt2_command(char* command, uint8_t length);
void process_command(char* thecommand, char* theargument);
void parse_setup_command(char* command, uint8_t length);
void sequence_command(uint8_t value);
void open_command(uint8_t value);
void close_command(uint8_t value);
void rc_command(uint8_t value);
void stop_command(uint8_t value);
void hold_command(uint8_t value);

// i2c parsing (v1.8)
void parse_i2c_command(char* command,uint8_t length);
void sendI2C(uint8_t address, uint8_t* payload, uint8_t payload_length);
uint8_t append_token(uint8_t* payload, uint8_t* index, char* token);


void init_jedi();
void resetJEDIcallback();

////////////////////////////////////////////
// below are the internally generated
// serial commands (not sent directly by R2 Touch)
// Mostly to generate sequence scripts
///////////////////////////////////////////

// Perform HP actions
void HPOff();
void HPOn();
void HPStop();
void HPRandom();
void HPFlicker(uint8_t seconds);
void HP1Flicker(uint8_t seconds);
void HPFlash(uint8_t seconds);
void HP1RC();

// Perform Magic Panel actions
void MagicFlicker(uint8_t seconds);
void MagicPanelVU();
void MagicPanelCylonH();
void resetMPcallback();

// Perform Display actions
void DisplayNormal();
void DisplayScream();
void DisplayFlash4();
void DisplaySpectrum();
void DisplayShortCircuit();
void DisplayLeia();

// RLD text
void RLDSetMessage(char* message);
void RLDDisplayMessage();

// Perform Sound
void Sound(uint8_t bank, uint8_t number);
void SoundScream();
void SoundWave();
void SoundDisco();
void SoundFastWave();
void SoundOpenWave();
void SoundBeepCantina();
void SoundFaint();
void SoundCantina();
void SoundLeia();
void SoundRandom();
void SoundStop();


// Slave sequencer
void StartSlaveSequence(uint8_t value);
uint8_t slave_delay_time;
void SendSetupToSlave(char* command, uint8_t value);



#endif /* MAIN_H_ */
