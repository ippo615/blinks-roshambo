/**
 * Title: Rock, Paper, Scissors, Blinks! (aka RGB)
 * Author: Andrew Ippoliti
 * Platform: https://blinks.games/
 * Instructions: Read the code??
 * License: MIT
 */

/**
 * Styles
 */
#define N_TEAMS 4
Color TEAM_COLORS[] = {
  RED,BLUE,YELLOW,GREEN
};
Color COLOR_SELF_WIN_LOSE = WHITE;
#define BRIGHTNESS_WON 255
#define BRIGHTNESS_SELF 128
#define BRIGHTNESS_LOST 0
#define BRIGHTNESS_TIE 32
#define BRIGHTNESS_LONELY 0

byte BITMAP_ROCK[] = {255,255,255,255,255,0};
byte BITMAP_PAPER[] = {0,255,255,0,255,255};
byte BITMAP_SCISSOR[] = {255,0,255,0,255,0};
byte* BITMAPS[] = {
  (byte*)&BITMAP_ROCK,
  (byte*)&BITMAP_PAPER,
  (byte*)&BITMAP_SCISSOR,
};

/**
 * Definitions for Readability/Maintence
 */
enum {
  ROCK,
  PAPER,
  SCISSOR
};

#define N_PIECE_TYPES 3
byte PIECE_TYPES[] = {ROCK,PAPER,SCISSOR};
byte values[] = {ROCK,PAPER,SCISSOR};

enum {
  MODE_START,
  MODE_TEAM_SELECT,
  MODE_TEAM_PROPAGATE,
  MODE_PIECE_SELECT,
  MODE_PIECE_SELECT_FEEDBACK,
  MODE_BOARD,
  MODE_BOARD_INFO
};

#define DURATION_FEEDBACK_MS 800
#define DURATION_PIECE_INFO_MS 2000
Timer timerFeedbackAnimation;



#define RESULT_FACE_WON 0
#define RESULT_FACE_LOST 1
#define RESULT_FACE_TIE 2
#define RESULT_FACE_LONELY 3

/**
 * Game State
 */
byte results[] = {
  RESULT_FACE_LONELY,
  RESULT_FACE_LONELY,
  RESULT_FACE_LONELY,
  RESULT_FACE_LONELY,
  RESULT_FACE_LONELY,
  RESULT_FACE_LONELY
};
byte infos[] = {0,0,0,0,0,0};
byte mode = MODE_TEAM_SELECT;
byte team_index = 0;
byte piece_type_index = 0;

/*
// Another way of "bit manipulation" but due to point/address stuff
// and a few extra lines of code... the macro bit manipulation seems
// easier to maintain.
typedef struct {
  byte team: 3;
  byte piece: 2;
} type_data;
type_data received_datas[] = {0,0,0,0,0,0};
type_data sent_data = {
  team: team_index,
  piece: piece_type_index
};
setValueSentOnAllFaces( (byte*)&sent_data );
*/

/**
 * Data Communication Format
 * We can send a number between 0 and 64. This means we have 6 bits
 * of information.
 * For pieces we have 4 types: 2 bits (full)
 * For teams we have 4 types: 2 bits (full)
 * For functions we have 3 types: 2 bits (not full)
 */
#define INFO_BUILD(TEAM,PIECE,FUNCTION) ( (FUNCTION<<4) | (TEAM<<2) | PIECE )
#define INFO_GET_FUNCTION( INFO ) ((INFO&0b00110000)>>4)
#define INFO_GET_TEAM( INFO ) ((INFO&0b00001100)>>2)
#define INFO_GET_PIECE( INFO ) (INFO&0b00000011)

enum {
  FUNCTION_INFORM,
  FUNCTION_SET_TEAM,
  FUNCTION_CRUMPLE
};

/**
 * Initialization
 */
void setup() {
  randomize();
  team_index = getSerialNumberByte(0) % N_TEAMS;
}

/**
 * Mode: Start
 * 
 * Does nothing really. Maybe I should add some reset functionality here.
 */
void loop_mode_start(){
  mode = MODE_TEAM_SELECT;
}

/**
 * Mode: Team select
 * 
 * You can single click to cycle through the team colors. Once you have selected
 * the team color - long press to lock it in. It will propagate the selection to
 * any connected Blinks.
 */
void loop_mode_team_select(){
  if (buttonSingleClicked()) {
    team_index = (team_index + 1) % N_TEAMS;
  }

  byte isActivated = false;
  FOREACH_FACE(f){
    if(!isValueReceivedOnFaceExpired(f)){
      byte info = getLastValueReceivedOnFace(f);
      if( INFO_GET_FUNCTION(info) == FUNCTION_SET_TEAM ){
        isActivated = true;
        team_index = INFO_GET_TEAM(info);
      }
    }
  }
  if( buttonLongPressed() ){
    isActivated = true;
  }

  setColor( TEAM_COLORS[team_index] );

  if( isActivated ){
    mode = MODE_TEAM_PROPAGATE;
    timerFeedbackAnimation.set(DURATION_FEEDBACK_MS);
  }
}

/**
 * Mode: Team Propagate
 * 
 * It will animate itself to show the team that it is and will broadcast it's
 * selection to neighboring Blinks.
 */
void loop_mode_team_propagate(){
  if( timerFeedbackAnimation.isExpired() ){
    mode = MODE_PIECE_SELECT;
  }else{
    draw_animate_pulse( TEAM_COLORS[team_index], timerFeedbackAnimation, DURATION_FEEDBACK_MS, 2 );
    setValueSentOnAllFaces( INFO_BUILD( team_index, values[piece_type_index], FUNCTION_SET_TEAM ) );
  }
}

/**
 * Mode: Piece Select
 * 
 * Single click to choose between Rock/Paper/Scissor/Crumple.
 * Long click to lock selection.
 */
void loop_mode_piece_select(){
  if (buttonSingleClicked()) {
    piece_type_index = (piece_type_index + 1) % N_PIECE_TYPES;
  }
  draw_bitmap( BITMAPS[PIECE_TYPES[piece_type_index]], TEAM_COLORS[team_index], 255 );

  if( buttonLongPressed() ){
    mode = MODE_PIECE_SELECT_FEEDBACK;
    timerFeedbackAnimation.set(DURATION_FEEDBACK_MS);
  }
}

/**
 * Mode: Piece Select Feedback
 * 
 * Animates the user's piece selection before entering board mode.
 */
void loop_mode_piece_select_feedback(){
  if( timerFeedbackAnimation.isExpired() ){
    mode = MODE_BOARD;
  }else{
    byte piece = values[piece_type_index];
    draw_animate_pulse_bitmap( BITMAPS[piece], TEAM_COLORS[team_index], timerFeedbackAnimation, DURATION_FEEDBACK_MS, 2 );
  }
}

/**
 * Mode: Board Mode
 * 
 * Shows winner/loser tie when placed next to another Blink.
 * On single click - animates the piece to show if it is rock/paper/scissor.
 * On long press - resets this Blink for a new round.
 */
void loop_mode_board(){
  setValueSentOnAllFaces( INFO_BUILD( team_index, values[piece_type_index], FUNCTION_INFORM ) );
  FOREACH_FACE(f){
    byte info = getLastValueReceivedOnFace(f);
    if( ! isValueReceivedOnFaceExpired(f) ){
      infos[f] = info;
      results[f] = result_lookup( values[piece_type_index], values[INFO_GET_PIECE(info)] );
      result_draw( results[f], INFO_GET_TEAM(info), team_index, f );
    }else{
      setColorOnFace( dim( TEAM_COLORS[team_index], BRIGHTNESS_LONELY ), f );
    }
    
  }

  if( buttonLongPressed() ){
    mode = MODE_START;
  }

  if( buttonSingleClicked() ){
    mode = MODE_BOARD_INFO;
    timerFeedbackAnimation.set(DURATION_PIECE_INFO_MS);
  }
}

/**
 * Mode: Board Info
 * 
 * Shows the piece (rock/paper/scissor) that this Blink is.
 * Then returns to board mode.
 */
void loop_mode_board_info(){
  if( timerFeedbackAnimation.isExpired() ){
    mode = MODE_BOARD;
  }else{
    byte piece = values[piece_type_index];
    draw_animate_pulse_bitmap( BITMAPS[piece], TEAM_COLORS[team_index], timerFeedbackAnimation, DURATION_PIECE_INFO_MS, 1 );
  }
}

/**
 * Main loop
 */
void loop() {

  if( mode == MODE_START ){
    loop_mode_start();
  }else
  if( mode == MODE_TEAM_SELECT ){
    loop_mode_team_select();
  }else
  if( mode == MODE_TEAM_PROPAGATE ){
    loop_mode_team_propagate();
  }else
  if( mode == MODE_PIECE_SELECT ){
    loop_mode_piece_select();
  }else
  if( mode == MODE_PIECE_SELECT_FEEDBACK ){
    loop_mode_piece_select_feedback();
  }else
  if( mode == MODE_BOARD ){
    loop_mode_board();
  }else
  if( mode == MODE_BOARD_INFO ){
    loop_mode_board_info();
  }

}

void draw_animate_pulse( Color color, Timer timer, unsigned int duration, byte cycles ){
  byte pulseMapped = map(timer.getRemaining(), 0, duration, 0, 255*cycles);
  setColor( dim(color, sin8_C(pulseMapped)) );
}
void draw_animate_pulse_bitmap( byte* bitmap, Color color, Timer timer, unsigned int duration, byte cycles ){
  byte pulseMapped = map(timer.getRemaining(), 0, duration, 0, 255*cycles);
  draw_bitmap( bitmap, color, sin8_C(pulseMapped) );
}
void draw_animate_fade_in_function( void *draw(Color), Color color, Timer timer, unsigned int duration ){
  byte pulseMapped = map(timer.getRemaining(), 0, duration, 0, 128);
  draw( dim(color, sin8_C(pulseMapped)) );
}
void draw_animate_fade_out_function( void *draw(Color), Color color, Timer timer, unsigned int duration ){
  byte pulseMapped = map(timer.getRemaining(), 0, duration, 128, 0);
  draw( dim(color, sin8_C(pulseMapped)) );
}
void draw_bitmap( byte* bitmap, Color color, byte brightness ){
  FOREACH_FACE( f ){
    byte alpha = map( bitmap[f], 0, 255, 0, brightness );
    setColorOnFace( dim(color,alpha), f );
  }
}

/**
 * Given a result and a face, light the appropriate color on that face.
 * 
 * The idea is to be able to read the score directly from the game board. To
 * do that we draw the following:
 * If it's you vs you - draw white on both faces (otherwise you can win by
 * simply putting your pieces together).
 * Win: draw your color on both faces (+2 you)
 * Lose: draw your opponents color on both faces (+2 opponent)
 * Tie: draw your own color on your face (+1 you, +1 opponent)
 */
void result_draw( byte result, byte other_team, byte this_team, byte face ){
  if( other_team == this_team ){
    setColorOnFace( dim( COLOR_SELF_WIN_LOSE, BRIGHTNESS_SELF ), face );
  }else{
    if( result == RESULT_FACE_WON ){
      setColorOnFace( dim( TEAM_COLORS[team_index], BRIGHTNESS_WON ), face );
    }else if( result == RESULT_FACE_LOST ){
      setColorOnFace( dim( TEAM_COLORS[other_team], BRIGHTNESS_WON ), face );
    }else if( result == RESULT_FACE_TIE ){
      setColorOnFace( dim( TEAM_COLORS[team_index], BRIGHTNESS_WON ), face );
    }else{
      setColorOnFace( dim( TEAM_COLORS[team_index], BRIGHTNESS_LONELY ), face );
    }
  }
}

/**
 * Given the value (ROCK/PAPER/SCISSORS) of this Blink and the value
 * of the opponent's blick -- tell me if I won, lost or tied.
 */
byte result_lookup( byte my_value, byte your_value ){
  if( my_value == your_value ){
    return RESULT_FACE_TIE;
  }
  if( my_value == ROCK ){
    if( your_value == PAPER ){
      return RESULT_FACE_LOST;
    }
    if( your_value == SCISSOR ){
      return RESULT_FACE_WON;
    }
  }
  if( my_value == PAPER ){
    if( your_value == SCISSOR ){
      return RESULT_FACE_LOST;
    }
    if( your_value == ROCK ){
      return RESULT_FACE_WON;
    }
  }
  if( my_value == SCISSOR ){
    if( your_value == ROCK ){
      return RESULT_FACE_LOST;
    }
    if( your_value == PAPER ){
      return RESULT_FACE_WON;
    }
  }
}
