#ifndef STATE_H
#define STATE_H

typedef enum {
  START,
  FLAG_RCV,
  A_RCV,
  C_RCV,
  BCC_OK,
  STOP,
  INVALID
} SupervisionState;

typedef enum {
  I_START,
  I_FLAG_RCV,
  I_A_RCV,
  I_C_RCV,
  I_BCC_OK,
  I_DATA_RCV,
  I_STOP,
  I_INVALID
} InformationState;

SupervisionState nextSupervisionState(SupervisionState currentState,
                                      unsigned char byte,
                                      unsigned char expectedAddress,
                                      unsigned char *control);

InformationState nextInformationState(InformationState currentState,
                                      unsigned char byte,
                                      unsigned char expectedAddress,
                                      unsigned char *control);

#endif