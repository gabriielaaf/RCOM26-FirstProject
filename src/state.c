#define FLAG 0x7E
#include "state.h"

State nextState(State currentState, unsigned char byte,
                unsigned char expectedAddress, unsigned char expectedControl) {

  switch (currentState) {
  case START:
    if (byte == FLAG)
      return FLAG_RCV;

    return START;

    break;
  case FLAG_RCV:

    if (byte == FLAG)
      return FLAG_RCV;

    if (byte == expectedAddress)
      return A_RCV;

    return START;

    break;
  case A_RCV:

    if (byte == FLAG)
      return FLAG_RCV;

    if (byte == expectedControl)
      return C_RCV;

    return START;

    break;

  case C_RCV:

    if (byte == (expectedAddress ^ expectedControl))
      return BCC_OK;

    if (byte == FLAG)
      return FLAG_RCV;

    return START;

    break;

  case BCC_OK:

    if (byte == FLAG)
      return STOP;

    return START;

    break;

  default:

    return INVALID;

    break;
  }
}