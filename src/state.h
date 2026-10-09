
typedef enum { START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP, INVALID } SupervisionState;

SupervisionState nextSupervisionState(SupervisionState currentState, unsigned char byte,
                unsigned char expectedAddress, unsigned char expectedControl);