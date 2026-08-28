
#ifndef OMNI_INPUT_SCREENCAPTUREBACKEND_H_
#define OMNI_INPUT_SCREENCAPTUREBACKEND_H_

#include <QImage>
#include <QSize>

class ScreenCaptureBackend
{
public:
	ScreenCaptureBackend();
	~ScreenCaptureBackend();

	bool capture(QImage& image);
};

#endif /* OMNI_INPUT_SCREENCAPTUREBACKEND_H_ */

