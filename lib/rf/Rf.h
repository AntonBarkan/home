
#ifndef Rf_h
#define Rf_h

#include <iarduino_RF433_Receiver.h>
#include <iarduino_RF433_Transmitter.h>

class Rf {
public:
  Rf(const char *id, byte txPin, byte rxPin);
  void sendCommand(char *to, char *command);
  char *readCommand();

private:
  char _id[2];
  iarduino_RF433_Transmitter *radioTX;
  iarduino_RF433_Receiver *radioRX;
  char sendedMessages[100];
  char messageId[2];

  void fillId(char *);
  void sendAck(char *, char *);
  void increaseMessageId();
};

#endif
