#ifndef COMMUNICATE_H_
#define COMMUNICATE_H_

void enableTransmit(bool enable);
void sendParameter(int voicenumber, int parameter, float value);
void sendChoice(int voicenumber, int choice, int value);

#endif // COMMUNICATE_H_
