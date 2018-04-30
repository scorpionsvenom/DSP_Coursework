#include "CircleBuffer.h"

CircleBuffer::CircleBuffer(int length) : length(length)
{
	buffer = new float[length];
	tail = 0;
}

CircleBuffer::~CircleBuffer()
{
	delete buffer;
}

void CircleBuffer::put(float value)
{
	buffer[tail % length] = value;
	tail++;
}

float CircleBuffer::getValueAtIndex(int index)
{
	if (index < 0 || index > tail)
		return -1.f;

	return buffer[index % length];
}

int CircleBuffer::getCurrentTail()
{
	return tail;
}

int CircleBuffer::getBufferLength()
{
	return length;
}