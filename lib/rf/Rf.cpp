#include "Rf.h"
#include <iarduino_RF433_Receiver.h>
#include <iarduino_RF433_Transmitter.h>

#define MESSAGE_SIZE 17
#define ACK_POSITION 4
#define MESSAGE_ID_POSITION 5
#define COMMAND_START_POSITION 7

#define ACK_REQUERED '1'
#define IS_ACK '2'

Rf::Rf(const char *id, byte txPin, byte rxPin) {
  radioTX = new iarduino_RF433_Transmitter(txPin);
  radioTX->begin();
  radioTX->setDataRate(i433_1KBPS);
  radioTX->openWritingPipe(5);

  radioRX = new iarduino_RF433_Receiver(rxPin);
  radioRX->begin();
  radioRX->setDataRate(i433_1KBPS);
  radioRX->openReadingPipe();
  radioRX->startListening();

  _id[0] = id[0];
  _id[1] = id[1];

  messageId[0] = 0;
  messageId[1] = 0;
}

void Rf::sendCommand(char *to, char *command) {
  char j[MESSAGE_SIZE];
  fillId(&j[2]);
  j[0] = to[0];
  j[1] = to[1];
  j[ACK_POSITION] = ACK_REQUERED;
  j[MESSAGE_ID_POSITION] = messageId[1];
  j[MESSAGE_ID_POSITION + 1] = messageId[0];
  increaseMessageId();
  for (int i = COMMAND_START_POSITION; i < MESSAGE_SIZE; i++) {
    j[i] = command[i - COMMAND_START_POSITION];
  }
  radioTX->write(&j, MESSAGE_SIZE);
}

void Rf::sendAck(char *to, char *messageId) {
  char j[MESSAGE_SIZE];
  fillId(&j[2]);
  j[0] = to[0];
  j[1] = to[1];
  j[ACK_POSITION] = IS_ACK;
  j[MESSAGE_ID_POSITION] = messageId[0];
  j[MESSAGE_ID_POSITION + 1] = messageId[1];
  radioTX->write(&j, MESSAGE_SIZE);
}

void Rf::fillId(char *toId) {
  toId[0] = _id[0];
  toId[1] = _id[1];
}

void Rf::increaseMessageId() {
  messageId[1] += 1;
  if (messageId[1] == 0) {
    messageId[0] += 1;
  }
}

char *Rf::readCommand() {
  char message[MESSAGE_SIZE];
  while (radioRX->available()) {
    radioRX->read(&message, MESSAGE_SIZE);
    if (message[0] == _id[0] && message[1] == _id[1]) {
      if (IS_ACK == message[ACK_POSITION]) {
        // TODO ack logic
      } else {
        if (ACK_REQUERED == message[ACK_POSITION]) {
          sendAck(&message[2], &message[MESSAGE_ID_POSITION]);
        }
        char *command = new char[10];
        for (int i = COMMAND_START_POSITION; i < MESSAGE_SIZE; i++) {
          command[i - COMMAND_START_POSITION] = message[i];
        }
        return command;
      }
    }
  }
  return 0;
}
