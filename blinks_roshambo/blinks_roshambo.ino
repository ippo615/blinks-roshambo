
#define ROCK 1
#define PAPER 2
#define SCISSORS 3
#define N_CHOICES 3

#define CLICK_TIMER_DURATION 2000
#define IDLE_TIMER_DURATION 2000

Timer clickTimer;
Timer idleTimer;
int choice;
int visibleChoice = 0;
int team = 0;
int brightness = 0;

Color teamColors[] = {RED,GREEN,BLUE};
#define N_TEAMS 3

enum {
    MODE_IDLE,
    MODE_SHOW_CHOICE,
    MODE_SHOW_BOARD,
    MODE_CORE
};
int mode = MODE_IDLE;

void setup() {
  setValueSentOnAllFaces(0);
  choice = 1+random(N_CHOICES);
  visibleChoice = 1+random(N_CHOICES);
  team = random(N_TEAMS);
}

void loop() {
    
    if( isAlone() ){
        if( buttonLongPressed() ){
            team = 0;
            choice = visibleChoice;
            mode = MODE_CORE;
        }
        if( mode == MODE_SHOW_BOARD ){
            mode = MODE_IDLE;
        }
    }
    if( !isAlone() ){
        if( buttonPressed() ){
            mode = MODE_SHOW_BOARD;
        }
    }

    //if( mode == MODE_SHOW_BOARD ){
    uint8_t data = ((choice&0b11)<<2) | (team&0b11);
    setValueSentOnAllFaces(data);
    
    if( !isAlone() && (mode != MODE_CORE) ){

        FOREACH_FACE(f){
            
            if( mode == MODE_IDLE || mode == MODE_SHOW_CHOICE ){
                if( isValueReceivedOnFaceExpired(f) ){
                    setColorOnFace( teamColors[team], f );
                    continue;
                }else{
                    setColorOnFace( WHITE, f );
                    continue;
                }
            }
            
            if( isValueReceivedOnFaceExpired(f ) ){
                if( mode == MODE_SHOW_BOARD ){
                    setColorOnFace(OFF, f);
                    continue;
                }
            }
            uint8_t recieved = getLastValueReceivedOnFace(f);
            uint8_t rTeam = (recieved & 0b0011);
            uint8_t rChoice = (recieved & 0b1100)>>2;
            
            // ties
            if( (rTeam == team) || (rChoice == choice) ){
                setColorOnFace(dim(WHITE,255),f);
                continue;
            }
            
            // LOSING CONDITIONS
            if(
                ((rChoice == ROCK) && (choice == SCISSORS)) ||
                ((rChoice == PAPER) && (choice == ROCK)) ||
                ((rChoice == SCISSORS) && (choice == PAPER))
            ){
                setColorOnFace( dim(teamColors[rTeam],255), f );
                continue;
            }
            // Winning conditions
            if(
                ((choice == ROCK) && (rChoice == SCISSORS)) ||
                ((choice == PAPER) && (rChoice == ROCK)) ||
                ((choice == SCISSORS) && (rChoice == PAPER))
            ){
                setColorOnFace( dim(teamColors[team],255), f );
                continue;
            }
            
        }
        return;
    }
    
    if( mode == MODE_IDLE ){
        if( isAlone() ){
          if( buttonPressed() ){
            clickTimer.set(CLICK_TIMER_DURATION);      
            choice = visibleChoice;
            mode = MODE_SHOW_CHOICE;
        }
      }
    }

    if( ! idleTimer.isExpired () ){
        brightness = 2*sin8_C(map(idleTimer.getRemaining(),0,IDLE_TIMER_DURATION,0,128))-255;
    }else{
        idleTimer.set(IDLE_TIMER_DURATION);
        visibleChoice = 1+ ((visibleChoice + 1) % N_CHOICES)
    }
     
    if( mode == MODE_SHOW_CHOICE ){
        if( ! clickTimer.isExpired() ){
            brightness = map(clickTimer.getRemaining(),0,CLICK_TIMER_DURATION,0,255);
            if( buttonPressed() ){
                team = (team+1) % N_TEAMS;
            }
        }else{
            mode = MODE_IDLE;
            idleTimer.set(IDLE_TIMER_DURATION);
        }
    }

    Color color = teamColors[team];
    if( mode == MODE_CORE ){
        setColor( WHITE );
    }else{
        setColor( dim(color,brightness) );
    }
    if( mode == MODE_SHOW_CHOICE || mode == MODE_CORE ){
        visibleChoice = choice;
    }
    if( visibleChoice == ROCK ){
        setColorOnFace(OFF, 0);
    }
    if( visibleChoice == PAPER ){
        setColorOnFace( OFF, 0 );
        setColorOnFace( OFF, 3 );
    }
    if( visibleChoice == SCISSORS ){
        setColorOnFace( OFF, 0 );
        setColorOnFace( OFF, 2 );
        setColorOnFace( OFF, 4 );
    }
}

