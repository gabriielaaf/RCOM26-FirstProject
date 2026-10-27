#define FLAG 0x7E
#include "state.h"
#include "control_values.h"

SupervisionState nextSupervisionState(SupervisionState currentState,
                                      unsigned char byte,
                                      unsigned char expectedAddress,
                                      unsigned char *control) {

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

    *control = byte;
    return C_RCV;

    break;

  case C_RCV:

    if (byte == (expectedAddress ^ *control))
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

InformationState nextInformationState(InformationState currentState,
                                      unsigned char byte,
                                      unsigned char expectedAddress,
                                      unsigned char *control) {

  switch (currentState) {

  case I_START:

    if (byte == FLAG)
      return I_FLAG_RCV;

    return I_START;

  case I_FLAG_RCV:

    if (byte == FLAG)
      return I_FLAG_RCV;

    if (byte == expectedAddress)
      return I_A_RCV;

    return I_START;

  case I_A_RCV:

    if (byte == FLAG)
      return I_FLAG_RCV;

    if (byte == I0 || byte == I1 || byte == DISC || byte == SET || byte == UA ||
        byte == RR0 || byte == RR1 || byte == REJ0 || byte == REJ1) {

      *control = byte;
      return I_C_RCV;
    }

    return I_START;

  case I_C_RCV:

    if (byte == (expectedAddress ^ *control))
      return I_BCC_OK;

    if (byte == FLAG)
      return I_FLAG_RCV;

    return I_START;

  case I_BCC_OK:

    if (*control == I0 || *control == I1) {

      if (byte == FLAG)
        return I_FLAG_RCV;

      return I_DATA_RCV;
    }

    // Supervision frames end immediately after BCC1
    if (byte == FLAG)
      return I_SUPERVISION_STOP;

    return I_START;

  case I_DATA_RCV:

    if (byte == FLAG)
      return I_STOP;

    return I_DATA_RCV;

  default:
    return I_INVALID;
  }
}