/**
 * Title: Rock, Paper, Scissors, Blinks! (aka RGB)
 * Author: Andrew Ippoliti
 * Platform: https://blinks.games/
 * Instructions: Read the code??
 * License: MIT
 */

#define N_THINGS 3

#define VALUE_NOT_READY 0
#define VALUE_ROCK 1
#define VALUE_PAPER 2
#define VALUE_SCISSOR 4

#define COLOR_ROCK makeColorRGB(255,0,0)
#define COLOR_PAPER makeColorRGB(0,255,0)
#define COLOR_SCISSOR makeColorRGB(0,0,255)
#define COLOR_NONE makeColorRGB(0,0,0)
#define COLOR_WIN makeColorRGB(0,255,0)
#define COLOR_LOSE makeColorRGB(255,0,0)
#define COLOR_TIE makeColorRGB(0,0,255)

Color colors[] = {COLOR_ROCK,COLOR_PAPER,COLOR_SCISSOR};
byte values[] = {VALUE_ROCK,VALUE_PAPER,VALUE_SCISSOR};

#define MODE_INPUT 0
#define MODE_BATTLE 1

#define RESULT_FACE_WON 0
#define RESULT_FACE_LOST 1
#define RESULT_FACE_TIE 2
#define RESULT_FACE_LONELY 3

byte selection = 0;
byte mode = MODE_INPUT;
byte results[] = {
  RESULT_FACE_LONELY,
  RESULT_FACE_LONELY,
  RESULT_FACE_LONELY,
  RESULT_FACE_LONELY,
  RESULT_FACE_LONELY,
  RESULT_FACE_LONELY
};

void setup() {
  randomize();
  setValueSentOnAllFaces(VALUE_NOT_READY);
  selection = getSerialNumberByte(0) % N_THINGS;
}

void loop() {

  /**
   * INPUT MODE
   * The player can single click to change their selection.
   * All the faces show the color of the current selection.
   * By long clicking they turn off the color and enter battle mode.
   */
  if( mode == MODE_INPUT ){
    if (buttonSingleClicked()) {
      selection = selection + 1;
      if ( selection >= N_THINGS ) {
        selection = 0; 
      }
    }
    setColor( colors[selection] );
    setValueSentOnAllFaces( VALUE_NOT_READY );
    if( buttonLongPressed() ){
      mode = MODE_BATTLE;
    }
  }

  /**
   * BATTLE MODE
   * Players put their Blinks together.
   * Each face lights up to show which side won or lost.
   * Red is a loss, green is a win, and blue is a tie.
   * A long press will "reset" into INPUT MODE and randomize
   * the starting selection (if the starting selection was
   * not randomized then players chould cheat by listening to
   * their opponents clicking... now they can only cheat by
   * observing the color of the light given off by the blink
   * which can be adjusted with brightness).
   */
  if( mode == MODE_BATTLE ){
    setValueSentOnAllFaces( values[selection] );
    FOREACH_FACE(f){
      byte otherValue = getLastValueReceivedOnFace(f);
      if( ! isValueReceivedOnFaceExpired(f) ){
        results[f] = result_lookup( values[selection], otherValue );
      }else{
        results[f] = RESULT_FACE_LONELY;
      }
      result_draw( results[f], f );
    }

    if( buttonLongPressed() ){
      mode = MODE_INPUT;
      selection = random(N_THINGS) % N_THINGS;
    }
  }

}

/**
 * Given a result and a face, light the appropriate color on that face.
 */
void result_draw( byte result, byte face ){
  if( result == RESULT_FACE_WON ){
    setColorOnFace( COLOR_WIN, face );
  }else if( result == RESULT_FACE_LOST ){
    setColorOnFace( COLOR_LOSE, face );
  }else if( result == RESULT_FACE_TIE ){
    setColorOnFace( COLOR_TIE, face );
  }else{
    setColorOnFace( COLOR_NONE, face );
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
  if( my_value == VALUE_ROCK ){
    if( your_value == VALUE_PAPER ){
      return RESULT_FACE_LOST;
    }
    if( your_value == VALUE_SCISSOR ){
      return RESULT_FACE_WON;
    }
  }
  if( my_value == VALUE_PAPER ){
    if( your_value == VALUE_SCISSOR ){
      return RESULT_FACE_LOST;
    }
    if( your_value == VALUE_ROCK ){
      return RESULT_FACE_WON;
    }
  }
  if( my_value == VALUE_SCISSOR ){
    if( your_value == VALUE_ROCK ){
      return RESULT_FACE_LOST;
    }
    if( your_value == VALUE_PAPER ){
      return RESULT_FACE_WON;
    }
  }
}
