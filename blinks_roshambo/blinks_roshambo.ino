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
#define N_TEAMS 7
Color TEAM_COLORS[] = {
  RED,BLUE,YELLOW,MAGENTA,GREEN,ORANGE,CYAN
};
Color COLOR_SELF_WIN_LOSE = WHITE;
#define BRIGHTNESS_WON 255
#define BRIGHTNESS_SELF 128
#define BRIGHTNESS_LOST 0
#define BRIGHTNESS_TIE 32
#define BRIGHTNESS_LONELY 0

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
  MODE_BOARD
};

#define DURATION_FEEDBACK_MS 800
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
#define INFO_BUILD(TEAM,PIECE) ((TEAM<<2)|PIECE)
#define INFO_GET_TEAM( INFO ) ((INFO&0b00011100)>>2)
#define INFO_GET_PIECE( INFO ) (INFO&0b00000011)

/**
 * Initialization
 */
void setup() {
  randomize();
  //setValueSentOnAllFaces(VALUE_NOT_READY);
  team_index = getSerialNumberByte(0) % N_TEAMS;
}

/**
 * Loops for each game mode
 */
void loop_mode_start(){
  // maybe add a delay here so that we dont get noise and can propagate team selection
  mode = MODE_TEAM_SELECT;
}
void loop_mode_team_select(){
  if (buttonSingleClicked()) {
    team_index = (team_index + 1) % N_TEAMS;
  }
  setColor( TEAM_COLORS[team_index] );
  if( buttonLongPressed() ){
    mode = MODE_TEAM_PROPAGATE;
    timerFeedbackAnimation.set(DURATION_FEEDBACK_MS);
  }

  /*
   * // Listening seems to be buggy
  FOREACH_FACE(f){
    byte otherValue = getLastValueReceivedOnFace(f);
    if( ! isValueReceivedOnFaceExpired(f) ){
      team_index = otherValue;
      mode = MODE_TEAM_PROPAGATE;
    }
  }
  */
}
void loop_mode_team_propagate(){
  if( timerFeedbackAnimation.isExpired() ){
    mode = MODE_PIECE_SELECT;
  }else{
    draw_animate_pulse( TEAM_COLORS[team_index], timerFeedbackAnimation, DURATION_FEEDBACK_MS, 2 );
  }
}
void loop_mode_piece_select(){
  if (buttonSingleClicked()) {
    piece_type_index = (piece_type_index + 1) % N_PIECE_TYPES;
  }
  if( PIECE_TYPES[piece_type_index] == ROCK ){
    setColorOnFace( TEAM_COLORS[team_index], 0 );
    setColorOnFace( TEAM_COLORS[team_index], 1 );
    setColorOnFace( TEAM_COLORS[team_index], 2 );
    setColorOnFace( TEAM_COLORS[team_index], 3 );
    setColorOnFace( TEAM_COLORS[team_index], 4 );
    setColorOnFace( OFF, 5 );
  }else
  if( PIECE_TYPES[piece_type_index] == PAPER ){
    setColorOnFace( OFF, 0 );
    setColorOnFace( TEAM_COLORS[team_index], 1 );
    setColorOnFace( TEAM_COLORS[team_index], 2 );
    setColorOnFace( OFF, 3 );
    setColorOnFace( TEAM_COLORS[team_index], 4 );
    setColorOnFace( TEAM_COLORS[team_index], 5 );
  }else
  if( PIECE_TYPES[piece_type_index] == SCISSOR ){
    setColorOnFace( TEAM_COLORS[team_index], 0 );
    setColorOnFace( OFF, 1 );
    setColorOnFace( TEAM_COLORS[team_index], 2 );
    setColorOnFace( OFF, 3 );
    setColorOnFace( TEAM_COLORS[team_index], 4 );
    setColorOnFace( OFF, 5 );
  }
  if( buttonLongPressed()){
    mode = MODE_BOARD;
  }
}
void loop_mode_board(){
  setValueSentOnAllFaces( INFO_BUILD( team_index, values[piece_type_index] ) );
  FOREACH_FACE(f){
    byte info = getLastValueReceivedOnFace(f);
    if( ! isValueReceivedOnFaceExpired(f) ){
      infos[f] = info;
      results[f] = result_lookup( values[piece_type_index], values[INFO_GET_PIECE(info)] );
    }else{
      results[f] = RESULT_FACE_LONELY;
    }
    result_draw( results[f], INFO_GET_TEAM(info), team_index, f );
  }

  if( buttonLongPressed() ){
    mode = MODE_START;
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
  if( mode == MODE_BOARD ){
    loop_mode_board();
  }

}

void draw_animate_pulse( Color color, Timer timer, unsigned int duration, byte cycles ){
  byte pulseMapped = map(timer.getRemaining(), 0, duration, 0, 255*cycles);
  setColor( dim(color, sin8_C(pulseMapped)) );
}

/**
 * Given a result and a face, light the appropriate color on that face.
 */
void result_draw( byte result, byte other_team, byte this_team, byte face ){
  if( result == RESULT_FACE_WON ){
    if( other_team == this_team ){
      setColorOnFace( dim( COLOR_SELF_WIN_LOSE, BRIGHTNESS_SELF ), face );
    }else{
      setColorOnFace( dim( TEAM_COLORS[team_index], BRIGHTNESS_WON ), face );
    }
  }else if( result == RESULT_FACE_LOST ){
    if( other_team == this_team ){
      setColorOnFace( dim( COLOR_SELF_WIN_LOSE, BRIGHTNESS_SELF ), face );
    }else{
      setColorOnFace( dim( TEAM_COLORS[other_team], BRIGHTNESS_WON ), face );
    }
  }else if( result == RESULT_FACE_TIE ){
    setColorOnFace( dim( TEAM_COLORS[team_index], BRIGHTNESS_WON ), face );
  }else{
    setColorOnFace( dim( TEAM_COLORS[team_index], BRIGHTNESS_LONELY ), face );
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
