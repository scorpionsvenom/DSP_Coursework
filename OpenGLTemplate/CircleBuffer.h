#pragma once

class CircleBuffer
{
public:
	CircleBuffer(int length);

	~CircleBuffer();

	void put(float value);
	float getValueAtIndex(int index);
	int getCurrentTail();
	int getBufferLength();

private:
	int length;
	int tail;
	float *buffer;
};