
typedef enum { START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP, INVALID } State;

State nextState(State currentState, unsigned char byte,
                unsigned char expectedAddress, unsigned char expectedControl);